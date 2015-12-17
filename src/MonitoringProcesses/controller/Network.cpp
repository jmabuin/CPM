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

#include "Network.h"

Network::Network()
{

}

struct in_addr Network::getOwnIp(char *iface) {

	struct ifreq ifr;
	struct sockaddr_in saddr;
	int fd=socket(PF_INET,SOCK_STREAM,0);
	strcpy(ifr.ifr_name,iface);
	ioctl(fd,SIOCGIFADDR,&ifr);
	saddr=*((struct sockaddr_in *)(&(ifr.ifr_addr))); // is the address

	return(saddr.sin_addr);


}

struct sockaddr_in Network::buildAddr(short sin_family, unsigned short sin_port, in_addr sin_addr) {
	struct sockaddr_in addr;


	(addr).sin_family = sin_family;
	(addr).sin_port = htons(sin_port);
	(addr).sin_addr= sin_addr;
	bzero(&((addr).sin_zero),8);

	return addr;
}

int Network::createUDPSocket(int broadcast, int isTxSocket, sockaddr_in myAddr) {
	int sock;



	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("Could not create socket\n");
		return(-1);
	}

	int reuse_addr =1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof reuse_addr);

	if (isTxSocket) {
		if (broadcast) {
			if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
				perror("setsockopt (SO_BROADCAST)");
				return(-1);
			}
		}
	}

	if (bind(sock, (struct sockaddr *)&myAddr, sizeof(struct sockaddr)) == -1) {
		perror("Could not bind socket\n");
		return(-1);
	}


	return sock;
}

int Network::sendMsg(int txSocket, Agent2MasterDataMsg *msg, sockaddr_in *txAddrs) {

	return sendto(txSocket, (const void *) msg, sizeof(struct Agent2MasterDataMsg), 0, (struct sockaddr *)&txAddrs, sizeof(struct sockaddr));

}


char * Network::strAddr(sockaddr_in addr) {
	char *str = new char[40];

	sprintf(str, "(%d, %s, %d)", addr.sin_family, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	return str;
}

int Network::createTxUPDSocket(unsigned int port, char *address, struct sockaddr_in *rxMasterSocket) {


	int s;

	if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		fprintf(stderr, "socket() failed\n");
	}

	memset((char *) rxMasterSocket, 0, sizeof(*rxMasterSocket));
	rxMasterSocket->sin_family = AF_INET;
	rxMasterSocket->sin_port = htons(port);

	if (inet_aton(address , &(rxMasterSocket->sin_addr)) == 0) {
		fprintf(stderr, "inet_aton() failed\n");
		return -1;
	}

	return s;
}

int Network::sendMsgTo(void *message, unsigned int msgType, unsigned int port, char *ip){

	struct sockaddr_in destinationSocket;
	int s;

	if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		fprintf(stderr, "socket() failed\n");
	}

	memset((char *) &destinationSocket, 0, sizeof(destinationSocket));
	destinationSocket.sin_family = AF_INET;
	destinationSocket.sin_port = htons(port);
	inet_aton(ip,&destinationSocket.sin_addr);


	if((msgType == PACKAGE_ID_DATAPROCESS ) || (msgType == PACKAGE_ID_STOP)) {
		if (sendto(s, message, sizeof(struct ProcessesInfo) , 0 , (struct sockaddr *) &destinationSocket, sizeof(struct sockaddr))==-1) {
			fprintf(stderr,"Error in sendto()\n");
			return 0;
		}
	}
	else if(msgType == PACKAGE_ID_DATAMSG) {
		if (sendto(s, message, sizeof(struct Agent2MasterDataMsg) , 0 , (struct sockaddr *) &destinationSocket, sizeof(struct sockaddr))==-1) {
			fprintf(stderr,"Error in sendto()\n");
			return 0;
		}
	}



	return 1;

}

std::vector<std::string> Network::getInterfaces() {

	std::vector<std::string> returnedInterfaces;

	struct ifaddrs *ifaddr, *ifa;
	int n;

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");

	}
	else{
		for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {

			if (ifa->ifa_addr == NULL) {
				continue;
			}
			if(std::find(returnedInterfaces.begin(),returnedInterfaces.end(),ifa->ifa_name) == returnedInterfaces.end()){
				//printf("%d %s\n",n,ifa->ifa_name);
				returnedInterfaces.push_back(ifa->ifa_name);
			}
		}
	}
	return returnedInterfaces;

}
