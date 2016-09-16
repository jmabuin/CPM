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

#ifndef DEFS_H
#define DEFS_H 

// Libraries
#include <string.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>

#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>

#include "Globals.h"

#define MASTER_BASE_PORT 8000
#define DAEMON_AGENT_PORT 20000
#define CLIENT_BASE_PORT 10000

#define MAX_BUF_SIZE 20480

#define DAEMON_NAME "MonitoringAgent"

#define PACKAGE_ID_ENERGY 4
#define PACKAGE_ID_STOP 3
#define PACKAGE_ID_DATAMSG 2
#define PACKAGE_ID_DATAPROCESS 1

#define SLEEP_NUM_SECS 5

#define NETWORK_INTERFACE "eth0"

#define MAX_PROCESS_NAME 2048

typedef struct Agent2MasterDataMsg{
	unsigned int packageId;
	int PID;
	unsigned int agentId;
	unsigned long int messageNumber;
	unsigned long int measureNumber;
	double cpuPercentage;
	double totalCpuPercentage;
	char userName[256];
	char processName[MAX_PROCESS_NAME];
	unsigned long long memory;
	double memoryPercentage;
	struct sockaddr_in destinationNode;
	int numberOfThreads;
	long long int papiMeasures[3];
	//double energyMeasures[24];
  
} Agent2MasterDataMsg;

typedef struct ProcessesInfo{
	unsigned int packageId;
	char userName[256];
	char processName[MAX_PROCESS_NAME];
	char processStartsWith[MAX_PROCESS_NAME];
	unsigned int agentId;
	unsigned int uid;
	unsigned int cpuThreshold;
	struct sockaddr_in from;
	char nodeName[256];
	struct sockaddr_in destinationNode;
	unsigned int port;
	int measurePapi;
	int measureEnergy;

} ProcessesInfo;

typedef struct Agent2MasterEnergyMsg{
	unsigned int packageId;
	int PID;
	unsigned int agentId;
	unsigned long int messageNumber;
	unsigned long int measureNumber;
	char userName[256];
	struct sockaddr_in destinationNode;
	double energyMeasures[24];
  
} Agent2MasterEnergyMsg;

struct in_addr getOwnIp(char *iface);
struct sockaddr_in buildAddr(short int sin_family, unsigned short int sin_port, struct in_addr sin_addr);
int createUDPSocket(int broadcast, int isTxSocket, struct sockaddr_in myAddr);
int sendMsg(int txSocket, Agent2MasterDataMsg *msg, struct sockaddr_in *txAddrs);
int createTxUPDSocket(unsigned int port, char *address, struct sockaddr_in *rxMasterSocket);
char *strAddr(struct sockaddr_in);
int sendMsgTo(void *message, unsigned int msgType, unsigned int port, char *ip);

#endif
