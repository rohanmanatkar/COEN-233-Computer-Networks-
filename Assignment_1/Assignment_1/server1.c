/*

 Programming Assignment 1: Client using customized protocol on top of UDP protocol for sending information to the server.
 One client connects to one server.
 Name: Rohan Manatkar
 Campus ID: W1538036
 
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>


#define PORT 8080
#define PACKETID 0XFFFF
#define CLIENTID 0XFF
#define DATATYPE 0XFFF1
#define ENDPACKETID 0XFFFF
#define TIMEOUT 3
#define ACKPACKET 0XFFF2
#define REJECTCODE 0XFFF3
#define LENGTHMISMATCHCODE 0XFFF5
#define ENDPACKETIDMISSINGCODE 0XFFF6
#define OUTOFSEQUENCECODE 0XFFF4
#define DUPLICATECODE 0XFFF7

// Structure of Data Packet 
struct DataPacket{
	uint16_t packetID;
	uint8_t clientID;
	uint16_t data;
	uint8_t segmentNo;
	uint8_t length;
	char payload[255];
	uint16_t endPacketID;
};

// Structure of Acknowledgement Packet
struct AckPacket {
	uint16_t packetID;
	uint8_t clientID;
	uint16_t ack;
	uint8_t segmentNo;
	uint16_t endPacketID;
};

// Structure of Reject Packet
struct RejectPacket {
	uint16_t packetID;
	uint8_t clientID;
	uint16_t reject;
	uint16_t subCode;
	uint8_t segmentNo;
	uint16_t endPacketID;
};


// Function to print the packet details
void show(struct DataPacket dataPacket) {
	printf("Packet Details Received from Client:\n");
	printf("Packet ID: %hx\n",dataPacket.packetID);
	printf("Client ID: %hhx\n",dataPacket.clientID);
	printf("Data: %x\n",dataPacket.data);
	printf("Segment No: %d\n",dataPacket.segmentNo);
	printf("Length: %d\n",dataPacket.length);
	printf("Payload: %s\n",dataPacket.payload);
	printf("End of Packet ID: %x\n",dataPacket.endPacketID);
}

// Function to create the ACK (Acknowledge) packet
struct AckPacket createAckPacket(struct DataPacket dataPacket) {
	struct AckPacket ackPacket;
	ackPacket.packetID = dataPacket.packetID;
	ackPacket.clientID = dataPacket.clientID;
	ackPacket.segmentNo = dataPacket.segmentNo;
	ackPacket.ack = ACKPACKET ;
	ackPacket.endPacketID = dataPacket.endPacketID;
	return ackPacket;
}

// Function to create the reject packet
struct RejectPacket createRejectPacket(struct DataPacket dataPacket) {
	struct RejectPacket rejectPacket;
	rejectPacket.packetID = dataPacket.packetID;
	rejectPacket.clientID = dataPacket.clientID;
	rejectPacket.segmentNo = dataPacket.segmentNo;
	rejectPacket.reject = REJECTCODE;
	rejectPacket.endPacketID = dataPacket.endPacketID;
	return rejectPacket;
}

int main(int argc, char **argv)
{
	int sockfd,n;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;
	struct DataPacket dataPacket;
	struct AckPacket  ackPacket;
	struct RejectPacket rejectPacket;

	// Buffer to store received packets
	int buffer[20];
	for(int j = 0; j < 20;j++) {
		buffer[j] = 0;
	}
	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	int expectingPacket = 1;
	bzero(&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serverAddr.sin_port=htons(PORT);
	bind(sockfd,(struct sockaddr *)&serverAddr,sizeof(serverAddr));
	addr_size = sizeof serverAddr;
	printf("Server up and Running...\n");

	while(1){
		// Receiving packets from the client
		n = recvfrom(sockfd,&dataPacket,sizeof(struct DataPacket),0,(struct sockaddr *)&serverStorage, &addr_size);
		printf("---New---\n");
		show(dataPacket);
		buffer[dataPacket.segmentNo]++;
		if(dataPacket.segmentNo == 11 || dataPacket.segmentNo == 12) {
			buffer[dataPacket.segmentNo] = 1;
		}
		int length = strlen(dataPacket.payload);

		if(buffer[dataPacket.segmentNo] != 1) {
			rejectPacket = createRejectPacket(dataPacket);
			
			rejectPacket.subCode = DUPLICATECODE;
			sendto(sockfd,&rejectPacket,sizeof(struct RejectPacket),0,(struct sockaddr *)&serverStorage,addr_size);
			
			printf("Error: DUPLICATE PACKET HAS BEEN RECIEVED\n\n");
		}

		else if(length != dataPacket.length) {
			rejectPacket = createRejectPacket(dataPacket);

			rejectPacket.subCode = LENGTHMISMATCHCODE ;
			sendto(sockfd,&rejectPacket,sizeof(struct RejectPacket),0,(struct sockaddr *)&serverStorage,addr_size);

			printf("Error: LENGTH MISMATCH \n\n");
		}
		else if(dataPacket.endPacketID != ENDPACKETID ) {
			rejectPacket = createRejectPacket(dataPacket);

			rejectPacket.subCode = ENDPACKETIDMISSINGCODE ;
			sendto(sockfd,&rejectPacket,sizeof(struct RejectPacket),0,(struct sockaddr *)&serverStorage,addr_size);

			printf("Error: END OF PACKET IDENTIFIER IS MISSING\n\n");

		}
		else if(dataPacket.segmentNo != expectingPacket && dataPacket.segmentNo != 11 && dataPacket.segmentNo != 12) {
			rejectPacket = createRejectPacket(dataPacket);

			rejectPacket.subCode = OUTOFSEQUENCECODE;
			sendto(sockfd,&rejectPacket,sizeof(struct RejectPacket),0,(struct sockaddr *)&serverStorage,addr_size);

			printf("Error: OUT OF SEQUENCE PACKET RECEIVED\n\n");
		}
		else {
			if(dataPacket.segmentNo == 11) {
				sleep(6);
			}
			ackPacket = createAckPacket(dataPacket);
			sendto(sockfd,&ackPacket,sizeof(struct AckPacket),0,(struct sockaddr *)&serverStorage,addr_size);
		}
		expectingPacket++;
		printf("\n*******************************************************************\n");
	}
}



