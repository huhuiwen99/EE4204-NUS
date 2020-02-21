/*#include "headsock.h"

void str_ser1(int sockfd);                                                           // transmitting and receiving function

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in my_addr;
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0); //create socket

	if (socket<0) {			
		printf("error in socket");
		exit(1);
	}
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYUDP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero), 8);
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {           //bind socket
		printf("error in binding");
		exit(1);
	}
	printf("start receiving\n");
	while(1) {
		str_ser1(sockfd);                        // send and receive
	}
	close(sockfd);
	exit(0);
}

#include "headsock.h"

// Function Protoypes
void readfromsocket(int sockfd); // Receive data from socket sockfd
void send_ack(int sockfd, struct sockaddr *client_addr, socklen_t client_addrlen, long fileoffset); // Block until acknowledge is sent

int main(int argc, char *argv[]) {
    // Setup server socket address
    struct sockaddr_in server_addr_in;
    server_addr_in.sin_family = AF_INET;
    server_addr_in.sin_port = htons(MYUDP_PORT); // htons() converts port number to big endian
    server_addr_in.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY=0, 0 means receive data from any IP address
    memset(&(server_addr_in.sin_zero), 0, sizeof(server_addr_in.sin_zero));
    // Typecast internet socket address (struct sockaddr_in) to generic socket address (struct sockaddr)
    struct sockaddr *server_addr = (struct sockaddr *)&server_addr_in;
    socklen_t server_addrlen = sizeof(struct sockaddr);

    // Create UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { printf("error creating socket"); exit(1); }

    // Bind socket address to socket (server only, client don't need)
    if (bind(sockfd, server_addr, server_addrlen) == -1) { printf("error in binding"); exit(1); }

    // Read file data from socket
    while (1) {
        printf("Ready to receive data\n");
        readfromsocket(sockfd);
    }
    close(sockfd);

    // Compare myfile.txt and myUDPreceive.txt
    printf("Differences between myfile.txt and myUDPreceive.txt:\n");
    char * const args[]={"diff", "myfile.txt", "myUDPreceive.txt"}; 
    execv("/usr/bin/diff", args);
}

void readfromsocket(int sockfd) {
    //---------------------------------------------//
    // buffer used to contain the incoming packet. //
    //---------------------------------------------//
    char packet[DATAUNIT];

    // Create empty struct to contain client address
    // It will be filled in by recvfrom()
    struct sockaddr client_addr;
    socklen_t client_addrlen = sizeof(struct sockaddr);

    // Get filesize from client
    long filesize;
    int n = recvfrom(sockfd, &filesize, sizeof(filesize), 0, &client_addr, &client_addrlen);
    if (n < 0) { printf("error in receiving packet\n"); exit(1); }
    send_ack(sockfd, &client_addr, client_addrlen, 0); // Acknowledge that filesize has been received
    printf("Client says the file is %ld bytes big. DATAUNIT %d bytes\n", filesize-1, DATAUNIT);

    //----------------------------------------//
    // buffer used to contain the entire file //
    //----------------------------------------//
    char filebuffer[filesize];

    int dum = 1; // data unit multiple
    long fileoffset = 0; // Tracks how many bytes have been received so far
    char packetlastbyte; // Tracks the last byte of the latest packet received
    //-----------------------------------------------------------------------------------------------------------//
    // Keep reading from socket until the last byte of the latest packet is an End of Transmission character 0x4 //
    //-----------------------------------------------------------------------------------------------------------//
    do {
        // Depending on dum, read packets of DATAUNIT size 1, 2, 3 or 4 times before sending an ACK
        for (int i=0; i<dum; i++) {
            // Read incoming packet (recvfrom will block until data is received)
            int bytesreceived = recvfrom(sockfd, &packet, DATAUNIT, 0, &client_addr, &client_addrlen);
            if (bytesreceived < 0) {
                printf("error in receiving packet\n");
                exit(1);
            }

            // Append packet data to filebuffer
            memcpy((filebuffer + fileoffset), packet, bytesreceived);
            fileoffset += bytesreceived;
            if ((packetlastbyte = packet[bytesreceived-1]) == 0x4) break;
        }

        // Acknowledge that packet has been received
        send_ack(sockfd, &client_addr, client_addrlen, fileoffset);
        dum = (++dum % 5 == 0) ? 1 : dum % 5;
    } while (packetlastbyte != 0x4);
    fileoffset-=1; // Disregard the last byte of filebuffer because it is the End of Transmission character 0x4

    // Open file for writing
    char filename[] = "myUDPreceive.txt";
    FILE* fp = fopen(filename, "wt");
    if (fp == NULL) { printf("File %s doesn't exist\n", filename); exit(1); }

    // Copy the filebuffer contents into file
    fwrite(filebuffer, 1, fileoffset, fp);
    fclose(fp);
    printf("File data received successfully, %d bytes written\n", (int)fileoffset);
}

void send_ack(int sockfd, struct sockaddr *addr, socklen_t addrlen, long fileoffset) {
    const int ACKNOWLEDGE = 1;
    int ack_sent = 0;
    int ack_thresh = 10;
    while (!ack_sent) {
        if (sendto(sockfd, &ACKNOWLEDGE, sizeof(ACKNOWLEDGE), 0, addr, addrlen) >= 0) {
            ack_sent = 1;
        } else {
            if (ack_thresh-- <= 0) {
                printf("(%ld) emergency breakout of ack error loop\n", fileoffset * DATAUNIT);
                exit(1);
            } else printf("error sending ack, trying again..\n");
        }
    }
}*/
/**********************************
udp_ser.c: the source file of the server in udp transmission
***********************************/

