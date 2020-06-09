/*
 
 Programming Assignment 1: Client using customized protocol on top of UDP protocol for sending information to the server.
 One client connects to one server.
 Name: Rohan Manatkar
 Campus ID: W1538036
 
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <strings.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

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
struct DataPacket
{
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segmentNo;
	uint8_t length;
	char payload[255];
	uint16_t endPacketId;
};

// Structure of Acknowledgement Packet
struct AckPacket
{
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segmentNo;
	uint16_t endPacketId;
};

// Structure of Reject Packet
struct RejectPacket
{
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint16_t subCode;
	uint8_t segmentNo;
	uint16_t endPacketId;
};

// Structure for Packet data
struct DataPacket createDataPacket()
{
	struct DataPacket datapacket;
	datapacket.packetID = PACKETID;
	datapacket.clientID = CLIENTID;
	datapacket.type = DATATYPE;
	datapacket.endPacketId = ENDPACKETID;
	return datapacket;
}

// Function to print the Packet details
void displayPacket(struct DataPacket dataPacket)
{
	printf("Packet ID: %x\n", dataPacket.packetID);
	printf("Client ID: %hhx\n", dataPacket.clientID);
	printf("Data: %x\n", dataPacket.type);
	printf("Segment No: %d \n", dataPacket.segmentNo);
	printf("Length: %d\n", dataPacket.length);
	printf("Payload: %s\n", dataPacket.payload);
	printf("End of DataPacket ID: %x\n", dataPacket.endPacketId);
}

int main()
{
	struct DataPacket dataPacket;
	struct RejectPacket receivedPacket;
	struct sockaddr_in clientAddr;
	socklen_t addr_size;
	FILE *fp;
	char line[255];
	int sockfd;
	int n = 0;
	int count = 0;
	int segmentNo = 1;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		printf("Socket has failed to create!\n");
	}

	// Reset and define connection
	bzero(&clientAddr, sizeof(clientAddr));
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	clientAddr.sin_port = htons(PORT);
	addr_size = sizeof clientAddr;

	// Timeout (3 seconds)
	struct timeval timeValue;
	timeValue.tv_sec = TIMEOUT;
	timeValue.tv_usec = 0;
	
	// Predefined parameters
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeValue, sizeof(struct timeval));
	dataPacket = createDataPacket();

	// To open the input file
	fp = fopen("input.txt", "rt");
	if (fp == NULL)
	{
		printf("Cannot Open File...\n");
		exit(0);
	}

	// To transfer data from File to Packet
	while (fgets(line, sizeof(line), fp) != NULL)
	{
		n = 0;
		count = 0;
		dataPacket.segmentNo = segmentNo;
		strcpy(dataPacket.payload, line);
		dataPacket.length = strlen(dataPacket.payload);
		if (segmentNo == 8)
		{
			dataPacket.length++;
		}
		if (segmentNo == 10)
		{
			dataPacket.segmentNo = dataPacket.segmentNo + 4;
		}
		if (segmentNo == 7)
		{
			dataPacket.segmentNo = 3;
		}
		if (segmentNo == 9)
		{
			dataPacket.endPacketId = 0;
		}
		if (segmentNo != 9)
		{
			dataPacket.endPacketId = ENDPACKETID;
		}
		
		// To display packet data
		displayPacket(dataPacket); 	
		while (n <= 0 && count < 3)
		{
			sendto(sockfd, &dataPacket, sizeof(struct DataPacket), 0, (struct sockaddr *)&clientAddr, addr_size);
			n = recvfrom(sockfd, &receivedPacket, sizeof(struct RejectPacket), 0, NULL, NULL);

			if (n <= 0)
			{
				printf("No Response received... Trying once again!\n");
				count++;
			}
			else if (receivedPacket.type == ACKPACKET)
			{
				printf("Acknowledgement received!!\n \n");
			}
			else if (receivedPacket.type == REJECTCODE)
			{
				printf("Reject packet received! \n");
				printf("Type: %x \n", receivedPacket.subCode);
				if (receivedPacket.subCode == LENGTHMISMATCHCODE)
				{
					printf("Error: Length Mismatch!\n");
				}
				else if (receivedPacket.subCode == ENDPACKETIDMISSINGCODE)
				{
					printf("Error: End of packet identifier is missing!\n");
				}
				else if (receivedPacket.subCode == OUTOFSEQUENCECODE)
				{
					printf("Error: Packet out of sequence!\n");
				}
				else if (receivedPacket.subCode == DUPLICATECODE)
				{
					printf("Error: Duplicate packet received!\n");
				}
			}
		}

		if (count >= 3)
		{
			printf("Server does not respond!");
			exit(0);
		}
		segmentNo++;
		printf("\n*****************************************************************\n");
	}
}
