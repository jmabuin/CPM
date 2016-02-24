/**
  * Copyright 2015 José Manuel Abuín Mosquera <josemanuel.abuin@usc.es>
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

#include <sys/procfs.h>

#include <proc/readproc.h>

#include <pwd.h>
#include <regex.h>
#include <proc/sysinfo.h>
#include <syslog.h>
#include <vector>
#include <signal.h>
#include <pthread.h>
#include <map>
#include <cstdlib>
#include <sys/wait.h> 
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "Network.h"


typedef struct processesRunning {
	pid_t PID;
	char userName[256];
	char processName[MAX_PROCESS_NAME];

} processesRunning;

int stopAgent = 0;
sig_atomic_t child_exit_status;

static void synch_signal (int sig, siginfo_t *info, void *data) {

	stopAgent = 1;
	//fprintf(stderr,"[%s] Receiving signal\n",__func__);
	syslog(LOG_INFO,"[%s] Receiving signal\n",__func__);
	
}

void stopAgentHandler(int num){

	stopAgent = 1;
	//fprintf(stderr,"[%s] Receiving signal and stoping agent\n",__func__);
	syslog(LOG_INFO, "[%s] Receiving signal and stoping agent\n",__func__);
	
}


void handleChildren(int sigNum) {
	
	/* Clean up the child process.  */ 
	int status; 
	//fprintf(stderr,"[%s] Cleaning up children process\n",__func__);
	syslog(LOG_INFO, "[%s] Cleaning up children process\n",__func__);
	wait(&status); 

	/* Store its exit status in a global variable.  */ 

	child_exit_status = status; 
}

void *stopAgentFunction(void *ptr) {
	
	//Waits for signal and when arrives, stopAgent=1
	
	//unsigned int agentId = (*(unsigned int *)ptr);
	
	struct sigaction usr_action;
	//sigset_t block_mask;
	
	sigemptyset (&usr_action.sa_mask);
	//usr_action.sa_handler = synch_signal;
	usr_action.sa_sigaction = synch_signal;
	usr_action.sa_flags = SA_SIGINFO;
	sigaction (SIGUSR1, &usr_action, NULL);
	
	sigsuspend(&usr_action.sa_mask);
	
	return NULL;

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
	
	//Regular expression to find processes
	regex_t regex;
	int reti;
	
	reti = regcomp(&regex, processNameRE, 0);
	if (reti) {
		//fprintf(stderr, "Could not compile regex\n");
		syslog(LOG_ERR,"[%s] Could not compile regex in %d\n",__func__,getpid());
		exit(1);
	}
	
	//regex self_regex(processNameRE, regex_constants::ECMAScript | regex_constants::icase);

	unsigned long int messageNumber = 0;
	unsigned long int measureNumber = 0;

	msgToSend.packageId = PACKAGE_ID_DATAMSG;

	//Create thread to wait for signal to stop the agent
	/*
	pthread_t threadId;
	
	pthread_create(&threadId,NULL,&stopAgentFunction,(void *)&agent_id);
	*/
	
	signal(SIGUSR1, stopAgentHandler);

	syslog(LOG_INFO,"[%s] New agent created %d %lu\n",__func__,getpid(),strlen(processStartsWith));

	while(!stopAgent ){
		
		//Iterate over the processes table
		proc_tab = readproctab(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS | PROC_FILLCOM | PROC_FILLENV);

		procTabIndex = 0;
		seconds_since_boot = uptime(0,0);
		
		//Clear processes pCPU map
		processesCPU.clear();

		while((proc_info = proc_tab[procTabIndex])!=NULL){

			//Store the current process name into the processName variable
			
			if(proc_info->cmdline) {
				char *tmpCmd = proc_info->cmdline[0];
				//char *space = " ";
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
						
						//fprintf(stderr,"[%s] Cmdline not null %s\n",__func__,processName);
					}
					else{
						if((strlen(processName) + strlen(tmpCmd)+1) < MAX_PROCESS_NAME) {
							strcat(processName, space);
							strcat(processName, tmpCmd);
						}
						//fprintf(stderr,"[%s] Cmdline not null %s%d\n",__func__,processName,strlen(processName));
						
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
				
				//fprintf(stderr,"[%s] Cmdline is null %d\n",__func__,proc_info->tid);
				
			}
			//fprintf(stderr,"[%s] Processname %s%d\n",__func__,processName,strlen(processName));
			//sprintf(processName,"%s",proc_info->cmd);
			
			//We check if the process belongs to our user, the process fulfills the regular expression and the CPU usage is above the threshold (currently 0)
			//if((strcmp(userName,getpwuid(proc_info->ruid)->pw_name)==0) && (cpuUsage <= (proc_info->pcpu)) && (regex_search(processName, self_regex))){
			reti = regexec(&regex, processName, 0, NULL, 0);
			
			//fprintf(stderr,"%s %s %d,%d,%d\n",userName,getpwuid(proc_info->ruid)->pw_name,strcmp(userName,getpwuid(proc_info->ruid)->pw_name),proc_info->pcpu,reti);
			
			if((strcmp(userName,getpwuid(proc_info->ruid)->pw_name)==0) && (cpuUsage <= (proc_info->pcpu)) && (strstr(processName,processNameRE)!=NULL) && strncmp(processStartsWith, processName, strlen(processStartsWith)) == 0){
				
				//fprintf(stderr,"[%s] Processname %s:%d\n",__func__,userName,strlen(processName));
				
				//We calculate the TOTAL percentage of CPU
				pcpu = 0;
				total_time = proc_info->utime + proc_info->stime;
				seconds = seconds_since_boot - proc_info->start_time / Hertz;
				if(seconds) {
					pcpu = (total_time * 1000ULL / Hertz) / seconds;
					
				}
				
				processesCPU.insert ( pair<unsigned int,unsigned int>(proc_info->tid,(total_time * 1000ULL / Hertz)) );
				
				
				//fprintf(stderr,"[%s] Sending %s %d to %s:%u\n",__func__,processName,proc_info->tid,inet_ntoa(masterIp),rxPort);
				
				//Fulfill the data to send back
				msgToSend.PID 			= proc_info->tid;
				msgToSend.agentId		= agent_id;
				msgToSend.messageNumber		= messageNumber;
				msgToSend.measureNumber		= measureNumber*SLEEP_NUM_SECS;
				msgToSend.totalCpuPercentage	= (double) (pcpu/10U) + (double) (pcpu%10U)/10.0;
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
				
				//Send the message
				sendMsgTo((void *)&msgToSend,PACKAGE_ID_DATAMSG, rxPort, inet_ntoa(masterIp));
				
				messageNumber++;
			}

			procTabIndex++;
			freeproc(proc_info);
		}
		free(proc_tab);
		measureNumber++;
		
		processesCPU_Old.clear();
		processesCPU_Old = processesCPU;
		
		//Sleep for X seconds
		sleep(SLEEP_NUM_SECS);
	}
	
	processesCPU_Old.clear();
	processesCPU.clear();

	//_exit(0);
}