#include "headsock.h"

void str_ser(int sockfd, struct sockaddr *addr, int addrlen);             // transmitting and receiving function

void compareFiles();	// check if the file received is correct by comparing with original

int main(void)
{
	int sockfd, ret;
	struct sockaddr_in my_addr;
  
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);          //create socket
	if (sockfd<0)
	{
		printf("error in socket!");
		exit(1);
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYUDP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;//inet_addr("172.0.0.1");
	bzero(&(my_addr.sin_zero), 8);
	ret = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));                //bind socket
	if (ret<0)
	{
		printf("error in binding");
		exit(1);
	}
	
	while(1) {
		printf("waiting for data\n");
		str_ser(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in));                //receive packet and response
		compareFiles();		
	}

	close(sockfd);

	exit(0);
}

void str_ser(int sockfd, struct sockaddr *addr, int addrlen)
{
	char buf[BUFSIZE];
	FILE *fp;
	char recvs[4*DATALEN];
	struct ack_so ack;
	int end, n = 0;
	long lseek=0;
	end = 0;

	while(!end)
	{
   
		if ((n= recvfrom(sockfd, &recvs, 4*DATALEN, 0, addr, (socklen_t *)&addrlen))==-1)  //receive the packet
		{
			printf("error when receiving\n");
			exit(1);
		}
			
		if (recvs[n-1] == '\0')									//if it is the end of the file
		{
		  end = 1;
		  n --;
		}
		memcpy((buf+lseek), recvs, n);
		lseek += n;
		
		ack.num = 1;
		  ack.len = 0;
		  if ((n = sendto(sockfd, &ack, 2, 0, addr, addrlen))==-1)
		  {
			  printf("send error!");								//send the ack
			  exit(1);
		  }
	}

	if ((fp = fopen ("myUDPreceive.txt","wt")) == NULL)
	{
		printf("File doesn't exist\n");
		exit(0);
	}
	fwrite (buf , 1 , lseek , fp);					//write data into file
	fclose(fp);
	printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);
}

void compareFiles() 
{ 
	FILE *fp1;
	FILE *fp2;
	
	if((fp1 = fopen ("myfile.txt","r+t")) == NULL)
	{
		printf("File fp1 doesn't exit\n");
		exit(0);
	}
	
	if((fp2 = fopen ("myUDPreceive.txt","r+t")) == NULL)
	{
		printf("File fp2 doesn't exit\n");
		exit(0);
	}
	
    // fetching character of two file 
    // in two variable ch1 and ch2 
    char ch1 = getc(fp1); 
    char ch2 = getc(fp2); 
  
    // error keeps track of number of errors 
    // pos keeps track of position of errors 
    // line keeps track of error line 
    int error = 0, pos = 0, line = 1; 
  
    // iterate loop till end of file 
    while (ch1 != EOF && ch2 != EOF) 
    { 
        pos++; 
  
        // if both variable encounters new 
        // line then line variable is incremented 
        // and pos variable is set to 0 
        if (ch1 == '\n' && ch2 == '\n') 
        { 
            line++; 
            pos = 0; 
        } 
  
        // if fetched data is not equal then 
        // error is incremented 
        if (ch1 != ch2) 
        { 
            error++; 
            printf("Line Number : %d \tError"
               " Position : %d \n", line, pos); 
        } 
  
        // fetching character until end of file 
        ch1 = getc(fp1); 
        ch2 = getc(fp2); 
    } 
  
    printf("Total Errors : %d\t \n", error); 
} 
