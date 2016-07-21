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

//#include <sys/procfs.h>
//#include <pwd.h>
//#include <sys/types.h>
//#include <unistd.h>
//#include <errno.h>
//#include "Globals.h"

#include <vector>
#include <sys/wait.h> 
#include <syslog.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "Network.h"
#include "AgentChild.h"


using namespace std;

sig_atomic_t child_exit_status;

void handleChildren(int sigNum) {
	
	/* Clean up the child process.  */ 
	int status; 
	printFunction(0,"[%s] Cleaning up children process\n",__func__);
	wait(&status); 

	/* Store its exit status in a global variable.  */ 

	child_exit_status = status; 
}


static int usage()
{
	fprintf(stderr, "\n");
	fprintf(stderr, "Program: MonitoringMaster (Monitor to bridge cluster master node and computing nodes)\n");
	fprintf(stderr, "Version: %s\n", PACKAGE_VERSION);
	fprintf(stderr, "Contact: José M. Abuín <josemanuel.abuin@usc.es>\n\n");
	fprintf(stderr, "Usage:   MonitoringMaster [options]\n\n");
	fprintf(stderr, "Common options:\n");
	fprintf(stderr, "\n");

	fprintf(stderr, " -d		Debug mode. Default: False.\n");
	fprintf(stderr, " -h		Print this help.\n");

	fprintf(stderr, "\n");
	
	fprintf(stderr, "Network options:\n");
	fprintf(stderr, "\n");

	fprintf(stderr, " -a INT        Port where the agent is going to run in the computing nodes. Default: 20000.\n");
	fprintf(stderr, " -m INT        Port where the master monitor (this program) is going to run master node. Default: 8000.\n");
	fprintf(stderr, " -c INT        Port where the client program is going to run. Default: 10000.\n");
	fprintf(stderr, "\n");
	return 1;
}



int main(int argc, char **argv) {


	// Parse options
	int 			option;
	int			master_port = 0;
	int			agent_port = 0;
	int			client_port = 0;
	
	while ((option = getopt(argc, argv,"dha:m:c:")) >= 0) {
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
				
			case 'm':
				master_port = atoi(optarg);
				break;
				
			case 'c':
				client_port = atoi(optarg);
				
			default: break;
			
		}
	}
	
	//Daemon init
	if(!getDebugMode()) {
		setlogmask(LOG_UPTO(LOG_NOTICE | LOG_INFO));
		openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
	}
    
	// printFunction(0, "Entering Daemon MonitoringMaster\n");
	printFunction(0, "[%s] Entering Daemon MonitoringMaster\n", __func__);

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

	if (pid == 0){ //Child process
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

	//Close Standard File Descriptors if not in debug mode
	if(!getDebugMode()) {
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	}
	//Begin of Daemon
	int stopDaemon = 0;

	//Variables for network
	
	int rxPort = MASTER_BASE_PORT;
	
	if(master_port != 0) {
		rxPort = master_port;
	}
	
	vector<unsigned int> availablePorts;
	vector<unsigned int> busyPorts;

	for ( unsigned int numPort = MAX_MASTER_MONITORS; numPort <= 1; numPort--) {
		
		availablePorts.push_back(numPort);
	
	}

	//Creation of reception address
	struct in_addr rxAddr = { INADDR_ANY };
	struct sockaddr_in myRxAddr = buildAddr(AF_INET, rxPort, rxAddr);

	
	//Creation of reception socket
	int rxSocket = createUDPSocket(1, 1, myRxAddr); //0=no broadcast, 0=no tx socket, myRxAddr especifies an addr to bind to

	//signal(SIGCHLD, handleChildren);
	
	
	printFunction(0,"[%s] I am master node at %s\n", strAddr(myRxAddr),__func__);

	int nRxBytes;
	socklen_t addrLen;
	char rxBuffer[MAX_BUF_SIZE]; //Currently 20480. In Network.h
	ProcessesInfo rxMsg;


	while (!stopDaemon) { //We stay receiving data packages and sending them to the client

		//We stay blocked waiting for a new message
		nRxBytes = recvfrom(rxSocket, rxBuffer, sizeof(rxBuffer), 0,(struct sockaddr *) &myRxAddr, &addrLen);
		if(nRxBytes <= 0) {
			printFunction(1,"[%s] Error in recvfrom()\n",__func__);
		}

		//Message parse
		memcpy(&rxMsg, rxBuffer, sizeof(ProcessesInfo));
		
		if(rxMsg.packageId == PACKAGE_ID_DATAPROCESS) {
	
			printFunction(0,"[%s] Received message\n",__func__);
			
			//rxMsg.port = availablePorts.back();
			//availablePorts.pop_back();
	
			//Creation of new process to monitor communications
			if(fork() == 0){
				//printFunction(0,"[%s] Forked process\n",__func__);
				createMasterMonitor(client_port, master_port, agent_port, rxMsg);
				stopDaemon = 1;
			}
		
		}
		else{
			printFunction(1,"[%s] Package id not recognized %u\n",__func__,rxMsg.packageId);
		}
	}

	//Close the log
	if(!getDebugMode()) {
		closelog ();
	}
	return 0;
}