int main(int argc, char **argv) {

	using namespace std;

	//Daemon init
	setlogmask(LOG_UPTO(LOG_NOTICE | LOG_INFO));
	openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
    
	syslog(LOG_INFO, "Entering Daemon MonitoringAgent");

	/* 
	 * To launch the daemon and exit
	 *
	 */

	pid_t pid, sid;

	//Fork the Parent Process
	pid = fork();

	if (pid < 0) { 
		exit(EXIT_FAILURE);
	}

	//We got a good pid, Close the Parent Process
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	if (pid == 0){
		signal(SIGCHLD, handleChildren);
	}
	//Change File Mask
	umask(0);

	//Create a new Signature Id for our child
	sid = setsid();
	
	if (sid < 0) {
		exit(EXIT_FAILURE);
	}

	//Change Directory
	//If we cant find the directory we exit with failure.
	if ((chdir("/")) < 0) {
		exit(EXIT_FAILURE);
	}

	//Close Standard File Descriptors
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	//Begin of Daemon

	pid_t currentPID;

	//Variables for network
	int rxPort = DAEMON_AGENT_PORT;
	int stopDaemon = 0;
	int nRxBytes;
	socklen_t addrLen;
	char rxBuffer[MAX_BUF_SIZE];//Actualmente 20480. En Network.h
	ProcessesInfo rxMsg;

	//We create reception address
	struct in_addr rxAddr = { INADDR_ANY };
	struct sockaddr_in myRxAddr = buildAddr(AF_INET, rxPort, rxAddr);

	
	//Creation of reception socket
	int rxSocket = createUDPSocket(1, 1, myRxAddr); //0=no broadcast, 0=no tx socket, myRxAddr especifies an addr to bind to

	//signal(SIGCHLD, handleChildren);
	
	//Running agents
	vector<struct processesRunning> runningAgents;
	
	syslog(LOG_INFO,"[%s] I am daemon agent at %s with PID: %d\n",__func__, strAddr(myRxAddr),getpid());


	while (!stopDaemon) { //We stay receiving data packages and sending them to the client

		//We stay blocked waiting for a new message
		nRxBytes = recvfrom(rxSocket, rxBuffer, sizeof(rxBuffer), 0,(struct sockaddr *) &myRxAddr, &addrLen);

		if(nRxBytes <= 0) {
			syslog(LOG_ERR,"[%s] Error in recvfrom()\n",__func__);
		}


		//Message parse
		memcpy(&rxMsg, rxBuffer, sizeof(ProcessesInfo));

		//Init of child process to monitor and send data back
		if(rxMsg.packageId == PACKAGE_ID_DATAPROCESS) {
			//fork
			if((currentPID = fork()) == 0){
				searchAndSendInfo(rxMsg);
				stopDaemon = 1;
			}
			else {//In the parent process store the new agent data
				struct processesRunning currentProcess;
				
				currentProcess.PID = currentPID;
				strcpy(currentProcess.userName,rxMsg.userName);
				strcpy(currentProcess.processName,rxMsg.processName);
				
				runningAgents.push_back(currentProcess);
				
			}
		}
		//Stop requested agents
		else if (rxMsg.packageId == PACKAGE_ID_STOP) {
			vector<unsigned int> itemsToDelete;
			for(unsigned int i = 0; i< runningAgents.size(); i++) {
				
				if ((strcmp(runningAgents.at(i).processName,rxMsg.processName)==0) && (strcmp(runningAgents.at(i).userName,rxMsg.userName) == 0 )) {
					//Stop this agent. For that, we send a signal to the process
					syslog(LOG_INFO,"[%s] Stopping agent %d\n",__func__,runningAgents.at(i).PID);
					kill(runningAgents.at(i).PID,SIGUSR1);
					
					//Delete process from vector
					itemsToDelete.push_back(i);
				}
			}
			
			for(unsigned int i = itemsToDelete.size()-1; i<= 0; i--) {
				runningAgents.erase(runningAgents.begin()+itemsToDelete.at(i)); //The argument is the position inside an iterator
			}
			
			itemsToDelete.clear();
		}


	}


	//Close the log
	closelog ();
    
	return 0;

}

