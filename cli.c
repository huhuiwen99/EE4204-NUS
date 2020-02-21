/*******************************
assignment_udp_client.c: the source file of the client in udp transmission 
********************************/

#include "headsock.h"

float str_cli(FILE *fp, int sockfd, long *len, struct sockaddr *addr, int addrlen);                  //transmission function
void tv_sub(struct  timeval *out, struct timeval *in);	    //calcu the time interval between out and in

int main(int argc, char **argv)
{
	int sockfd;
	float ti, rt;
	long len;
	struct sockaddr_in ser_addr;
	char ** pptr;
	struct hostent *sh;
	struct in_addr **addrs;
	FILE *fp;

	if (argc != 2) {
		printf("parameters not match");
	}

	sh = gethostbyname(argv[1]);	                                       //get host's information
	if (sh == NULL) {
		printf("error when gethostby name");
		exit(0);
	}

	printf("canonical name: %s\n", sh->h_name);					//print the remote host's information
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++)
		printf("the aliases name is: %s\n", *pptr);
	switch(sh->h_addrtype)
	{
		case AF_INET:
			printf("AF_INET\n");
		break;
		default:
			printf("unknown addrtype\n");
		break;
	}
        
	addrs = (struct in_addr **)sh->h_addr_list;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);                           //create the socket
	if (sockfd <0)
	{
		printf("error in socket");
		exit(1);
	}
	ser_addr.sin_family = AF_INET;                                                      
	ser_addr.sin_port = htons(MYUDP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr.sin_zero), 8);
	
	if((fp = fopen ("myfile.txt","r+t")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}

	//ti = str_cli(fp, sockfd, &len);                       //perform the transmission and receiving
	ti = str_cli(fp, sockfd, &len, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr_in));   // receive and send
	rt = (len/(float)ti);                                         //caculate the average transmission rate
	printf("Time(ms) : %.3f, Data sent(byte): %d\nData rate: %f (Kbytes/s)\n", ti, (int)len, rt);

	close(sockfd);
	fclose(fp);

	exit(0);
}

float str_cli(FILE *fp, int sockfd, long *len, struct sockaddr *addr, int addrlen)
{
	char *buf;
	long lsize, ci;
	char sends[DATALEN];
	struct ack_so ack;
	int n, slen;
	float time_inv = 0.0;
	struct timeval sendt, recvt;
	ci = 0;
	int numDU = 0; // whether to send 1 or 2 packets

	n = 0;	
	fseek (fp , 0 , SEEK_END);
	lsize = ftell (fp);
	rewind (fp);
	printf("The file length is %d bytes\n", (int)lsize);
	printf("the packet length is %d bytes\n",DATALEN);

// allocate memory to contain the whole file.
	buf = (char *) malloc (lsize);
	if (buf == NULL) exit (2);

  // copy the file into the buffer.
	fread (buf,1,lsize,fp);

  /*** the whole file is loaded in the buffer. ***/
	buf[lsize] ='\0';									//append the end byte
	gettimeofday(&sendt, NULL);							//get the current time
	while(ci<= lsize)
	{
		// numDU starts from zero
		// so actual number of packets sent == numDU + 1
		for (int i=0; i<= numDU; i++) {
			if ((lsize+1-ci) <= DATALEN)
				slen = lsize+1-ci;
			else 
				slen = DATALEN;
			memcpy(sends, (buf+ci), slen);
			//n = send(sockfd, &sends, slen, 0);
			sendto(sockfd, &sends, slen, 0, addr, addrlen);                         //send the packet to server
			if(n == -1) {
				printf("send error!");								//send the data
				exit(1);
			}
			ci += slen;
		}
		
		if ((n=recvfrom(sockfd, &ack, 2, 0, addr, (socklen_t *)&addrlen)) == -1) {      //receive the packet
			printf("error receiving");
			exit(1);
		}
		
		if (ack.num != 1|| ack.len != 0)
			printf("error in transmission\n");

		// Make sure numDU varies between 0, 1
		numDU++;
		numDU %= 2;
	}
	
	gettimeofday(&recvt, NULL);
	*len= ci;                                                         //get current time
	tv_sub(&recvt, &sendt);                                                                 // get the whole trans time
	time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;
	return(time_inv);
}

void tv_sub(struct  timeval *out, struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) <0)
	{
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}


