/*
 
 Programming Assignment 2 (Client): Client using customized protocol on top of UDP protocol for requesting identification from server for access permission to the network
 Name: Rohan Manatkar
 Campus ID: W1538036
 
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>


#define port 8080
#define paid 0XFFFB
#define notPaid 0XFFF9
#define doesNotExist 0XFFFA


// Structure For Request Packet
struct RequestPacket {
	uint16_t packetId;
	uint8_t clientId;
	uint16_t Acc_Per;
	uint8_t segment_No;
	uint8_t length;
	uint8_t technology;
	unsigned int SourceSubscriberNo;
	uint16_t endPacket;
};


// Structure For Response Packet
struct ResponsePacket {
	uint16_t packetId;
	uint8_t clientId;
	uint16_t type;
	uint8_t segment_No;
	uint8_t length;
	uint8_t technology;
	unsigned int SourceSubscriberNo;
	uint16_t endPacket;
};


// Function to print the packet details
void DisplayPacket(struct RequestPacket requestPkt) {
	printf("Packet ID: %x\n",requestPkt.packetId);
	printf("Client ID: %hhx\n",requestPkt.clientId);
	printf("Access Permission: %x\n",requestPkt.Acc_Per);
	printf("Segment no : %d \n",requestPkt.segment_No);
	printf("length: %d\n",requestPkt.length);
	printf("Technology: %d \n", requestPkt.technology);
	printf("Subscriber no: %u \n",requestPkt.SourceSubscriberNo);
	printf("End of DataPacket ID: %x \n",requestPkt.endPacket);
}


// Initializing Request Packet
struct RequestPacket Initialize () {
	struct RequestPacket requestPkt;
	requestPkt.packetId = 0XFFFF;
	requestPkt.clientId = 0XFF;
	requestPkt.Acc_Per = 0XFFF8;
	requestPkt.endPacket = 0XFFFF;
	return requestPkt;

}

int main(int argc, char**argv){
	struct RequestPacket requestPkt;
	struct ResponsePacket responsePkt;
	char line[30];
	int i = 1;
	FILE *fp;
	int sockfd,n = 0;
	struct sockaddr_in clientAddr;
	socklen_t addr_size;
	sockfd = socket(AF_INET,SOCK_DGRAM,0);
	struct timeval timeValue;
	timeValue.tv_sec = 3;  // Timeout
	timeValue.tv_usec = 0;

	//Checking for connection
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeValue,sizeof(struct timeval));
	int counter = 0;
	if(sockfd < 0) {
		printf("Socket Connection Failed\n");
	}
	bzero(&clientAddr,sizeof(clientAddr));
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	clientAddr.sin_port=htons(port);
	addr_size = sizeof clientAddr ;


	//Loading of data into Packet
	requestPkt = Initialize();

	//Reading input file

	fp = fopen("input.txt", "rt");

	if(fp == NULL)
	{
		printf("ALERT: Cannot Open File\n");
	}


	while(fgets(line, sizeof(line), fp) != NULL) {
		counter = 0;
		n = 0;
		printf(" \n NEW REQUEST \n");
		char * words;
		// Split the line
		words = strtok(line," ");
		requestPkt.length = strlen(words);
		requestPkt.SourceSubscriberNo = (unsigned) atoi(words);
		words = strtok(NULL," ");
		requestPkt.length += strlen(words);
		requestPkt.technology = atoi(words);
		words = strtok(NULL," ");
		requestPkt.segment_No = i;
		// Printing Contents of the Packet
		DisplayPacket(requestPkt);
		
		// Check if packet is sent, if not try again.
		while(n <= 0 && counter < 3) { 
			sendto(sockfd,&requestPkt,sizeof(struct RequestPacket),0,(struct sockaddr *)&clientAddr,addr_size);
			// Get response from Server
			n = recvfrom(sockfd,&responsePkt,sizeof(struct ResponsePacket),0,NULL,NULL);
			if(n <= 0 ) {
				// No response
				printf("Out of Time(3 seconds)\n");
				counter ++;
			}
			else if(n > 0) {
				// Response received
				printf("Status = ");
				if(responsePkt.type == notPaid) {
					printf("SUBSCRIBER HAS NOT PAID\n");
				}
				else if(responsePkt.type == doesNotExist ) {
					printf("SUBSCRIBER DOES NOT EXIST\n");
				}
				else if(responsePkt.type == paid) {
					printf("PERMITTED TO ACCESS THE NETWORK\n");

				}
			}
		}
		// After 3 attempts timeout
		if(counter >= 3 ) {
			printf("ALERT- SERVER IS NOT RESPONDING");
			exit(0);
		}
		i++;
		printf("\n*************************************************************************\n");
	}
	fclose(fp);
}

