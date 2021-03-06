#include "Controller.h"
#include <iostream>
using namespace std;




//Msg type=100
Controller::Controller(void):QuorumTable{{0, 1, 2, 3, 4, 8, 12}, 
		{0, 1, 2, 3, 5, 9, 13}/*,
		{0, 1, 2, 3, 6, 10, 14},
		{0, 1, 2, 3, 7, 11, 15},
		{4, 5, 6, 7, 0, 8, 12},
		{4, 5, 6, 7, 1, 9, 13},
		{4, 5, 6, 7, 2, 10, 14},
		{4, 5, 6, 7, 3, 11, 15},
		{8, 9, 10, 11, 0, 4, 12},
		{8, 9, 10, 11, 1, 5, 13},
		{8, 9, 10, 11, 2, 6, 14},
		{8, 9, 10, 11, 3, 7, 15},
		{12, 13, 14, 15, 0, 4, 8},
		{12, 13, 14, 15, 1, 5, 9},
		{12, 13, 14, 15, 2, 6, 10},
		{12, 13, 14, 15, 3, 7, 11}*/
}
{
	Algorithm = 0;
}

Controller::~Controller(void)
{
	//delete QuorumTable;
}

void DieWithError1(char* errorMessage)
{
    perror(errorMessage);
    exit(1);
}

void DieWithError1(char *errorMessage);  /* Error handling function */

void Controller::handle(int clntSock1,char* client_ip,int counter,Controller *con)
{
	/*QuorumTable[16][7]={{0, 1, 2, 3, 4, 8, 12}, 
											{0, 1, 2, 3, 5, 9, 13},
											{0, 1, 2, 3, 6, 10, 14},
											{0, 1, 2, 3, 7, 11, 15},
											{4, 5, 6, 7, 0, 8, 12},
											{4, 5, 6, 7, 1, 9, 13},
											{4, 5, 6, 7, 2, 10, 14},
											{4, 5, 6, 7, 3, 11, 15},
											{8, 9, 10, 11, 0, 4, 12},
											{8, 9, 10, 11, 1, 5, 13},
											{8, 9, 10, 11, 2, 6, 14},
											{8, 9, 10, 11, 3, 7, 15},
											{12, 13, 14, 15, 0, 4, 8},
											{12, 13, 14, 15, 1, 5, 9},
											{12, 13, 14, 15, 2, 6, 10},
											{12, 13, 14, 15, 3, 7, 11}
	};*/
	
	communication com;
	char msg[128]={'\0',};
	int sd=0;
	com.readFromSocket(clntSock1,msg,128);
	puts("returned\n");
	printf("Client %s is UP",msg);
	puts("\n");
	int qColSize=QuorumSize;
	int qRowSize=MAXNODES;
	puts("\n");
	com.writeToSocket(clntSock1,&counter,sizeof(int));
	com.writeToSocket(clntSock1,&qColSize,sizeof(int));
	com.writeToSocket(clntSock1,&qRowSize,sizeof(int));
	puts("quorum table");
	for(int i=0;i<qRowSize;i++){
		for(int j=0;j<qColSize;j++){
			printf("%d\t",QuorumTable[i][j]);
		}
		printf("\n");	
		
	}
	string strToSend,tempStr; strToSend.clear(); tempStr.clear();
	char temp[5] = {0,};
	for(int i=0;i<qRowSize;i++){
		for(int j=0;j<qColSize;j++){
			snprintf(temp,4,"%d",QuorumTable[i][j]);
			tempStr = temp;
			strToSend += tempStr;
			strToSend += ":";
			memset(&temp,0,sizeof(temp));
			tempStr.clear();
		}
		//printf("size: %d\n",sizeof(row)*sizeof(int));
		//com.writeToSocket(clntSock1,row,sizeof(row)*sizeof(int));	
		
	}
	com.writeToSocket(clntSock1,const_cast<char*> (strToSend.c_str()), strToSend.length());	
	cout<<"Sent Msg - "<<strToSend<<endl;
	
	printf("Counter value %d, ",counter);
	strcpy(mapIPtoID[counter], client_ip);
	printf("MaptoIP %s \n",mapIPtoID[counter]);

}

