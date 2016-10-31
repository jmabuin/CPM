/**
  * Copyright 2016 José Manuel Abuín Mosquera <josemanuel.abuin@usc.es>
  * 
  * This file is part of CPM.
  *
  * CPM is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * CPM is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with CPM. If not, see <http://www.gnu.org/licenses/>.
  */

#include "PapiCounts.h"
#include "Globals.h"


int stopPapi = 0;

void stopPapiHandler(int num){

	stopPapi = 1;
	printFunction(0, "[%s] Receiving signal and stopping Papi measures\n",__func__);

}

int countPapi(int PID){

	//int status = PAPI_NULL;

	//PAPI Variables
	long long int contadoresPapi[NUMEVENTS];

	int events[] = { PAPI_L1_DCM, PAPI_L2_DCM, PAPI_TOT_INS};
	int EventSet = PAPI_NULL;
	int retval = 0;
	//char error_str[PAPI_MAX_STR_LEN]; //To control possible errors
	const PAPI_hw_info_t *hwinfo = NULL; // Hardware info structure

	//long long start_time,after_time;
	//double total_time;

	char *shmName = (char*)malloc(sizeof(char)*25);
	char *fileName = (char*)malloc(sizeof(char)*30);

	strcpy(fileName,"/tmp/");
	strcpy(shmName,"PapiCount");

	char charPID[6];
	sprintf(charPID,"%d\0",PID);
	strcat(shmName,charPID);
	strcat(fileName,shmName);

	int fd;

	long long int *map;

	//unsigned int meassuringTime = 0;
	int i=0;


	//We listen into the signal to stop the agent

	signal(SIGUSR1, stopPapiHandler);

	//PAPI_event_info_t info; // Estrutura de informacion dun evento
	//PAPI_option_t options;



	if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT){
		printFunction(1,"[%s] Error initing PAPI: %s\n",__func__,PAPI_strerror(retval));
		exit(-1);
	}
	
	if ((hwinfo = PAPI_get_hardware_info()) == NULL) {
		printFunction(1,"[%s] Error in PAPI_get_hardware_info\n",__func__);
		exit(-1);
	}

	/* For now, the only available granularity available in PAPI is the PAPI_GRN_THR, that only measures one thread.
	 * The granularity intended to measure a whole process, PAPI_GRN_PROC, it is not yet implemented.
	 * The following code is to measure a whole process when this option will be ready.
	 */
	 
	/*
	if ((retval = PAPI_set_cmp_granularity(PAPI_GRN_PROC,0)) != PAPI_OK){
		printFunction(1,"[%s] Error assigning events granularity %s\n",__func__, PAPI_strerror(retval));
		exit(-1);
	}
	*/
	
	//PAPI_set_granularity( PAPI_GRN_MAX);
	//PAPI_set_domain(PAPI_DOM_ALL);

	//eventSet creation
	if ((retval = PAPI_create_eventset(&EventSet)) != PAPI_OK){
		printFunction(1,"[%s] Error creating eventset %s\n",__func__, PAPI_strerror(retval));
		exit(-1);
	}

	//eventSet assign
	if ((retval = PAPI_assign_eventset_component(EventSet, 0)) != PAPI_OK) {
		printFunction(1,"[%s] Error in PAPI_assign_eventset_component: %s\n",__func__, PAPI_strerror(retval));
		exit(-1);
	}

	//Start meassuring
	if ((retval=PAPI_add_events(EventSet, events, NUMEVENTS)) != PAPI_OK){
		//PAPI_perror(retval,error_str,PAPI_MAX_STR_LEN);
		printFunction(1,"[%s] Error adding events: %s\n",__func__,PAPI_strerror(retval));
		exit(-1);
	}

	//Attach to PID
	if ((retval = PAPI_attach(EventSet, PID)) != PAPI_OK){
		printFunction(1,"[%s] Error in PAPI_attach: %s\n",__func__, PAPI_strerror(retval));
		exit(1);
	}

	// Reseteamos os contadores antes de comezar
	if ((retval = PAPI_reset(EventSet)) != PAPI_OK) {
		printFunction(1,"[%s] Error in PAPI_reset: %s\n",__func__, PAPI_strerror(retval));
		exit(-1);
	}

	if ((retval = PAPI_start(EventSet)) != PAPI_OK) {
		printFunction(1,"[%s] Error initing PAPI %s\n",__func__, PAPI_strerror(retval));
		exit(-1);
	}


	//File open to mmap it into memory and write results
	if ((fd=open(fileName,O_CREAT | O_RDWR, 0666)) < 0) {
		printFunction(1,"[%s] Error opening file for writing: %s\n",__func__,strerror(errno));
		exit(EXIT_FAILURE);
	}

	int result = lseek(fd, FILESIZE-1, SEEK_SET);
	if (result == -1) {
		close(fd);
		printFunction(1,"[%s] Error calling lseek() to 'stretch' the file %s\n",__func__,strerror(errno));
		exit(EXIT_FAILURE);
	}

	result = write(fd, "", 1);
	if (result != 1) {
		close(fd);
		printFunction(1,"[%s] Error writing last byte of the file %s\n",__func__,strerror(errno));
		exit(EXIT_FAILURE);
	}

	map = (long long int *)mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED) {
		close(fd);
		printFunction(1,"[%s] Error mmapping the file %s\n",__func__,strerror(errno));
		exit(EXIT_FAILURE);
	}

	while((kill(PID,0)==0) && (!stopPapi)){

		if ((retval = PAPI_read(EventSet, contadoresPapi)) != PAPI_OK) {
			printFunction(1,"[%s] Error in PAPI_read %s\n",__func__, PAPI_strerror(retval));
			exit(-1);
		}

		if ((retval = PAPI_reset(EventSet)) != PAPI_OK) {
			printFunction(1,"[%s] Error in PAPI_reset %s\n",__func__, PAPI_strerror(retval));
			exit(-1);
		}

		for (i = 0; i<NUMEVENTS ; i++) {
			map[i] = contadoresPapi[i];
		}

		sleep(SLEEP_TIME_PAPI);

	}


	//Finishing
	if ((retval = PAPI_stop(EventSet,contadoresPapi)) != PAPI_OK) {
		printFunction(1,"[%s] Error stopping PAPI: %s\n",__func__, PAPI_strerror(retval));
		exit(-1);
	}
	//Dettach from PID
	if ((retval = PAPI_detach(EventSet)) != PAPI_OK){
		printFunction(1,"[%s] Error in PAPI_detach: %s\n",__func__, PAPI_strerror(retval));
		exit(1);
	}
	if ((retval = PAPI_cleanup_eventset(EventSet)) != PAPI_OK) {
		printFunction(1,"[%s] Error cleaning EventSet %s\n",__func__, PAPI_strerror(retval));
		exit(-1);
	}
	if ((retval = PAPI_destroy_eventset(&EventSet)) != PAPI_OK) {
		printFunction(1,"[%s] Error destroying EventSet %s\n",__func__, PAPI_strerror(retval));
		exit(-1);
	}

	if (munmap(map, FILESIZE) == -1) {
		printFunction(1,"[%s] Error un-mmapping the file %s\n",__func__,strerror(errno));
	}

	close(fd);

	if( remove( fileName ) != 0 )
		printFunction(1,"[%s] Error deleting file %s\n",__func__,strerror(errno));



	return 0;

}