/*socket udp 服务端
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>

int main()
{
    //创建socket对象
    int sockfd=socket(AF_INET,SOCK_DGRAM,0);

    //创建网络通信对象
    struct sockaddr_in addr;
    addr.sin_family =AF_INET;
    addr.sin_port =htons(1324);
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");

    //绑定socket对象与通信链接
    int ret =bind(sockfd,(struct sockaddr*)&addr,sizeof(addr));
    if(0>ret)
    {
        printf("bind\n");
        return -1;

    }
    struct sockaddr_in cli;
    socklen_t len=sizeof(cli);

    while(1)
    {
        char buf =0;
        recvfrom(sockfd,&buf,sizeof(buf),0,(struct sockaddr*)&cli,&len);
        printf("recv num =%hhd\n",buf);

        buf =66;
        sendto(sockfd,&buf,sizeof(buf),0,(struct sockaddr*)&cli,len);

    }
    close(sockfd);

}
#include "headsock.h"
#include <arpa/inet.h>

// Function Prototypes
void sendtosocket(int sockfd, struct sockaddr *addr, socklen_t addrlen); // Send data to socket address addr through socket sockfd
void tv_sub(struct  timeval *out, struct timeval *in); // Calculate the time interval between out and in
void wait_ack(int sockfd, struct sockaddr *addr, socklen_t addrlen); // Block until acknowledge is received

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Please provide an IP_addr. Example: ./server.out 127.0.0.1\n");
        printf("'localhost' does not work, use 127.0.0.1 instead of 'localhost'\n");
        exit(1);
    }
    char *IP_addr = argv[1];

    // Setup server socket address
    struct sockaddr_in server_addr_in;
    server_addr_in.sin_family = AF_INET;
    server_addr_in.sin_port = htons(MYUDP_PORT); // htons() converts port number to big endian form
    server_addr_in.sin_addr.s_addr = inet_addr(IP_addr); // inet_addr converts IP_addr string to big endian form
    memset(&(server_addr_in.sin_zero), 0, sizeof(server_addr_in.sin_zero));
    // Typecast internet socket address (struct sockaddr_in) to generic socket address (struct sockaddr)
    struct sockaddr *server_addr = (struct sockaddr *)&server_addr_in;
    socklen_t server_addrlen = sizeof(struct sockaddr);

    // Create UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { printf("error creating socket"); exit(1); }

    // Send file data to socket
    sendtosocket(sockfd, server_addr, server_addrlen);
    close(sockfd);
}
void sendtosocket(int sockfd, struct sockaddr *server_addr, socklen_t server_addrlen) {
    //--------------------------------------------//
    // buffer used to contain the outgoing packet //
    //--------------------------------------------//
    char packet[DATAUNIT];

    // Open file for reading
    char filename[] = "myfile.txt";
    FILE* fp = fopen(filename, "rt");
    if (fp == NULL) { printf("File %s doesn't exist\n", filename); exit(1); }

    // Get the total filesize
    fseek(fp, 0, SEEK_END); long filesize = ftell(fp); rewind(fp);
    printf("The file length is %d bytes\n", (int)filesize);

    //----------------------------------------//
    // buffer used to contain the entire file //
    //----------------------------------------//
    char filebuffer[filesize];

    // Copy the file contents into filebuffer
    fread(filebuffer, 1, filesize, fp);
    filebuffer[filesize]=0x4; // Append the filebuffer contents with the End of Transmission ASCII character 0x4
    filesize+=1; // Increase filesize by 1 byte because of the addition of the EOT ASCII character 0x4
    fclose(fp);

    // Tell server how big the file is and wait for server acknowledgement
    sendto(sockfd, &filesize, sizeof(filesize), 0, server_addr, server_addrlen);
    wait_ack(sockfd, server_addr, server_addrlen);

    // Get start time
    struct timeval timeSend;
    gettimeofday(&timeSend, NULL);

    long fileoffset = 0; // Tracks how many bytes have been sent so far
    int dum = 1; // data unit multiple, alternates between 1,2,3,4
    //---------------------------------------------------------------------------------//
    // Keep sending data until number of bytes sent is no longer smaller than filesize //
    //---------------------------------------------------------------------------------//
    while (fileoffset < filesize) {
        // Depending on dum, send packets of DATAUNIT size 1, 2, 3 or 4 times before waiting for an ACK
        for (int i=0; i<dum; i++) {
            int packetsize = (DATAUNIT < filesize - fileoffset) ? DATAUNIT : filesize - fileoffset; // packetsize should never be bigger than what's left to send
            memcpy(packet, (filebuffer + fileoffset), packetsize); // Copy the next section of the filebuffer into the packet
            fileoffset += packetsize; // Update fileoffset
            int n = sendto(sockfd, &packet, packetsize, 0, server_addr, server_addrlen); // Send packet data into socket
            if (n < 0) {
                printf("error in sending packet\n");
                exit(1);
            }
            printf("packet of size %d sent\n", n);
        }
        wait_ack(sockfd, server_addr, server_addrlen);
        dum = (++dum % 5 == 0) ? 1 : dum % 5;
    }

    // Get end time
    struct timeval timeRcv;
    gettimeofday(&timeRcv, NULL);

    // Calculate difference between start and end time and print transfer rate
    tv_sub(&timeRcv, &timeSend);
    float time = (timeRcv.tv_sec)*1000.0 + (timeRcv.tv_usec)/1000.0;
    printf("DATAUNIT %d bytes | %ld bytes sent over %.3fms | %.3f Mbytes/s\n", DATAUNIT, fileoffset, time, fileoffset/time/1000);
}

void tv_sub(struct  timeval *out, struct timeval *in) {
    if ((out->tv_usec -= in->tv_usec) <0) {
        --out ->tv_sec;
        out ->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}

void wait_ack(int sockfd, struct sockaddr *addr, socklen_t addrlen) {
    int ack_received = 0;
    int ACKNOWLEDGE = 0;
    while (!ack_received) {
        if (recvfrom(sockfd, &ACKNOWLEDGE, sizeof(ACKNOWLEDGE), 0, addr, &addrlen) >= 0) {
            if (ACKNOWLEDGE == 1) {
                ack_received = 1;
                printf("ACKNOWLEDGE received\n");
            } else {
                printf("ACKNOWLEDGE received but value was not 1\n");
                exit(1);
            }
        } else {
            printf("error when waiting for acknowledge\n");
            exit(1);
        }
    }
}*
