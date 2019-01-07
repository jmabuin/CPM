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

typedef struct processesRunning {
	pid_t PID;
	char userName[256];
	char processName[MAX_PROCESS_NAME];

} processesRunning;


sig_atomic_t child_exit_status;


void handleChildren(int sigNum) {

	/* Clean up the child process.  */
	int status;
	//fprintf(stderr,"[%s] Cleaning up children process\n",__func__);
	printFunction(0, "[%s] Cleaning up children process\n",__func__);
	wait(&status);

	/* Store its exit status in a global variable.  */

	child_exit_status = status;
}


static int usage()
{
	fprintf(stderr, "\n");
	fprintf(stderr, "Program: MonitoringAgent (Monitor to processes information in the computing nodes)\n");
	fprintf(stderr, "Version: %s\n", PACKAGE_VERSION);
	fprintf(stderr, "Contact: José M. Abuín <josemanuel.abuin@usc.es>\n\n");
	fprintf(stderr, "Usage:   MonitoringAgent [options]\n\n");
	fprintf(stderr, "Common options:\n");
	fprintf(stderr, "\n");

	fprintf(stderr, " -d		Debug mode. Default: False.\n");
	fprintf(stderr, " -h		Print this help.\n");

	fprintf(stderr, "\n");
	
	fprintf(stderr, "Network options:\n");
	fprintf(stderr, "\n");

	fprintf(stderr, " -a INT        Port where the agent (this program) is going to run in the computing nodes. Default: 20000.\n");
	fprintf(stderr, "\n");
	return 1;
}

int main(int argc, char **argv) {

	using namespace std;

	// Parse options
	int 			option;
	int			agent_port 		= 0;

	
	while ((option = getopt(argc, argv,"dha:")) >= 0) {
		switch (option) {
			case 'd' :
				setDebugMode(1);
				break;
			
			case 'h':
				usage();
				exit(0);
			
			case 'a':
				agent_port = atoi(optarg);
				break;
				
				
			default: break;
			
		}
	}

	//Daemon init
	if(!getDebugMode()) {
		setlogmask(LOG_UPTO(LOG_NOTICE | LOG_INFO));
		openlog(AGENT_DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
	}
	
	printFunction(0, "[%s] Entering Daemon MonitoringAgent\n",__func__);

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
	if(!getDebugMode()) {
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	}

	//Begin of Daemon
	pid_t currentPID;

	//Variables for network
	int rxPort = DAEMON_AGENT_PORT;
	
	if(agent_port != 0) {
		rxPort = agent_port;
	}
	
	
	
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

	printFunction(0,"[%s] I am daemon agent at %s with PID: %d\n",__func__, strAddr(myRxAddr),getpid());


	while (!stopDaemon) { //We stay receiving data packages and sending them to the client

		//We stay blocked waiting for a new message
		nRxBytes = recvfrom(rxSocket, rxBuffer, sizeof(rxBuffer), 0,(struct sockaddr *) &myRxAddr, &addrLen);

		if(nRxBytes <= 0) {
			printFunction(1,"[%s] Error in recvfrom()\n",__func__);
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
					printFunction(0,"[%s] Stopping agent %d\n",__func__,runningAgents.at(i).PID);
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
	if(!getDebugMode()) {
		closelog ();
	}
	
	return 0;

}

