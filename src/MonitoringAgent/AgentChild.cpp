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


#include "AgentChild.h"
#include "Globals.h"

int stopAgent = 0;

void stopAgentHandler(int num){

	stopAgent = 1;
	//fprintf(stderr,"[%s] Receiving signal and stoping agent\n",__func__);
	syslog(LOG_INFO, "[%s] Receiving signal and stoping agent\n",__func__);

}

int vectorContainsPid(std::vector<unsigned int> pidVector, unsigned int PID){

	unsigned int i = 0;

	for( i = 0; i< pidVector.size(); i++) {
		if(pidVector[i] == PID){
			return 1;
		}
	}

	return 0;
}

void searchAndSendInfo(struct ProcessesInfo rxMsg) {

	using namespace std;

	//Variables for network
	//Save address and port of the received message
	struct in_addr masterIp = rxMsg.from.sin_addr;
	unsigned int rxPort = rxMsg.from.sin_port;

	//Variable for store data to send
	struct Agent2MasterDataMsg msgToSend;

	//To store this agent id
	unsigned int agent_id = rxMsg.agentId;

	//To check if the user wants to measure PAPI performance
	static int measurePapi = rxMsg.measurePapi;

	//To check if the user wants to measure energy with PAPI and RAPL
	static int measureEnergy = rxMsg.measureEnergy;
	
	//Max number fo shared memory regions for PAPI measurement
	//int maxShm = smp_num_cpus;

	//Map to store the shared memory regions <PID,long long int[3]>
	std::map<unsigned int,long long int*> shmRegionsData;
	//The same, but with the file descriptors
	std::map<unsigned int,int> fdRegionsData;
	
	//Another map that stores the PID of the process being monitorized as key and the related PID of the process monitoring the PAPI counters as value
	std::map<unsigned int, unsigned int> pidsPapi;

	//File descriptor for the shm region for RAPL
	int fd_rapl = -1;
	
	//Shared region memory for RAPL
	double *shm_rapl = NULL;

	pid_t childPidRAPL = 0;
	//Variables to get process information
	unsigned int cpuUsage = 0;

	char userName[1024];// = (char*)calloc(1024,sizeof(char));
	char processName[MAX_PROCESS_NAME];// = (char*)calloc(1024,sizeof(char));
	char *processNameRE = (char*)calloc(MAX_PROCESS_NAME,sizeof(char));
	char processStartsWith[MAX_PROCESS_NAME];

	//char *processNameFull = (char*)calloc(1024,sizeof(char));


	strcpy(userName,rxMsg.userName);
	strcpy(processStartsWith,rxMsg.processStartsWith);
	//strcpy(processName,rxMsg.processName);

	strcpy(processNameRE,rxMsg.processName);

	//Variables to calculate pcpu -> % CPU
	unsigned long long seconds;
	unsigned pcpu = 0;
	unsigned long long total_time;

	unsigned long seconds_since_boot = -1;

	int procTabIndex = 0;

	//Variables to iterate over the processes in this node
	proc_t *proc_info;// = (proc_t *)malloc(sizeof(proc_t));
	proc_t **proc_tab;
	//PROCTAB* proc;

	//Map table to store PID, Seconds of CPU and calculate percentage of used CPU in the next iteration
	map<unsigned int,unsigned int> processesCPU;
	map<unsigned int,unsigned int> processesCPU_Old;


	std::vector<unsigned int> pidsInUse;

	unsigned long int messageNumber = 0;
	unsigned long int measureNumber = 0;

	msgToSend.packageId = PACKAGE_ID_DATAMSG;

	//We listen into the signal to stop the agent

	signal(SIGUSR1, stopAgentHandler);

	syslog(LOG_INFO,"[%s] New agent created %d %lu\n",__func__,getpid(),strlen(processStartsWith));

	while(!stopAgent ){

		//Iterate over the processes table
		proc_tab = readproctab(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS | PROC_FILLCOM | PROC_FILLENV | PROC_LOOSE_TASKS);

		procTabIndex = 0;
		seconds_since_boot = uptime(0,0);

		//Clear processes pCPU map
		processesCPU.clear();

		if(measurePapi){
			pidsInUse.clear();
		}

		//To get the memory information into global variables
		meminfo();

		while((proc_info = proc_tab[procTabIndex])!=NULL){
			

			//Store the current process name into the processName variable

			if(proc_info->cmdline) {
				char *tmpCmd = proc_info->cmdline[0];

				char space[1];
				strcpy(space," ");

				int j = 0;

				while(tmpCmd) {

					if(j == 0) {
						if(strlen(tmpCmd) < MAX_PROCESS_NAME) {
							sprintf(processName,"%s",tmpCmd);
						}
						else{
							memcpy(processName,tmpCmd,MAX_PROCESS_NAME-1);
							processName[MAX_PROCESS_NAME-1] = '\0';
						}


					}
					else{
						if((strlen(processName) + strlen(tmpCmd)+1) < MAX_PROCESS_NAME) {
							strcat(processName, space);
							strcat(processName, tmpCmd);
						}


					}

					j++;
					tmpCmd = proc_info->cmdline[j];
				}
			}
			else{
				if(strlen(proc_info->cmd)  < MAX_PROCESS_NAME) {
					sprintf(processName,"%s",proc_info->cmd);
				}
				else {
					memcpy(processName,proc_info->cmd,MAX_PROCESS_NAME-1);
					processName[MAX_PROCESS_NAME-1] = '\0';
				}

			}

			//We check if the process belongs to our user, the process fulfills the regular expression and the CPU usage is above the threshold (currently 0)

			if((strcmp(userName,getpwuid(proc_info->ruid)->pw_name)==0) && (cpuUsage <= (proc_info->pcpu)) && (strstr(processName,processNameRE)!=NULL) && strncmp(processStartsWith, processName, strlen(processStartsWith)) == 0){


				//We calculate the TOTAL percentage of CPU
				pcpu = 0;
				total_time = proc_info->utime + proc_info->stime;
				seconds = seconds_since_boot - proc_info->start_time / Hertz;
				if(seconds) {
					pcpu = (total_time * 1000ULL / Hertz) / seconds;

				}

				processesCPU.insert ( pair<unsigned int,unsigned int>(proc_info->tid,(total_time * 1000ULL / Hertz)) );

				//Check if PAPI counter have to be measured
				if(measurePapi){

					pidsInUse.push_back(proc_info->tid);


					//Check if the shared memory region exists
					if(shmRegionsData.count(proc_info->tid) <= 0){

						//The shared memory region has to be created and the PAPI monitor launched
						//syslog(LOG_INFO,"[%s] Creating shared memory region\n",__func__);

						pid_t childPid;

						//Fork and launch countPapi
						childPid = fork();

						if (childPid < 0) {
							exit(EXIT_FAILURE);
						}

						//We got a good pid, Close the Parent Process
						if (childPid == 0) {
							syslog(LOG_INFO,"[%s] Launching PAPI monitor\n",__func__);
							countPapi(proc_info->tid);
							exit(EXIT_SUCCESS);
						}






						char *shmName = (char*)malloc(sizeof(char)*25);
						char *fileName = (char*)malloc(sizeof(char)*30);

						strcpy(fileName,"/tmp/");
						strcpy(shmName,"PapiCount");

						char charPID[5];
						sprintf(charPID,"%d",proc_info->tid);
						strcat(shmName,charPID);
						strcat(fileName,shmName);

						int fd;

						long long int *map;

						fd = open(fileName, O_RDONLY | O_CREAT , 0666);
    						if (fd == -1) {
							syslog(LOG_ERR,"[%s] Error opening file for reading: %s\n",__func__,strerror(errno));
							exit(EXIT_FAILURE);
    						}
						//syslog(LOG_INFO,"[%s] File opened\n",__func__);
						map = (long long int *)mmap(0, FILESIZE, PROT_READ, MAP_SHARED, fd, 0);
						if (map == MAP_FAILED) {
							close(fd);
							syslog(LOG_ERR,"[%s] Error mmapping the file\n",__func__);
							exit(EXIT_FAILURE);
						}
						//syslog(LOG_INFO,"[%s] Region mapped\n",__func__);
						shmRegionsData.insert(std::pair<unsigned int,long long int*>(proc_info->tid,map));

						//We save the file descriptor, so we will be able to close it later
						fdRegionsData.insert(std::pair<unsigned int,int> (proc_info->tid,fd));
						pidsPapi.insert(std::pair<unsigned int,unsigned int>(proc_info->tid,childPid));

						msgToSend.papiMeasures[0] = 0;
						msgToSend.papiMeasures[1] = 0;
						msgToSend.papiMeasures[2] = 0;

					}
					else{
						//syslog(LOG_INFO,"[%s] Fulfilling PAPI data\n",__func__);

						msgToSend.papiMeasures[0] = shmRegionsData.at(proc_info->tid)[0];
						msgToSend.papiMeasures[1] = shmRegionsData.at(proc_info->tid)[1];
						msgToSend.papiMeasures[2] = shmRegionsData.at(proc_info->tid)[2];

					}

					//syslog(LOG_INFO,"[%s] PAPI data fulfilled\n",__func__);

				}

				//Fulfill the data to send back
				//syslog(LOG_INFO,"[%s] Fulfilling all data\n",__func__);
				msgToSend.PID 			= proc_info->tid;
				msgToSend.agentId		= agent_id;
				msgToSend.messageNumber		= messageNumber;
				msgToSend.measureNumber		= measureNumber*SLEEP_NUM_SECS;
				msgToSend.totalCpuPercentage	= (double) (pcpu/10U) + (double) (pcpu%10U)/10.0;
				msgToSend.numberOfThreads	= proc_info->nlwp;
				msgToSend.cpuPercentage = 0;

				if(processesCPU_Old.count(proc_info->tid) > 0) {
					double cpu2 = (double) (processesCPU[proc_info->tid]/10U) + (double) (processesCPU[proc_info->tid]%10U)/10.0;
					double cpu1 = (double) (processesCPU_Old[proc_info->tid]/10U) + (double) (processesCPU_Old[proc_info->tid]%10U)/10.0;


					msgToSend.cpuPercentage = (cpu2 - cpu1) / SLEEP_NUM_SECS;
					//fprintf(stderr,"[%s] Secs %f %f\n",__func__,cpu2,cpu1);
				}
				else{
					msgToSend.cpuPercentage = msgToSend.totalCpuPercentage;
				}

				strcpy(msgToSend.userName,getpwuid(proc_info->ruid)->pw_name);
				//strcpy(msgToSend.processName, proc_info->cmd);
				strcpy(msgToSend.processName, processName);
				msgToSend.memory = proc_info->vm_rss;
				msgToSend.memoryPercentage = (msgToSend.memory*100.0)/kb_main_total;

				//Send the message
				sendMsgTo((void *)&msgToSend,PACKAGE_ID_DATAMSG, rxPort, inet_ntoa(masterIp));
				//syslog(LOG_INFO,"[%s] Process data package %lu sent to %s:%d\n",__func__,msgToSend.messageNumber,inet_ntoa(masterIp),rxPort);
				messageNumber++;
			}

			procTabIndex++;
			freeproc(proc_info);
		}
		free(proc_tab);
		
		//If the user wants to measure energy
		if(measureEnergy){
		
			Agent2MasterEnergyMsg energyMsg;
		
			if(measureNumber == 0) {
			
				//Create the SHM region and launch fork
				
				//The shared memory region has to be created and the PAPI monitor launched
				//syslog(LOG_INFO,"[%s] Creating shared memory region\n",__func__);

				pid_t childPid;

				//Fork and launch countPapi
				childPid = fork();

				if (childPid < 0) {
					exit(EXIT_FAILURE);
				}

				//We got a good pid, Close the Parent Process
				if (childPid == 0) {
					syslog(LOG_INFO,"[%s] Launching Energy monitor\n",__func__);
					//countPapi(proc_info->tid);
					measureEnergyFunction();
					exit(EXIT_SUCCESS);
				}
				
				//We store the child process PID
				childPidRAPL = childPid;

				char *shmName = (char*)malloc(sizeof(char)*25);
				char *fileName = (char*)malloc(sizeof(char)*30);

				strcpy(fileName,"/tmp/");
				strcpy(shmName,"EnergyCount");

				char charPID[5];
				sprintf(charPID,"%d",childPidRAPL);
				strcat(shmName,charPID);
				strcat(fileName,shmName);

				int fd;
				

				fd = open(fileName, O_RDONLY | O_CREAT , 0666);
				if (fd == -1) {
					syslog(LOG_ERR,"[%s] Error opening file for reading: %s\n",__func__,strerror(errno));
					exit(EXIT_FAILURE);
				}
				//syslog(LOG_INFO,"[%s] File opened\n",__func__);
				shm_rapl = (double *)mmap(0, FILESIZE_RAPL, PROT_READ, MAP_SHARED, fd, 0);
				if (shm_rapl == MAP_FAILED) {
					close(fd);
					syslog(LOG_ERR,"[%s] Error mmapping the file\n",__func__);
					exit(EXIT_FAILURE);
				}
				
				//We save the file descriptor, so we will be able to close it later
				fd_rapl = fd;



				energyMsg.packageId 		= PACKAGE_ID_ENERGY;
				energyMsg.PID 			= childPidRAPL;
				energyMsg.agentId		= agent_id;
				energyMsg.messageNumber		= messageNumber;
				energyMsg.measureNumber		= measureNumber*SLEEP_NUM_SECS;
				strcpy(energyMsg.userName, userName);
				//energyMsg.destinationNode

				int j = 0;
				
				//The region is just inizializated, so it still has no values
				for ( j = 0; j< NUMEVENTS_RAPL ; j++) {
				
					energyMsg.energyMeasures[j] = 0.0;
				}
				
				
			
			}
			else {
			
				energyMsg.packageId 		= PACKAGE_ID_ENERGY;
				energyMsg.PID 			= childPidRAPL;
				energyMsg.agentId		= agent_id;
				energyMsg.messageNumber		= messageNumber;
				energyMsg.measureNumber		= measureNumber*SLEEP_NUM_SECS;
				strcpy(energyMsg.userName, userName);
			
				//Read from the SHM region
				int j = 0;
				
				//The region is just inizializated, so it still has no values
				for ( j = 0; j< NUMEVENTS_RAPL ; j++) {
				
					energyMsg.energyMeasures[j] = shm_rapl[j];
				}
			}
			
			//Send energy PKG
			sendMsgTo((void *)&energyMsg,PACKAGE_ID_ENERGY, rxPort, inet_ntoa(masterIp));
			//syslog(LOG_INFO,"[%s] Energy package %lu ID: %u sent to %s:%d\n",__func__,energyMsg.messageNumber,energyMsg.packageId,inet_ntoa(masterIp),rxPort);

			messageNumber++;
		}
		
		
		measureNumber++;

		processesCPU_Old.clear();
		processesCPU_Old = processesCPU;

		//If measuring PAPI, check if shared memory regions have to be closed
		if(measurePapi){
			//Loop over map with shared memory regions. If the PID is not in the used PIDs vector, close the region and the file descriptor

			std::map<unsigned int,long long int * >::iterator iteratorShmRegionsData = shmRegionsData.begin();
			std::map<unsigned int, int>::iterator iteratorFdRegionsData = fdRegionsData.begin();

			for( iteratorShmRegionsData = shmRegionsData.begin(); iteratorShmRegionsData != shmRegionsData.end(); iteratorShmRegionsData++) {

				if (!vectorContainsPid(pidsInUse, iteratorShmRegionsData->first)) {
					//Unmap the shm region and close fd
					if (munmap(iteratorShmRegionsData->second, FILESIZE) == -1) {
						syslog(LOG_ERR,"[%s] Error un-mmapping the file\n",__func__);
    					}
    					close(iteratorFdRegionsData->second);

					//Send signal to the process counting PAPI counters

					kill(pidsPapi[iteratorShmRegionsData->first],SIGUSR1);

				}


				iteratorFdRegionsData++;

			}

		}

		//Sleep for X seconds
		sleep(SLEEP_NUM_SECS);
	}


	//If measuring PAPI, check if shared memory regions have to be closed
	if(measurePapi){
		//Loop over map with shared memory regions. If the PID is not in the used PIDs vector, close the region and the file descriptor

		std::map<unsigned int,long long int * >::iterator iteratorShmRegionsData = shmRegionsData.begin();
		std::map<unsigned int, int>::iterator iteratorFdRegionsData = fdRegionsData.begin();

		for( iteratorShmRegionsData = shmRegionsData.begin(); iteratorShmRegionsData != shmRegionsData.end(); iteratorShmRegionsData++) {


			//Unmap the shm region and close fd
			if (munmap(iteratorShmRegionsData->second, FILESIZE) == -1) {
				perror("Error un-mmapping the file");
			}

			close(iteratorFdRegionsData->second);

			//Send signal to the process counting PAPI counters

			kill(pidsPapi[iteratorShmRegionsData->first],SIGUSR1);




			iteratorFdRegionsData++;

		}

	}

	//Stop the measure energy process if needed
	if(measureEnergy) {
		
		//Unmap the shm region and close fd
		if (munmap(shm_rapl, FILESIZE_RAPL) == -1) {
			perror("Error un-mmapping the file");
		}

		close(fd_rapl);

		//Send signal to the process counting PAPI counters

		kill(childPidRAPL,SIGUSR1);
			
	}


	processesCPU_Old.clear();
	processesCPU.clear();

	//_exit(0);
}
