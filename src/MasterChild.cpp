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

#include <vector>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>

#include "Network.h"


void createMasterMonitor(int client_port, int master_port, int agent_port, char *network_outside, char *network_internal, struct ProcessesInfo processInfo) {
	
	using namespace std;
	
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
	Agent2MasterEnergyMsg energyPkg;
	
	//Message to senf info data to client
	ProcessesInfo newProcess;
	
	//To store IPs, username and process to be monitorized. Using this, we will be able to stop them in the computing node
	vector<struct in_addr> agentsAddresses;


	//Set the network interfaces to use. Defined in Network.h or passed as arguments from main program
	int length = 0;
	char *networkInterface;
	char *networkInterfaceInternal;
	
	if(network_outside) {
		length = strlen(network_outside)+1;
		networkInterface = (char *)malloc(sizeof(char) * length);
		strcpy(networkInterface, network_outside);
	}
	else {
		std::vector<std::string> interfaces_names = getInterfaces();
		
		if(interfaces_names.size() >= 2) {
			if(strcmp(interfaces_names[0].c_str(), "lo") == 0) {
				length = strlen(interfaces_names[1].c_str())+1;
				networkInterface = (char *)malloc(sizeof(char) * length);
				strcpy(networkInterface, interfaces_names[1].c_str());
			}
			else {
				length = strlen(interfaces_names[0].c_str())+1;
				networkInterface = (char *)malloc(sizeof(char) * length);
				strcpy(networkInterface, interfaces_names[0].c_str());
			}
		}
		else {
			length = strlen(NETWORK_INTERFACE)+1;
			networkInterface = (char *)malloc(sizeof(char) * length);
			strcpy(networkInterface,NETWORK_INTERFACE);
		}
	}
	
	
	if(network_internal) {
		length = strlen(network_internal)+1;
		networkInterfaceInternal = (char *)malloc(sizeof(char) * length);
		strcpy(networkInterfaceInternal, network_internal);
	}
	else {
		std::vector<std::string> interfaces_names = getInterfaces();
		
		if(interfaces_names.size() >= 2) {
			if(strcmp(interfaces_names[0].c_str(), "lo") == 0) {
				length = strlen(interfaces_names[1].c_str())+1;
				networkInterfaceInternal = (char *)malloc(sizeof(char) * length);
				strcpy(networkInterfaceInternal, interfaces_names[1].c_str());
			}
			else {
				length = strlen(interfaces_names[0].c_str())+1;
				networkInterfaceInternal = (char *)malloc(sizeof(char) * length);
				strcpy(networkInterfaceInternal, interfaces_names[0].c_str());
			}
		}
		else {
			length = strlen(NETWORK_INTERFACE_INTERNAL)+1;
			networkInterfaceInternal = (char *)malloc(sizeof(char) * length);
			strcpy(networkInterfaceInternal,NETWORK_INTERFACE_INTERNAL);
		}
	
	}
	


	//Get the current user ID
	userInfo = getpwnam(processInfo.userName);
	
	uid_t userId = userInfo->pw_uid;

	//Set the port where we are going to listen
	//int rxPort = MASTER_BASE_PORT + userId;
	
	//int rxPort = MASTER_BASE_PORT + processInfo.port;
	
	int rxPort = MASTER_BASE_PORT + 1;
	
	if(master_port != 0) {
		rxPort = master_port + 1;
	}
	
	
	int cli_port;
	
	if(client_port != 0) {
		cli_port = client_port;
	}
	else{
		cli_port = CLIENT_BASE_PORT;
	}
	//int rxPort = 0;

	int age_port;
	
	if(agent_port != 0) {
		age_port = agent_port;
	}
	else{
		age_port = DAEMON_AGENT_PORT;
	}

	newProcess.port = rxPort;
	newProcess.from.sin_port = rxPort;
	
	struct in_addr addresFromClient = processInfo.from.sin_addr;
	
	
	
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
	printFunction(0,"[%s] Creating MasterMonitor %s\n",__func__, strAddr(myRxAddr));
	printFunction(0,"[%s] Internal address is %s:%s\n",__func__, networkInterfaceInternal, inet_ntoa(myInternalAddress));
	printFunction(0,"[%s] External address is %s:%s\n",__func__, networkInterface, inet_ntoa(processInfo.from.sin_addr));

	//We send ACK to client with info. Client must know new port to send the Agents info
	
	printFunction(0,"[%s] Messaging back to Client %s:%u\n",__func__, inet_ntoa(addresFromClient),cli_port);
	
	if(sendMsgTo((void *)&processInfo, PACKAGE_ID_DATAPROCESS, cli_port, inet_ntoa(addresFromClient)) == -1){
	
		printFunction(1,"[%s] Error sending message back to Client %s:%u\n",__func__, inet_ntoa(addresFromClient),cli_port);
	}
	else {
		printFunction(0,"[%s] Message sent back to Client %s:%u\n",__func__, inet_ntoa(addresFromClient),cli_port);
	}

	int i = 0 ;
	
	int stopMaster = 0;
	
	while (!stopMaster) { //We stay receiving data packages and sending them to the client



		//We stay blocked waiting for a new message
		nRxBytes = recvfrom(rxSocket, rxBuffer, sizeof(rxBuffer), 0,(struct sockaddr *) &myRxAddr, &addrLen);
		
		if(nRxBytes <= 0) {
			printFunction(1,"[%s] Error in recvfrom()\n",__func__);
		}
		
		//Parse message
		memcpy(&rxMsg, rxBuffer, sizeof(Agent2MasterDataMsg));
		//memcpy(&energyPkg, rxBuffer, sizeof(Agent2MasterEnergyMsg));
		
		//printFunction(0,"[%s] Received message %d type %u\n",__func__,i,rxMsg.packageId);
		//printFunction(0,"[%s] Received message Energy %d - %lu type %u\n",__func__,i,energyPkg.messageNumber,energyPkg.packageId);
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
				sendMsgTo((void *)&newProcess, PACKAGE_ID_DATAPROCESS, age_port, inet_ntoa(destinationIp));
				//We store the agent IP and port. This is to be able to stop them in a future step
				agentsAddresses.push_back(destinationIp);
				
			}
			else {
				printFunction(1,"[%s] Error in getaddrinfo() for %s\n",__func__,newProcess.nodeName);
			}
			
		}
		
		else if(rxMsg.packageId == PACKAGE_ID_DATAMSG) {
			//Re-Route the package to the client
			
			//fprintf(stderr,"[%s] Re-sending message to client - %s - %s:%u\n",__func__,rxMsg.processName,inet_ntoa(processInfo.from.sin_addr),CLIENT_BASE_PORT);
			
			//printFunction(0,"[%s] Re-sending data package to client %u %lu\n",__func__,rxMsg.packageId,rxMsg.messageNumber);
			sendMsgTo((void *)&rxMsg,PACKAGE_ID_DATAMSG, cli_port, inet_ntoa(addresFromClient));
			
		}
		
		else if(rxMsg.packageId == PACKAGE_ID_ENERGY) {
			//Re-Route the package to the client
			
			//fprintf(stderr,"[%s] Re-sending message to client - %s - %s:%u\n",__func__,rxMsg.processName,inet_ntoa(processInfo.from.sin_addr),CLIENT_BASE_PORT);
			
			//Agent2MasterEnergyMsg energyPkg;
			
			
			memcpy(&energyPkg, rxBuffer, sizeof(Agent2MasterEnergyMsg));
			//printFunction(0,"[%s] Re-sending energy package to client %u %lu\n",__func__,energyPkg.packageId,energyPkg.messageNumber);
			sendMsgTo((void *)&energyPkg,PACKAGE_ID_ENERGY, cli_port, inet_ntoa(addresFromClient));
			
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
	
				sendMsgTo((void *)&newProcess, PACKAGE_ID_STOP, age_port, inet_ntoa(agentsAddresses.at(i)));
	
			}
			
			//Stop this MonitoringMaster
			stopMaster = 1;
		}

	}
	
}
