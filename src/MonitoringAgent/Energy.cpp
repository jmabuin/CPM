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


#include "Energy.h"

#define MAX_RAPL_EVENTS 64

#include "Globals.h"

int stopEnergy = 0;

void stopEnergyHandler(int num){

	stopEnergy = 1;
	printFunction(0, "[%s] Receiving signal and stopping Energy measures\n",__func__);

}

void measureEnergyFunction(){

	int EventSet = PAPI_NULL;
	int retval = 0;
	//char error_str[PAPI_MAX_STR_LEN]; //To control possible errors
	const PAPI_hw_info_t *hwinfo = NULL; // Hardware info structure

	long long start_time,after_time;
	double total_time;


	//int status = PAPI_NULL;
	int numcmp;
	int cid;
	int rapl_cid=-1;
	const PAPI_component_info_t *cmpinfo = NULL;
	
	int code, r;
	char event_names[MAX_RAPL_EVENTS][PAPI_MAX_STR_LEN];
	char units[MAX_RAPL_EVENTS][PAPI_MIN_STR_LEN];
	PAPI_event_info_t evinfo;
	int num_events=0;
	//int data_type[MAX_RAPL_EVENTS];
	long long *values;
	
	//unsigned int meassuringTime = atoi(argv[1]);
	int i=0;
	
	//PAPI_event_info_t info; // Estrutura de informacion dun evento
	//PAPI_option_t options;
	
	//Variables for Shared memory region
	char *shmName = (char*)malloc(sizeof(char)*25);
	char *fileName = (char*)malloc(sizeof(char)*30);

	strcpy(fileName,"/tmp/");
	strcpy(shmName,"EnergyCount");

	pid_t PID = getpid();

	char charPID[5];
	sprintf(charPID,"%d",PID);
	strcat(shmName,charPID);
	strcat(fileName,shmName);

	int fd;

	double *map;
	
	signal(SIGUSR1, stopEnergyHandler);

	if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT){
		printFunction(1,"[%s] Error initing PAPI: %s\n",__func__,PAPI_strerror(retval));
		exit(-1);
	}

	if ((hwinfo = PAPI_get_hardware_info()) == NULL) {
		printFunction(1,"[%s] Error in PAPI_get_hardware_info\n",__func__);
		exit(-1);
	}
	
	numcmp = PAPI_num_components();

	//We look for the RAPL component cid
	for(cid=0; cid<numcmp; cid++) {

		if ( (cmpinfo = PAPI_get_component_info(cid)) == NULL) {
			printFunction(1,"[%s] PAPI_get_component_info failed\n",__func__);
		}

		if (strstr(cmpinfo->name,"rapl")) {

			rapl_cid=cid;

			//printf("Found rapl component at cid %d\n",rapl_cid);
			
			if (cmpinfo->disabled) {

				printFunction(1,"[%s] RAPL component disabled: %s\n",__func__,cmpinfo->disabled_reason);
				
			}
			
			//break;
		}
	}
	
	//EventSet creation
	if ((retval = PAPI_create_eventset(&EventSet)) != PAPI_OK){
		printFunction(1,"[%s] Error creating eventset %s\n",__func__, PAPI_strerror(retval));
		exit(-1);
	}
	
	//EventSet assign rapl events
	code = PAPI_NATIVE_MASK;

	r = PAPI_enum_cmp_event( &code, PAPI_ENUM_FIRST, rapl_cid  );
	
	while ( r == PAPI_OK ) {

		retval = PAPI_event_code_to_name( code, event_names[num_events] );

		if ( retval != PAPI_OK ) {
			printFunction(1,"[%s] Error translating %#x, %s\n",__func__,code,PAPI_strerror(retval));
		}
		
		//printf("Adding %s \n",event_names[num_events]);

		retval = PAPI_get_event_info(code,&evinfo);
		if (retval != PAPI_OK) {
			printFunction(1,"[%s] Error getting event info. %s\n",__func__,PAPI_strerror(retval));
		}

		strncpy(units[num_events],evinfo.units,sizeof(units[0])-1);
		// buffer must be null terminated to safely use strstr operation on it below
		units[num_events][sizeof(units[0])-1] = '\0';

		//data_type[num_events] = evinfo.data_type;
		
		//We add the event to the eventset
		retval = PAPI_add_event( EventSet, code );
		
		if (retval != PAPI_OK) {
			printFunction(1,"[%s] Error adding eventset. %s\n",__func__,PAPI_strerror(retval));
			break;
		}
		num_events++;
  		  
		r = PAPI_enum_cmp_event( &code, PAPI_ENUM_EVENTS, rapl_cid );
	}
	
	
	
	//Values to return information
	values=(long long int *)calloc(num_events,sizeof(long long));
	
	if (values==NULL) {
		printFunction(1,"[%s] No memory available. %s\n",__func__,strerror(errno));
		exit(-1);
	 }

	//Counter reset before begin
	if ((retval = PAPI_reset(EventSet)) != PAPI_OK) {
		printFunction(1,"[%s] Error in PAPI_reset %s\n",__func__, PAPI_strerror(retval));
		exit(-1);
	}
	
	if ((retval = PAPI_start(EventSet)) != PAPI_OK) {
		printFunction(1,"[%s]Error initing PAPI %s\n",__func__, PAPI_strerror(retval));
		exit(-1);
	}
	
	//File open to mmap it into memory and write results
	if ((fd=open(fileName,O_CREAT | O_RDWR, 0666)) < 0) {
		printFunction(1,"[%s] Error opening file for writing: %s\n",__func__,strerror(errno));
		exit(EXIT_FAILURE);
	}

	int result = lseek(fd, FILESIZE_RAPL-1, SEEK_SET);
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

	map = (double *)mmap(0, FILESIZE_RAPL, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED) {
		close(fd);
		printFunction(1,"[%s] Error mmapping the file %s\n",__func__,strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	int j = 0;
	
	while(!stopEnergy){
		
		start_time=PAPI_get_real_nsec();
		
		sleep(SLEEP_TIME_PAPI);
		
		//Read measures and write to shared memory region	
		if ((retval = PAPI_read(EventSet, values)) != PAPI_OK) {
			printFunction(1,"[%s] Error in PAPI_read %s\n",__func__, PAPI_strerror(retval));
			exit(-1);
		}

		if ((retval = PAPI_reset(EventSet)) != PAPI_OK) {
			printFunction(1,"[%s] Error in PAPI_reset %s\n",__func__, PAPI_strerror(retval));
			exit(-1);
		}
		
		
		after_time=PAPI_get_real_nsec();
		total_time=((double)(after_time-start_time))/1.0e9;
		
		j = 0;

		for (i = 0; i<num_events && j<NUMEVENTS_RAPL ; i++) {
		
			if (strstr(units[i],"nJ")) {
		
				map[j] = total_time;	//Time
				map[j+1] = (double)values[i]/1.0e9;	//Jules
				//map[j+2] = ((double)values[i]/1.0e9)/total_time;
			
				j+=2;
			}
		}
		
		for(; j<NUMEVENTS_RAPL;j++) {
		
			map[j] = 0.0;
		
		}
		
	}
	
	

	//Finishing
	if ((retval = PAPI_stop(EventSet,values)) != PAPI_OK) {
		printFunction(1,"[%s] Error stopping PAPI %s\n",__func__, PAPI_strerror(retval));
		exit(-1);
	}
	
	//Cleanup
	if ((retval = PAPI_cleanup_eventset(EventSet)) != PAPI_OK) {
		printFunction(1,"[%s] Error cleaning up EventSet %s\n",__func__, PAPI_strerror(retval));
		exit(-1);
	}
	if ((retval = PAPI_destroy_eventset(&EventSet)) != PAPI_OK) {
		printFunction(1,"[%s] Error destroying EventSet %s\n",__func__, PAPI_strerror(retval));
		exit(-1);
	}

	//Unmapping
	if (munmap(map, FILESIZE_RAPL) == -1) {
		printFunction(1,"[%s] Error un-mmapping the file %s\n",__func__,strerror(errno));
	}

	close(fd);

	if( remove( fileName ) != 0 )
		printFunction(1,"[%s] Error deleting file %s\n",__func__,strerror(errno));

}
