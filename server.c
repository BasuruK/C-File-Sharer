/**
 * Basuru Kusal
 * 18874536
 * Server.c => acts as a server for all the clients
 */

#include "stuff.h"

typedef struct node
{
    //file content
    char fileName[20];
    char clientAddr[20];
    int clientPort;
    char uploadTime[24];
    char lastDownloadTime[50];
    int totalDownloads;
    int socketDescriptor;

    //pointer to next node
    struct node *next;

} node;

//set parameters NULL in linked list.
node *start = NULL;
node *new = NULL;

int maxClients = 0;
int sharedArray[];
struct sockaddr_in cliaddr, servaddr;
int timeExpire;

void AddFile(char *fileName, char *clientAddr, int clientPort,int sockfd);
void printNodes(node * current);
node * deleteNode(char *filename, node * start);
void sig_handler(int sig);
void str_echo(int fd,struct sockaddr_in cliaddr,int maxClients,int * clientA);
static void * doit(void * arg);

int main(int argc, char **argv)
{

    //main program runs here

    //Server code begin
    int listenfd, connfd, *iptr;
    maxClients = atoi(argv[1]);
    tv.tv_sec = atoi(argv[2]);
    timeExpire = atoi(argv[2]);
    pid_t childpid;
    pthread_t tid;
    socklen_t clilen,len;



    if((listenfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        printf("\nError: Could not create socket \n");
        return 1;
    }
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family         = AF_INET;
    servaddr.sin_addr.s_addr    = htonl(INADDR_ANY);
    servaddr.sin_port           = htons(SERV_TCP_PORT);

    bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
    listen(listenfd, maxClients);

    printf("Waiting for incoming connections! \n");

    for(int i = 0; i <= maxClients; i++)
        sharedArray[i]= -1;

    for(;;)
    {
        iptr = malloc(sizeof(int));
        clilen = sizeof(cliaddr);
        len = clilen;
        if ((*iptr = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0)
        {
            if (errno == EINTR) {
                continue;
            } else {
                perror("Accept Error");
                exit(1);
            }
        }

        int m;
        for(m = 0; m <= maxClients; m++)
        {
            if(sharedArray[m] < 0)
            {
                sharedArray[m] = *iptr;
                break;
            }
        }

        //if too many clients
        if(m == maxClients)
        {
            perror("Too Many Clients!");
            close(*iptr);
            free(iptr);
        }

        //print client connection
        printf("Client Connected IP: %s Port: %d \n",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));
        pthread_create(&tid, NULL, &doit,iptr);
    }

    return 0;
}

/**
 * prints the entire list in a tabular format
 * @param currunt
 */
void printNodes(node * current)
{
    printf("File Name | Client Address | Client Port# |\t\t\t Upload Time \t\t| \t\tLast Download Time \t\t | Total Downloads \n");
    while(current != NULL)
    {
        printf("%s \t %s \t\t %d \t\t %s \t\t\t %s \t\t\t\t\t %d\n", current->fileName, current->clientAddr, current->clientPort,current->uploadTime,current->lastDownloadTime,current->totalDownloads);
        current = current->next;
    }
}

/**
 * This method adds an element to the end of the linked list 'node'
 *
 * @param fileName
 * @param clientAddr
 * @param clientPort
 * @param uploadTime
 * @param lastDownloadTime
 * @param previous
 * @return
 */
node * addNode(char *fileName, char *clientAddr, int clientPort, node *previous, int sockfd)
{
    //allocate memory
    node * newNode = malloc(sizeof(node));
    //assign variables
    time_t myTime = time(NULL);

    strcpy(newNode->fileName,fileName);
    strcpy(newNode->clientAddr, clientAddr);
    newNode->clientPort = clientPort;
    char time[50];
    strcpy(time,ctime(&myTime));
    time[strlen(time) - 1] = 0;
    strcpy(newNode->uploadTime,time);
    strcpy(newNode->lastDownloadTime, "");
    newNode->totalDownloads = 0;
    newNode->socketDescriptor = sockfd;
    if(previous != NULL )
    {
        previous->next = newNode;
    }

    return newNode;
}

/**
 * This method calls the node add method in order to enter elements to the linked list.
 * This method acts as the main controller for the linked list 'node'
 *
 * @param fileName
 * @param clientAddr
 * @param clientPort
 * @param uploadTime
 * @param lastDownloadTime
 * @param previous
 */
void AddFile(char *fileName, char *clientAddr, int clientPort, int sockfd)
{
    //add and elements to the beginning if its the first and adds to the end if its otherwise.
    if(start == NULL)
    {
        start = addNode(fileName, clientAddr, clientPort, NULL,sockfd);
        new = start;
    }
    else
    {
        new = addNode(fileName, clientAddr, clientPort, new,sockfd);
    }
}

/**
 * Delete a node.
 * @param filename
 * @param start
 * @return
 */
node * deleteNode(char *filename, node * start)
{
    node * previous = start;
    node * data;
    while(start != NULL)
    {
        if(strcmp(filename,start->fileName) == 0)
        {
            data = start;
            if(previous == start)
            {
                start = start->next;
                previous = start;
                break;
            }
            else
            {
                previous->next = start->next;
                break;
            }
        }
        previous = start;
        start = start->next;
    }

    return(data);
}

void resetTimer()
{
    tv.tv_sec = timeExpire;
}

/**
 * function the thread execute
 * @param arg
 * @return
 */
static void * doit(void * arg)
{
    int connfd;
    connfd = *((int *)arg);
    free(arg);
    pthread_detach(pthread_self());

    str_echo(connfd,cliaddr,maxClients,sharedArray);
    close(connfd);
    pthread_exit(NULL);
}

/**
 * The thread executes this functions
 * client is being serverd in this function.
 * @param fd
 * @param cliaddr
 * @param maxClients
 * @param clientA
 */
void str_echo(int fd,struct sockaddr_in cliaddr,int maxClients,int * clientA)
{
    ssize_t n;
    char reciveLine[MAXLINE];
    char file[MAXLINE];
    char command[MAXLINE];
    fd_set rset;
    int maxfdp1;
    int selectN;


    FD_ZERO(&rset);

    for(;;)
    {
        bzero(reciveLine,sizeof(reciveLine));
        bzero(file,sizeof(file));
        bzero(command,sizeof(command));
        FD_SET(fd,&rset);

        maxfdp1 = fd + 1;

        if((selectN = Select(maxfdp1,&rset,NULL,NULL,&tv)) == 0)
        {
            //Timeout has occurred.
            Writen(fd,"Good Bye\n",9);
            close(fd);
            FD_ZERO(&rset);
            FD_CLR(fd,&rset);
            selectN = -1;
            fd = NULL;
            resetTimer();
            return;
        }

        if(FD_ISSET(fd,&rset)) {
            if ((n = Readline(fd, reciveLine, MAXLINE)) == 0) {
                char error[20] = "Client ";
                strcat(error, inet_ntoa(cliaddr.sin_addr));
                strcat(error, " Terminated");
                perror(error);
                return;
            }
            sscanf(reciveLine, "%s %s", command, file);
        }

        if(strcmp(command,"upload") == 0)
        {
            AddFile(file, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port),fd);

            time_t myTime = time(NULL);

            char send[150] = "Upload ";
            strcat(send,file);
            strcat(send,",Location: ");
            strcat(send,inet_ntoa(cliaddr.sin_addr));
            strcat(send,",Last Download Time: ");
            strcat(send,",Total Download: 0");
            strcat(send,",Upload Time: ");
            strcat(send,ctime(&myTime));

            Writen(fd,send,strlen(send));
            bzero(send,sizeof(send));
            resetTimer();

        }
        else if(strcmp(command,"get") == 0 && strcmp(file,"table") == 0)
        {
            node dataTable[20];
            char sendTable[1000];
            bzero(dataTable,sizeof(dataTable));
            node * getCurrent = start;
            int count = 0;

            while(getCurrent != NULL)
            {
                strcpy(dataTable[count].fileName,getCurrent->fileName);
                strcpy(dataTable[count].clientAddr,getCurrent->clientAddr);
                dataTable[count].clientPort = getCurrent->clientPort;
                strcpy(dataTable[count].lastDownloadTime, getCurrent->lastDownloadTime);
                dataTable[count].totalDownloads =  getCurrent->totalDownloads;
                strcpy(dataTable[count].uploadTime, getCurrent->uploadTime);
                count++;
                getCurrent = getCurrent->next;
            }

            for(int i = 0;i < count;i++)
            {
                strcat(sendTable,dataTable[i].fileName);
                strcat(sendTable,",Location :");
                strcat(sendTable,dataTable[i].clientAddr);
                strcat(sendTable,":");
                char buffGetPort[10];
                snprintf(buffGetPort,sizeof(buffGetPort),"%d",dataTable[i].clientPort);
                strcat(sendTable,buffGetPort);
                strcat(sendTable,",Upload Time : ");
                char buffGetUpTime[80];
                strcpy(buffGetUpTime,dataTable[i].uploadTime);
                strcat(sendTable,buffGetUpTime);
                strcat(sendTable,",Last Download Time : ");
                strcat(sendTable,dataTable[i].lastDownloadTime);
                strcat(sendTable,",Total Downloads :");
                char buffGetTDown[5];
                snprintf(buffGetTDown,sizeof(buffGetTDown),"%d",dataTable[i].totalDownloads);
                strcat(sendTable,buffGetTDown);
                strcat(sendTable,",,");
            }
            strcat(sendTable,"\n");

            Writen(fd, sendTable, strlen(sendTable));
            bzero(sendTable,sizeof(sendTable));
            resetTimer();
        }
        else if(strcmp(command,"delete") == 0)
        {
            node * delete = deleteNode(file,start);

            char sendDelete[200] = "Delete ";
            strcat(sendDelete,delete->fileName);
            strcat(sendDelete,",Location :");
            strcat(sendDelete,delete->clientAddr);
            strcat(sendDelete,":");

            char buff1[32];
            snprintf(buff1,sizeof(buff1),"%d",delete->clientPort);
            strcat(sendDelete,buff1);
            strcat(sendDelete,",Last Download Time :");
            strcat(sendDelete,delete->lastDownloadTime);

            strcat(sendDelete,",Total Download: ");
            char buff[128];
            snprintf(buff,sizeof(buff),"%d",delete->totalDownloads);
            strcat(sendDelete, buff);

            strcat(sendDelete,",Upload Time : ");
            strcat(sendDelete,delete->uploadTime);
            strcat(sendDelete,"\n");

            Writen(fd,sendDelete,strlen(sendDelete));
            bzero(sendDelete,sizeof(sendDelete));
            resetTimer();
        }
        else if(strcmp(command,"download") == 0)
        {
            node * currentDownload = start;
            int requestFD = -1;
            char clientAddress[20];
            char receiveDownload[MAXLINE];

            time_t myTime = time(NULL);

            while(currentDownload != NULL)
            {
                if(strcmp(currentDownload->fileName,file) == 0 )
                {
                    requestFD = currentDownload->socketDescriptor;
                    strcpy(clientAddress,currentDownload->clientAddr);
                    currentDownload->totalDownloads = currentDownload->totalDownloads + 1;
                    char timeArray[50];
                    strcpy(timeArray,ctime(&myTime));
                    //remove the newline char
                    timeArray[strlen(timeArray) - 1] = 0;
                    strcpy(currentDownload->lastDownloadTime,timeArray);
                }
                currentDownload = currentDownload->next;
            }

            char sendRequest[50] = "Request ";
            strcat(sendRequest,file);
            strcat(sendRequest,"\n");

            //send Request
            Writen(requestFD,sendRequest,strlen(sendRequest));
            bzero(sendRequest,sizeof(sendRequest));
            //get the file to server
            Readline(requestFD,receiveDownload,MAXLINE);
            //send the file to requesting client
            Writen(fd,receiveDownload,strlen(receiveDownload));



            char timeArray1[20];
            strcpy(timeArray1,ctime(&myTime));
            timeArray1[strlen(timeArray1) - 1] = 0;
            struct sockaddr_in peeraddr;
            socklen_t addrlen = sizeof(peeraddr);


            if(getpeername(fd, &peeraddr, &addrlen) == -1)
            {
                perror("getpeername");
            }

            //Send Client Address
            char address[INET_ADDRSTRLEN];
            strcpy(address,inet_ntoa(peeraddr.sin_addr));
            strcat(address,"\n");
            Writen(fd,address,strlen(address));


            printf("%s from Client %s has been successfully transmitted to Client %s on %s\n",file,clientAddress,inet_ntoa(peeraddr.sin_addr),timeArray1);
            bzero(receiveDownload,sizeof(receiveDownload));
            resetTimer();
        }
        else
        {
            Writen(fd,"Incorrect Input\n",16);
            resetTimer();
        }
    }
}

