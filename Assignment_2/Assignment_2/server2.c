/*
 
 Programming Assignment 2 (Server): Client using customized protocol on top of UDP protocol for requesting identification from server for access permission to the network.
 Name: Rohan Manatkar
 Campus ID: W1538036

*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>


#define PORT 8080
#define LENGTH 10
#define PAID 0XFFFB
#define NOTPAID 0XFFF9
#define NOTEXIST 0XFFFA


// Structure For Request Packet
struct RequestPacket{
	uint16_t packetID;
	uint8_t clientID;
	uint16_t Acc_Per;
	uint8_t segment_No;
	uint8_t length;
	uint8_t technology;
	unsigned int SourceSubscriberNo;
	uint16_t endpacketID;
};


// Structure For Response Packet
struct ResponsePacket {
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segment_No;
	uint8_t length;
	uint8_t technology;
	unsigned int SourceSubscriberNo;
	uint16_t endpacketID;
};


// Structure for storing Subscriber data
struct SubscriberDatabase {
	unsigned long subscriberNumber;
	uint8_t technology;
	int status;

};

// To create Response Packet
struct ResponsePacket createResponsePacket(struct RequestPacket requestPacket) {
	struct ResponsePacket responsePkt;
	responsePkt.packetID = requestPacket.packetID;
	responsePkt.clientID = requestPacket.clientID;
	responsePkt.segment_No = requestPacket.segment_No;
	responsePkt.length = requestPacket.length;
	responsePkt.technology = requestPacket.technology;
	responsePkt.SourceSubscriberNo = requestPacket.SourceSubscriberNo;
	responsePkt.endpacketID = requestPacket.endpacketID;
	return responsePkt;
}

// Function to print Packet details
void displayPacket(struct RequestPacket requestPacket ) {
	printf("Packet ID: %x\n",requestPacket.packetID);
	printf("Client ID: %hhx\n",requestPacket.clientID);
	printf("Access Permission: %x\n",requestPacket.Acc_Per);
	printf("Segment no : %d \n",requestPacket.segment_No);
	printf("Length of the Packet: %d\n",requestPacket.length);
	printf("Technology: %d \n", requestPacket.technology);
	printf("Subscriber no: %u \n",requestPacket.SourceSubscriberNo);
	printf("End of RequestPacket ID: %x \n",requestPacket.endpacketID);
}


// Function to read file for mapping
void readFile(struct SubscriberDatabase subDatabase[]) {

	// Storing file from data locally
	char line[30];
	int i = 0;
	FILE *fp;

	fp = fopen("Verification_Database.txt", "rt");

	if(fp == NULL)
	{
		printf("Cannot Open File!\n");
		return;
	}
	while(fgets(line, sizeof(line), fp) != NULL)
	{
		char * words=NULL;
		words = strtok(line," ");
		subDatabase[i].subscriberNumber =(unsigned) atol(words); // long int
		words = strtok(NULL," ");
		subDatabase[i].technology = atoi(words); // int
		words = strtok(NULL," ");
		subDatabase[i].status = atoi(words);
		i++;
	}
	fclose(fp);
}


// Checking Subscriber's existence
int check(struct SubscriberDatabase subDatabase[],unsigned int subscriberNumber,uint8_t technology) {
	int value = -1;
	for(int j = 0; j < LENGTH;j++) {
		if(subDatabase[j].subscriberNumber == subscriberNumber && subDatabase[j].technology == technology) {
			return subDatabase[j].status;
		}
                else if (subDatabase[j].subscriberNumber == subscriberNumber && subDatabase[j].technology != technology)
                        return 2;
	}
	return value;
}


int main(int argc, char**argv){
	
        struct RequestPacket requestPacket;
	struct ResponsePacket responsePkt;
	
        struct SubscriberDatabase subDatabase[LENGTH];
	readFile(subDatabase);
	
        int sockfd,n;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;
	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	
        bzero(&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serverAddr.sin_port=htons(PORT);
	
        bind(sockfd,(struct sockaddr *)&serverAddr,sizeof(serverAddr));
	addr_size = sizeof serverAddr;
	printf("Server up and running...\n");
	
        for (;;) {
                // Get the packet
		n = recvfrom(sockfd,&requestPacket,sizeof(struct RequestPacket),0,(struct sockaddr *)&serverStorage, &addr_size);
		displayPacket(requestPacket);
		if(requestPacket.segment_No == 11) {
			exit(0);
		}

		if(n > 0 && requestPacket.Acc_Per == 0XFFF8) {
			// Response Packet
			responsePkt = createResponsePacket(requestPacket);

			int value = check(subDatabase,requestPacket.SourceSubscriberNo,requestPacket.technology);
			if(value == 0) {
				
				responsePkt.type = NOTPAID;
				printf("SUBSCRIBER HAS NOT PAID\n");
			}
			else if(value == 1) {
				
				printf("SUBSCRIBER HAS PAID\n");
				responsePkt.type = PAID;
			}

			else if(value == -1) {
               
				printf("SUBSCRIBER DOES NOT EXIST\n");
				responsePkt.type = NOTEXIST;
			}
                        
                        else{
                                
                                printf("SUBCRIBER'S TECHNOLOGY DOES NOT MATCH\n");
                                responsePkt.type = NOTEXIST;
                        }                        
			// Sending the response packet
			sendto(sockfd,&responsePkt,sizeof(struct ResponsePacket),0,(struct sockaddr *)&serverStorage,addr_size);
		}
		n = 0;
		printf(" \n**************************************************************************\n");
	}
}



