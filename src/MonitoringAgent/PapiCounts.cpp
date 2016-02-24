#include "PapiCounts.h"

//PAPI Variables
long long int contadoresPapi[NUMEVENTS];

int events[] = { PAPI_L1_DCM, PAPI_L2_DCM, PAPI_TOT_INS};
int EventSet = PAPI_NULL;
int retval = 0;
char error_str[PAPI_MAX_STR_LEN]; //To control possible errors
const PAPI_hw_info_t *hwinfo = NULL; // Hardware info structure

long long start_time,after_time;
double total_time;


int countPapi(int PID){

	int status = PAPI_NULL;
	
	char *shmName = (char*)malloc(sizeof(char)*25);
	char *fileName = (char*)malloc(sizeof(char)*30);
	
	strcpy(fileName,"/tmp/");
	strcpy(shmName,"PapiCount");
	
	char charPID[5];
	sprintf(charPID,"%d",PID);
	strcat(shmName,charPID);
	strcat(fileName,shmName);

	int fd;
	
	long long int *map;
	
	unsigned int meassuringTime = 0;
	int i=0;
	
	PAPI_event_info_t info; // Estrutura de informacion dun evento
	PAPI_option_t options;
	
	//fprintf(stdout,"PID: %d\n",PID);

	

	if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT){
		fprintf(stderr,"Error initing PAPI\n");
		exit(-1);
	}
	//Multiplexed support
	/*
	if ((retval = PAPI_multiplex_init()) != PAPI_OK){
		printf("Erro en PAPI_multiplex_init: %d\n", retval);
		exit(-1);
	}
	*/
	if ((hwinfo = PAPI_get_hardware_info()) == NULL) {
		fprintf(stderr,"Error in PAPI_get_hardware_info\n");
		exit(-1);
	}
	
	
	//PAPI_set_granularity( PAPI_GRN_MAX);
	//PAPI_set_domain(PAPI_DOM_ALL);
	
	//eventSet creation
	if ((retval = PAPI_create_eventset(&EventSet)) != PAPI_OK){
		fprintf(stderr,"Erro creando o set de eventos %d\n", retval);
		exit(-1);
	}

	/*
	memset( &options, 0x0, sizeof ( PAPI_option_t ) );
	options.inherit.inherit = PAPI_INHERIT_NONE;
	options.inherit.eventset = EventSet;
	*/
	
	
	
	//eventSet assign
	if ((retval = PAPI_assign_eventset_component(EventSet, 0)) != PAPI_OK) {
		printf("Erro en PAPI_assign_eventset_component: %d\n", retval);
		exit(-1);
	}
	/*
	if ((retval = PAPI_set_multiplex(EventSet)) != PAPI_OK){
		printf("Erro en PAPI_set_multiplex: %d\n", retval);
		exit(-1);
	}
	*/
	/*if ((retval = PAPI_thread_init(pthread_self)) != PAPI_OK){
		printf("Erro en PAPI_thread_init: %d\n", retval);
		exit(-1);
	}
	
	if((retval = PAPI_set_opt( PAPI_DOMAIN, &options )) != PAPI_OK){
		 printf("Error in PAPI_set_opt :: %s\n", PAPI_strerror(retval));
		 exit(1);
	}
	*/
	
	
	
	/*
	if((retval = PAPI_set_opt( PAPI_INHERIT, &options )) != PAPI_OK){
		printf("Error in PAPI_set_opt :: %s\n", PAPI_strerror(retval));
		exit(-1);
	}
	*/
	
	
	//Start meassuring
	if ((retval=PAPI_add_events(EventSet, events, NUMEVENTS)) != PAPI_OK){
		//PAPI_perror(retval,error_str,PAPI_MAX_STR_LEN);
		printf("Error adding events %d: %s\n",retval,PAPI_strerror(retval));
		exit(-1);
	}
	
	//Attach to PID
	if ((retval = PAPI_attach(EventSet, PID)) != PAPI_OK){
		printf("Error in PAPI_attach :: %s\n", PAPI_strerror(retval));
		exit(1);
	}
	
	// Reseteamos os contadores antes de comezar
	if ((retval = PAPI_reset(EventSet)) != PAPI_OK) {
		printf("Error in PAPI_reset %d\n", retval);
		exit(-1);
	}
	
	if ((retval = PAPI_start(EventSet)) != PAPI_OK) {
		printf("Error initing PAPI %d\n", retval);
		exit(-1);
	}
	
	
	//start_time=PAPI_get_real_nsec();

	
	//sleep(meassuringTime);
	if ((fd=open(fileName,O_CREAT | O_RDWR, 0666)) < 0) {
		perror("Error opening file for writing");
		exit(EXIT_FAILURE);
	}
	
	int result = lseek(fd, FILESIZE-1, SEEK_SET);
	if (result == -1) {
		close(fd);
		perror("Error calling lseek() to 'stretch' the file");
		exit(EXIT_FAILURE);
	}
	
	result = write(fd, "", 1);
	if (result != 1) {
		close(fd);
		perror("Error writing last byte of the file");
		exit(EXIT_FAILURE);
	}
	
	map = (long long int *)mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED) {
		close(fd);
		perror("Error mmapping the file");
		exit(EXIT_FAILURE);
	}
	
	while(kill(PID,0)==0){
		
		
		
		//Write to shm
		//if ((retval = PAPI_read(EventSet, contadoresPapi)) != PAPI_OK) {
		if ((retval = PAPI_read(EventSet, contadoresPapi)) != PAPI_OK) {
			printf("Erro en PAPI_read %d\n", retval);
			exit(-1);
		}
		
		if ((retval = PAPI_reset(EventSet)) != PAPI_OK) {
			printf("Error in PAPI_reset %d\n", retval);
			exit(-1);
		}
		
		for (i = 0; i<NUMEVENTS ; i++) {
			//PAPI_get_event_info(events[i], &info); // Obtense a informacion do evento
			//fprintf(stdout,"%s \t 0x%-10x\t %-40s \t %lld \n", info.symbol, info.event_code, info.long_descr, contadoresPapi[i]);	// Sacase por pantalla a informacion do contador hardware
			map[i] = contadoresPapi[i];
		}
		//fclose(f);
		sleep(meassuringTime);
		
	}
	
	//after_time=PAPI_get_real_nsec();
	//total_time=((double)(after_time-start_time))/1.0e9;

	//Finishing
	
	
	//Antes de rematar leemos os resultados do eventset
	/*if ((retval = PAPI_read(EventSet, contadoresPapi)) != PAPI_OK) {
		printf("Erro en PAPI_read %d\n", retval);
		exit(-1);
	}*/
	if ((retval = PAPI_stop(EventSet,contadoresPapi)) != PAPI_OK) {
		printf("Erro parando PAPI %d\n", retval);
		exit(-1);
	}
	//Dettach from PID
	if ((retval = PAPI_detach(EventSet)) != PAPI_OK){
		printf("Error in PAPI_detach :: %s\n", PAPI_strerror(retval));
		exit(1);
	}
	if ((retval = PAPI_cleanup_eventset(EventSet)) != PAPI_OK) {
		printf("Erro limpando o EventSet %d\n", retval);
		exit(-1);
	}
	if ((retval = PAPI_destroy_eventset(&EventSet)) != PAPI_OK) {
		printf("Erro destruindo o EventSet %d\n", retval);
		exit(-1);
	}

	if (munmap(map, FILESIZE) == -1) {
		perror("Error un-mmapping the file");
		/* Decide here whether to close(fd) and exit() or not. Depends... */
	}

	close(fd);

	//printf("Time: %.8f\n",total_time);
	printf ("\nPAPI Hardware counters\n");
	printf("Name \t\t Code \t\t Description \t\t\t\t\t Total \n");
	
	
	
	for (i = 0; i<NUMEVENTS ; i++) {
		PAPI_get_event_info(events[i], &info); // Obtense a informacion do evento
		fprintf(stdout,"%s \t 0x%-10x\t %-40s \t %lld \n", info.symbol, info.event_code, info.long_descr, contadoresPapi[i]);	// Sacase por pantalla a informacion do contador hardware
	}



	return 0;

}
