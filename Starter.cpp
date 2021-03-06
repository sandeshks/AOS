#include "Starter.h"

Starter::Starter() {
	printf("In Starter\n");
	
	//decideAlgorithm();
}

Starter::~Starter() {
	for(int i = 0; i < NumNodes; ++i)
		delete Quorum[i];
	delete Quorum;
}

void Starter::init(){
	//char controllerIP[15] = "129.110.92.15";
	char controllerIP[15] = "10.176.67.108";
	int port = LISTEN_PORT;
	registerAtController(controllerIP, port);
	decideAlgorithm();
}

void Starter::parseMsg(const string& s,const string& delim,std::vector<string>& tokens)
{
        size_t s0 = 0, s1 = string::npos;
        string token;
        while(s0 != string::npos)
          {

            s1 = s.find_first_of(delim, s0);

                // if(s1 != s0 && s1 != string::npos) // the second check added for space
				if(s1 != s0) // no need check Defect FIX  1,2 -> 2 will not be added in the list
            {
                  token = s.substr(s0, s1 - s0);

                if( token != "") 	
                {
                    //cout<<"Token :"<<token<<" length:"<<token.length()<<endl;
						//cout<<token<<endl;
					   tokens.push_back(token);
                }
            }
            s0 = s.find_first_not_of(delim, s1);
        }
        //EXIT
        token.clear();
        return;

    }

	


/*
 * send a message to controller when a node starts on a amachine.
 */
void Starter::registerAtController(char controllerIP[15],int port){
	//communication

	communication com;
	int sockfd = com.connectToServer(controllerIP,port);

	int z;
	char buf[128]={'\0',};
	z = gethostname(buf,sizeof buf);
	if ( z == -1 ) {
		fprintf(stderr, "%s: gethostname(2)\n",
		strerror(errno));
		exit(1);
	}
	int sd = 999;
	printf("\nContacted Controller, sent my hostname:%s\n",buf);
	com.writeToSocket(sockfd,buf,128);
	com.readFromSocket(sockfd,&id,sizeof(int));
	int qsize,nsize;
	com.readFromSocket(sockfd,&qsize,sizeof(int));
	com.readFromSocket(sockfd,&nsize,sizeof(int));
	//int size;
	//com.readFromSocket(sockfd,&size,sizeof(int));
	printf("Received ID: %d, Qsize: %d, NumNodes: %d\n",id,qsize,nsize);
	NumNodes = nsize;
	quorumSize = qsize;
	Quorum = new int*[nsize];
	for(int i = 0; i < nsize; ++i)
		Quorum[i] = new int[qsize];
	
	for (int i = 0; i < nsize; ++i) {
        for (int j = 0; j < qsize; ++j) {
            Quorum[i][j] = 0;
        }
    }
	/*for(int i=0;i<nsize;i++){
		for(int j=0;j<qsize;j++){
			printf("%d\t",Quorum[i][j]);
		}
		printf("\n");			
	}*/
	puts("Getting quorum table\n");
	char recvdMsg[4096] = {'\0',};
	com.readFromSocket(sockfd,recvdMsg,4095);
	string recvdStr = recvdMsg;
	vector<string> recvdValues;
	string delimiter = ":";
	parseMsg(recvdStr,delimiter,recvdValues);
	/*
	for (int i=0; i<recvdValues.size();i++)
	{
		cout<<recvdValues[i]<<endl; //<-- extract values from this vector and push into the attay in 123 line
		
	}*/
	for(int i=0; i<nsize;i++)
	{
		for(int j=0; j<qsize;j++)
		{
			//cout<<"Here"<<endl;
			Quorum[i][j] = atoi(recvdValues[i*qsize+j].c_str());
			//cout<<"Here1"<<endl;
		}
	}
	
	for(int i=0;i<nsize;i++){
		for(int j=0;j<qsize;j++){
	
			printf("%d\t",Quorum[i][j]);
		}
		printf("\n");	
		
	}
	shutdown(sockfd,2);
	int k = com.closeSocket(sockfd);
	if (k < 0) {
			printf("\nError in Closing");
			exit(0);
	}else
		printf("Node disconnected from Controller\n");

}

