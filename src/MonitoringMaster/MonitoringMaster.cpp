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
#include <pwd.h>
#include <vector>
#include <unistd.h>
#include <sys/wait.h> 
#include <syslog.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "Network.h"

using namespace std;

sig_atomic_t child_exit_status;

void handleChildren(int sigNum) {
	
	/* Clean up the child process.  */ 
	int status; 
	syslog(LOG_INFO,"[%s] Cleaning up children process\n",__func__);
	wait(&status); 

	/* Store its exit status in a global variable.  */ 

	child_exit_status = status; 
}

void createMasterMonitor(struct ProcessesInfo processInfo) {
	
	//Struct to get info from the user and listen in the correct port
	struct passwd *userInfo;
	
	//Variables for network
	int nRxBytes;
	int error;
	struct addrinfo *result;
	
	socklen_t addrLen;
	char rxBuffer[MAX_BUF_SIZE];//Currently 20480. In Network.h
	
	//Message to send data do Client
	Agent2MasterDataMsg rxMsg;
	
	//Message to senf info data to client
	ProcessesInfo newProcess;
	
	//To store IPs, username and process to be monitorized. Using this, we will be able to stop them in the computing node
	vector<struct in_addr> agentsAddresses;


	//To avoid a warning
	int length = strlen(NETWORK_INTERFACE);
	char networkInterface[length];
	
	length = strlen(NETWORK_INTERFACE_INTERNAL);
	char networkInterfaceInternal[length];

	//Get the current user ID
	userInfo = getpwnam(processInfo.userName);
	
	uid_t userId = userInfo->pw_uid;

	//Set the port where we are going to listen
	//int rxPort = MASTER_BASE_PORT + userId;
	
	int rxPort = MASTER_BASE_PORT + processInfo.port;

	newProcess.port = rxPort;
	newProcess.from.sin_port = rxPort;
	
	struct in_addr addresFromClient = processInfo.from.sin_addr;
	
	//Set the network interface to use. Defined in Network.h
	strcpy(networkInterface,NETWORK_INTERFACE);
	strcpy(networkInterfaceInternal,NETWORK_INTERFACE_INTERNAL);
	
	newProcess.from.sin_addr = getOwnIp(networkInterfaceInternal);
	
	//struct in_addr myInternalAddress = getOwnIp(networkInterfaceInternal);
	struct in_addr myInternalAddress = newProcess.from.sin_addr;
	
	//memcpy(&myInternalAddress,&(newProcess.from.sin_addr),sizeof(struct in_addr));
	
	processInfo.port = rxPort;
	processInfo.from.sin_port = rxPort;
	processInfo.from.sin_addr = getOwnIp(networkInterface);
	
	//Creation of reception address
	struct in_addr rxAddr = { INADDR_ANY };
	struct sockaddr_in myRxAddr = buildAddr(AF_INET, rxPort, rxAddr);

	//Creation of reception socket
	int rxSocket = createUDPSocket(1, 1, myRxAddr); //0=no broadcast, 0=no tx socket, myRxAddr especifies an addr to bind to
	syslog(LOG_INFO,"[%s] Creating MasterMonitor %s\n",__func__, strAddr(myRxAddr));
	syslog(LOG_INFO,"[%s] Internal address is %s\n",__func__, inet_ntoa(myInternalAddress));
	syslog(LOG_INFO,"[%s] External address is %s\n",__func__, inet_ntoa(processInfo.from.sin_addr));

	//We send ACK to client with info. Client must know new port to send the Agents info
	
	syslog(LOG_INFO,"[%s] Messaging back to Client %s:%u\n",__func__, inet_ntoa(addresFromClient),CLIENT_BASE_PORT);
	
	if(sendMsgTo((void *)&processInfo, PACKAGE_ID_DATAPROCESS, CLIENT_BASE_PORT, inet_ntoa(addresFromClient)) == -1){
	
		syslog(LOG_ERR,"[%s] Error sending message back to Client %s:%u\n",__func__, inet_ntoa(addresFromClient),CLIENT_BASE_PORT);
	}
	else {
		syslog(LOG_INFO,"[%s] Message sent back to Client %s:%u\n",__func__, inet_ntoa(addresFromClient),CLIENT_BASE_PORT);
	}

	int i = 0 ;
	
	int stopMaster = 0;
	
	while (!stopMaster) { //We stay receiving data packages and sending them to the client



		//We stay blocked waiting for a new message
		nRxBytes = recvfrom(rxSocket, rxBuffer, sizeof(rxBuffer), 0,(struct sockaddr *) &myRxAddr, &addrLen);
		
		if(nRxBytes <= 0) {
			syslog(LOG_ERR,"[%s] Error in recvfrom()\n",__func__);
		}
		
		//Parse message
		memcpy(&rxMsg, rxBuffer, sizeof(Agent2MasterDataMsg));

		syslog(LOG_INFO,"[%s] Received message %d type %u\n",__func__,i,rxMsg.packageId);
		i++;
		

		//This branch to launch a new agent
		if(rxMsg.packageId == PACKAGE_ID_DATAPROCESS) {
			//Launch a new agent in computing node
			
			memcpy(&newProcess, rxBuffer, sizeof(ProcessesInfo));
			
			//Set the user id and the info from where the message has been sent
			newProcess.uid = userId;
			newProcess.port = rxPort;
			newProcess.from.sin_port = rxPort;
			newProcess.from.sin_addr = myInternalAddress;
			//memcpy(&(newProcess.from.sin_addr),&myInternalAddress,sizeof(struct in_addr));

			//Retrieve IP address from nodeName
			error = getaddrinfo(newProcess.nodeName, NULL, NULL, &result);
			
			if (error == 0){
				struct in_addr destinationIp = ((struct sockaddr_in *)result->ai_addr)->sin_addr;
				//fprintf(stderr,"[%s] Sending message to create new agent %s - %s:%u from %s \n",__func__,newProcess.nodeName,inet_ntoa(destinationIp),DAEMON_AGENT_PORT,inet_ntoa(myInternalAddress));
				//Send message
				sendMsgTo((void *)&newProcess, PACKAGE_ID_DATAPROCESS, DAEMON_AGENT_PORT, inet_ntoa(destinationIp));
				//We store the agent IP and port. This is to be able to stop them in a future step
				agentsAddresses.push_back(destinationIp);
				
			}
			else {
				syslog(LOG_ERR,"[%s] Error in getaddrinfo() for %s\n",__func__,newProcess.nodeName);
			}
			
		}
		
		else if(rxMsg.packageId == PACKAGE_ID_DATAMSG) {
			//Re-Route the package to the client
			
			//fprintf(stderr,"[%s] Re-sending message to client - %s - %s:%u\n",__func__,rxMsg.processName,inet_ntoa(processInfo.from.sin_addr),CLIENT_BASE_PORT);
			
			
			sendMsgTo((void *)&rxMsg,PACKAGE_ID_DATAMSG, CLIENT_BASE_PORT, inet_ntoa(addresFromClient));
			
		}
		
		else if (rxMsg.packageId == PACKAGE_ID_STOP) {
			
			memcpy(&newProcess, rxBuffer, sizeof(ProcessesInfo));
			
			//Set the user id and the info from where the message has been sent
			newProcess.uid = userId;
			newProcess.port = rxPort;
			newProcess.from.sin_port = rxPort;
			newProcess.from.sin_addr = myInternalAddress;
		

			//Stop all the agents
			for (unsigned int i = 0; i < agentsAddresses.size(); i++) {
	
				sendMsgTo((void *)&newProcess, PACKAGE_ID_STOP, DAEMON_AGENT_PORT, inet_ntoa(agentsAddresses.at(i)));
	
			}
			
			//Stop this MonitoringMaster
			stopMaster = 1;
		}

	}
	
}