void *listener(void* c) {
	
	Controller *con = (Controller*)c;
	printf("In Listener\n");
	/*communication com;
	com.serverListen(1235,m_queue);*/
	int counter=0;
	int servSock;                    /* Socket descriptor for server */
	int clntSock;                    /* Socket descriptor for client */
	struct sockaddr_in echoServAddr; /* Local address */
	struct sockaddr_in echoClntAddr; /* Client address */
	unsigned short echoServPort;     /* Server port */
	socklen_t clntLen;            /* Length of client address data structure */


	echoServPort = LISTEN_PORT;  /* First arg:  local port */

	/* Create socket for incoming connections */
	if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		DieWithError1("socket() failed");

	/* Construct local address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
	echoServAddr.sin_family = AF_INET;                /* Internet address family */
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	echoServAddr.sin_port = htons(echoServPort);      /* Local port */

	/* Bind to the local address */
	if (bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
		DieWithError1("bind() failed");

	/* Mark the socket so it will listen for incoming connections */
	if (listen(servSock, MAXPENDING) < 0)
		DieWithError1("listen() failed");
	//int del=0;
	for (;counter<MAXNODES;) /* Run forever */
	{
		/* Set the size of the in-out parameter */
		clntLen = sizeof(echoClntAddr);

		/* Wait for a client to connect */
		if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr,&clntLen)) < 0)
		{
			close(servSock);
			DieWithError1("accept() failed");
		}

		/* clntSock is connected to a client! */
		char *client_ip = inet_ntoa(echoClntAddr.sin_addr);
		printf("\nClient socket %d, addlen : %d %s\n",clntSock,sizeof(client_ip),client_ip);
		con->handle(clntSock,client_ip,counter,con);
		counter++;
		shutdown(clntSock,0);
		int k = close(clntSock);
		if (k < 0) {
				printf("\nError in Closing");
				exit(0);
		}
	}
	shutdown(servSock,2);
	int k = close(servSock);
	if (k < 0) {
		printf("\nError in Closing");
		exit(0);
	}

	return NULL;
}

void sendTokenToNode()
{
	printf("In send token");
	//Send token
	Packet pack;
	pack.TYPE=SEND_TOKEN;
	pack.ORIGIN=45;
	pack.SEQ=1;
	pack.sender=45;
	/*
	communication com;
	printf("Map to ID %s",mapIPtoID[0]);
	char desIP[15];
	strncpy(desIP,mapIPtoID[1],15);
	printf("des to ID %s",desIP);

	com.sendMessage(pack,desIP,LISTEN_PORT);*/
}

void Controller::decideAlgorithm(){
	printf("Enter the Algorithm for the nodes to follow:\n");
	printf("\t\t1:Maekawa\t2:Torum\n");
	cin>>Algorithm;
	/*char *ip = "10.102.10.3";
	strcpy(mapIPtoID[0],ip);
	//delete ip;
	char *ip2 = "111.111.111.111";
	strcpy(mapIPtoID[1],ip2);
*/
	communication con;
	for(int i=0;i<MAXNODES;i++){
		printf("mapIP:%s\n",mapIPtoID[i]);
		int sockfd = con.connectToServer(mapIPtoID[i],LISTEN_PORT2);

		con.writeToSocket(sockfd,&Algorithm,sizeof(int));
		string strToSend,tempStr; strToSend.clear(); tempStr.clear();
		for(int j=0;j<MAXNODES;j++){
			//printf("ipaddress:%s\n",mapIPtoID[j]);
			tempStr = mapIPtoID[j];
			strToSend += tempStr;
			strToSend += ":";
			tempStr.clear();
		}
		printf("strtosend: %s",strToSend.c_str());
		con.writeToSocket(sockfd,const_cast<char*> (strToSend.c_str()),4095);
		shutdown(sockfd,0);
		close(sockfd);
	}


	if(Algorithm == 1){
		Algorithm1();
	}else if(Algorithm == 2){
		Algorithm2();

	}else
		printf("Invalid input\n");

}

void Controller::Algorithm1(){

}

void Controller::Algorithm2(){
	printf("\nYou have chosen Token and Quorum Based Mutual Exclusion Algorithm: Torum\n");
	sendTokenToNode();
	Packet pack1;
communication com;
		pack1.TYPE=MAKE_REQUEST;
		pack1.ORIGIN=45;
		pack1.SEQ=23;
		pack1.sender=45;
	char desIP[15]="129.110.92.15";
	
	printf("des to ID %s",desIP);
		com.sendMessage(pack1,desIP,LISTEN_PORT);

}

void Controller::initiate(Controller *c){
	pthread_t listen;
	pthread_create(&listen, NULL,listener,(void*)c );
	pthread_join(listen,NULL);

}
int main()
{
/*	int i;
	for(i=0; i <MAXNODES ; i++) {
		mapIPtoID[i] = (char *)malloc(sizeof(char)*MAXLENGTH_IP_ADDR);
	}
	*/
	Controller *c = new Controller;

	printf("Welcome to Controller Function!!!\n");
	c->initiate(c);
	c->decideAlgorithm();
		
		return 0;
}