void Starter::decideAlgorithm(){
	printf("\nIn decideAlgorithm() of Node\n");
	communication com;
	int serfd;
	int clifd = com.OpenListener(serfd,LISTEN_PORT2);
	//char buffer[4095]={'\0',};
	int intbuf;
	com.readFromSocket(clifd,&intbuf,sizeof(int));
	printf("recvd: %d\n",intbuf);
	puts("Getting ID to IP map\n");
	//assigining memory to mapIDtoIP
	mapIDtoIP = new char*[NumNodes];
	for(int i = 0; i < NumNodes; ++i){
		mapIDtoIP[i] = new char[MAXLENGTH_IP_ADDR];
	}

	char recvdMsg[4096] = {'\0',};
	strcpy(mapIDtoIP[0],"hello");
	com.readFromSocket(clifd,recvdMsg,4095);
	string recvdStr = recvdMsg;
	vector<string> recvdValues;
	string delimiter = ":";
	parseMsg(recvdStr,delimiter,recvdValues);
	for(int i=0;i<NumNodes;i++){
		strcpy(mapIDtoIP[i],recvdValues[i].c_str());
		printf("%d\t%s\n",i,mapIDtoIP[i]);
	}
	shutdown(clifd,2);
	com.closeSocket(clifd);
	com.closeSocket(serfd);
	int algo = intbuf;
	if(algo == 1){
		Algorithm1();
	}else if(algo == 2){
		Algorithm2();

	}else
		printf("Invalid input\n");

}

void Starter::Algorithm1(){
	printf("Starting Algorithm 1: Maekawa\n");
}

void Starter::Algorithm2(){
	printf("Starting Algorithm 2: Torum\n");
	node = Torum::getInstance();
	node->init();
	node->setID(id);
	node->getQuorumTable(Quorum,quorumSize,NumNodes);

	wqueue<Packet*> queue;
	pthread_t ProcessingThread;
	pthread_create(&ProcessingThread, NULL, TorumProcess,(void *)&queue);

	pthread_t ListenerThread;
	pthread_create(&ListenerThread, NULL, TorumListen, (void *)&queue);

	pthread_join(ProcessingThread, NULL);
	pthread_join(ListenerThread, NULL);
	printf("Torum execution done\n");
}

void *TorumListen(void* queue) {
	printf("Listener created");
	wqueue<Packet*> m_queue=*((wqueue<Packet*>*)queue);
	communication com;
	com.serverListen(LISTEN_PORT,m_queue);
	return NULL;
}


void Starter::MakeReq()
{
	node->requestCS();
}
void *TorumProcess(void* queue) {
	printf("Process created");
	wqueue<Packet*>*m_queue=(wqueue<Packet*>*)queue;

	// Remove 1 item at a time and process it. Blocks if no items are
	// available to process.
	for (int i = 0;; i++) {
		printf("thread , loop %d - waiting for item...\n", i);
		Packet* item = m_queue->remove();
		printf("thread  loop %d - got one item\n", i);
		printf("thread loop %d - item: messageType - %d, SEQ number - %ld\n",
				i, item->TYPE, item->SEQ);
		if (item->TYPE == 999) {
			//Recieve Quorum table and NodeID
		}
		if (item->TYPE == SEND_TOKEN){
			printf("Token Recieved from %d",item->ORIGIN);
			//inform all you quorum members
		}
		if (item->TYPE == MAKE_REQUEST){
			printf("Make Request recieved from Controller %d and packet type is %d",item->ORIGIN,item->TYPE);
			Starter s;
			s.MakeReq();
		}

		delete item;
	}
	return NULL;
}
