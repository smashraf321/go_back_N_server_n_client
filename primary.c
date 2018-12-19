#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include "ccitt16.h"
#include "utilities.h"
#include "introduceerror.h"

void primary(int sockfd, double ber)
{
	int read_size;
	char msg[100], pkt_data[DATA_LENGTH];
	unsigned char pkt[PACKET_SIZE];
	unsigned char srv_reply[150];
	int maxSeqNum;
	int window[WINDOW_SIZE];
	unsigned char win_seq_nos[WINDOW_SIZE];
	unsigned char pktSeqNum;
	int msg_len, no_of_pkts = 0;
	int i,j,k;
	int nak,shift,shifter;

	//init win_seq_nos
  win_seq_nos[0] = 1;
  win_seq_nos[1] = 2;
	win_seq_nos[2] = 3;

	// no_of_pkts = 0;
	// pkt_data[0]='a';
	// pkt_data[1]='b';
	// buildPacket(pkt,DATA_PACKET,pkt_data,0);
	// printPacket(pkt);
	//strcpy(pkt,"TESTTT");
	// for(i = 0;i<PACKET_SIZE;i++){
	// 	printf("pkt i, int, char, %d, %d, %c\n",i,(unsigned char)pkt[i],pkt[i]);
	// }
	//
	// if( send(sockfd , pkt, PACKET_SIZE, 0) < 0)
	// 	perror("Send failed");
	//
	// return;
	//keep communicating with server
	while(1)
	{
		//init window
		window[0] = no_of_pkts+0;
		window[1] = no_of_pkts+1;
		window[2] = no_of_pkts+2;

		//init max window number
		maxSeqNum = -1;
		//read message from user
		printf("Enter message : \n");
		fgets(msg, 100 , stdin);
		msg_len = strlen(msg) - 1;
		printf("\nlen of msg: %d \n",msg_len);

		//determine number of packets to send
		if((msg_len % 2) != 0 )
			no_of_pkts = msg_len/2 + no_of_pkts;
		else{
			msg[msg_len] = '\0';//add null terminator
			no_of_pkts = (++msg_len)/2 + no_of_pkts;
		}
		printf("num of pkts: %d \n",no_of_pkts);




		//sends window of packets each time until no more to send
		while( maxSeqNum < no_of_pkts ) /*go back N is to be done*/
		{
			//define
			j = 0; //counter to determin number of packets tranmitted
		  nak = 0; //boolean statement if a nak was sent this window

			// printf("Contents in window \n");
			// for(k=0; k < WINDOW_SIZE; k++){
			// 	printf(" %d", window[k]);
			// }
			// printf("\n");
			//
			// printf("Contents in win_seq_nos \n");
			// for(k=0; k < WINDOW_SIZE; k++){
			// 	printf(" %d", win_seq_nos[k]);
			// }
			//printf("\n");

			printf("maxSeqNum before is %d \n", maxSeqNum);

			//create packet to send
			for(i=0; (i < WINDOW_SIZE) && (window[i] < no_of_pkts); i++){

				printf("seq no. is: %d \n",window[i]);

				pkt_data[0] = msg[window[i]*2];
				pkt_data[1] = msg[window[i]*2 + 1];

				printf("Contents of pkt_data: %c %c ; msg[%d]: %c msg[%d]: %c \n",pkt_data[0],pkt_data[1],window[i]*2,msg[window[i]*2],window[i]*2+1,msg[window[i]*2+1]);

				buildPacket(pkt,DATA_PACKET,pkt_data,window[i]);
				printf("Contents of packet: \n");
				printPacket(pkt);
				printf("\n");

				IntroduceError(pkt, ber);

				// Send some data to the receiver
				// pkt - the message to be sent
				// strlen(pkt) - length of the message
				// 0 - Options and are not necessary here
				// If return value < 0, an error occured

				if( send(sockfd , pkt, PACKET_SIZE, 0) < 0)
				  perror("Send failed");


				// printf("wait: \n");
				// char test[6];
				// fgets(test, 6 , stdin);
			}

				// Receive a reply from the server
				// NOTE: If ynakou have more data than 149 bytes, it will
				// 			be received in the next call to "recv"
				// read_size - the length of the received message
				// 				or an error code in case of failure
				// msg - a buffer where the received message will be stored
				// 149 - the size of the receiving buffer (any more data will be
				// 		delievered in subsequent "recv" operations
				// 0 - Options and are not necessary here

				//if( (read_size = recv(sockfd , srv_reply , 149 , 0)) < 0)
				//perror("recv failed");

				//loop  to recieve ack/naks
				printf("start send loop <<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
			while((read_size = recv(sockfd , srv_reply , PACKET_SIZE , 0)) > 0){
				// Null termination since we need to print it
				srv_reply[read_size] = '\0';

				printf("Size of reply: %d\n",read_size);
				printf("Server's reply is \n");
				printPacket(srv_reply);
				printf("\n");

				//check if NAK_PACKET
				unsigned char pktType = srv_reply[0];
				if(pktType == NAK_PACKET){
					printf("NAC Packet Recieved\n");
					nak = 1;
					pktSeqNum = srv_reply[1];

					printf("seq no. of NAC is: %d \n",srv_reply[1]);

					// if(pktSeqNum > maxSeqNum)
					// 	maxSeqNum = pktSeqNum;

				}else if(pktType == ACK_PACKET){
					printf("ACK Packet Recieved --------------------\n");
					pktSeqNum = srv_reply[1];

					printf("seq no. of ACK is: %d \n",pktSeqNum);

					if(pktSeqNum > maxSeqNum)
						maxSeqNum = pktSeqNum;

				}else if(pktType == DATA_PACKET){
					printf("Data packet recieved\n");
				}
				j++;
				if(j==3 || j==(no_of_pkts-(window[0])))
				  break;
			}

			printf("maxSeqNum after is %d \n", maxSeqNum);
			printf("end loop >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
			//shift window
			if(!nak)
			  shift = contains(window,WINDOW_SIZE,maxSeqNum-1); //finds index of maxSeqNum in window array .. +1 was there b4
			else
			  shift = contains(window,WINDOW_SIZE,maxSeqNum);
											//computes shift from this index + 1


			if(shift != -1){

			  if(!nak)
			    shifter = shift+1;
			  else
			    shifter = shift;

				printf("shifter value is: %d shift: %d\nnak: %d\n", shifter, shift,nak);
			  shiftWindow(window,WINDOW_SIZE,shifter);

			  for(k = 0; k < WINDOW_SIZE; ++k)
			    {
			      win_seq_nos[k] += shifter;
			    }
			}

		}
	}
}