int main(int argc, char **argv) {

	//Daemon init
	setlogmask(LOG_UPTO(LOG_NOTICE | LOG_INFO));
	openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
    
	syslog(LOG_INFO, "Entering Daemon MonitoringMaster\n");

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

	//Close Standard File Descriptors
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	//Begin of Daemon
	int stopDaemon = 0;

	//Variables for network
	
	int rxPort = MASTER_BASE_PORT;
	
	
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
	
	
	syslog(LOG_INFO,"[%s] I am master node at %s\n", strAddr(myRxAddr),__func__);

	int nRxBytes;
	socklen_t addrLen;
	char rxBuffer[MAX_BUF_SIZE];//Currently 20480. In Network.h
	ProcessesInfo rxMsg;


	while (!stopDaemon) { //We stay receiving data packages and sending them to the client

		//We stay blocked waiting for a new message
		nRxBytes = recvfrom(rxSocket, rxBuffer, sizeof(rxBuffer), 0,(struct sockaddr *) &myRxAddr, &addrLen);
		if(nRxBytes <= 0) {
			syslog(LOG_ERR,"[%s] Error in recvfrom()\n",__func__);
		}

		//Message parse
		memcpy(&rxMsg, rxBuffer, sizeof(ProcessesInfo));
		
		if(rxMsg.packageId == PACKAGE_ID_DATAPROCESS) {
	
			syslog(LOG_INFO,"[%s] Received message\n",__func__);
			
			//rxMsg.port = availablePorts.back();
			//availablePorts.pop_back();
	
			//Creation of new process to monitor communications
			if(fork() == 0){
				//syslog(LOG_INFO,"[%s] Forked process\n",__func__);
				createMasterMonitor(rxMsg);
				stopDaemon = 1;
			}
		
		}
		else{
			syslog(LOG_ERR,"[%s] Package id not recognized %u\n",__func__,rxMsg.packageId);
		}
	}

	//Close the log
	closelog ();

	return 0;
}

