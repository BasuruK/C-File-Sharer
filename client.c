/**
 * Basuru Kusal
 * Server.c => acts as a Client
 */

#include "stuff.h"

void read_command(FILE *fp,int sockfd);

int main(int argc, char **argv)
{
    int sockfd;
    pid_t childpid;
    struct sockaddr_in servaddr;

    if(argc != 2)
    {
        printf("Program Name <IP ADDRESS>");
        exit(0);
    }

    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        printf("\nError: Could not create socket \n");
        return 1;
    }

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_TCP_PORT);
    if(inet_pton(AF_INET, argv[1],&servaddr.sin_addr) <= 0)
    {
        printf("\n Error Intet_pton Error\n");
    }
    if(connect(sockfd,(SA *) &servaddr,sizeof(servaddr)) < 0)
    {
        if(errno == EINTR)
        {
            perror("Connect Interrupted \n");
            close(sockfd);
            exit(1);
        } else
        {
            perror("Connect Error");
            close(sockfd);
            exit(1);
        }
    }


    //Client functions
    read_command(stdin,sockfd);

    exit(0);

}

/**
 * Read a user command and send it to the server.
 */
void read_command(FILE *fp,int sockfd)
{

    char user_command[20], recvline[MAXLINE];
    int maxfdp1, stdineof;
    fd_set rset;
    start:
    FD_ZERO(&rset);
    stdineof = 0;

    char command[MAXLINE];
    char file[MAXLINE];

    printf("%sCLIENT> %s",KGRN,KWHT);
    for(;;)
    {
        fputs("\n",stdout);

        FD_SET(sockfd, &rset);
        if(stdineof == 0)
            FD_SET(fileno(fp),&rset);
        maxfdp1 = (int) fmax(fileno(stdin), sockfd) + 1;

        Select(maxfdp1,&rset,NULL,NULL,NULL);

        if(FD_ISSET(sockfd,&rset)) {
            if (Readline(sockfd, recvline, MAXLINE) == 0) {
                if(stdineof == 1)
                    return;

                perror("Server terminated Prematurely");
                close(sockfd);
                exit(1);
            }

            //Handle Download Request
            sscanf(recvline,"%s %s",command,file);
            if(strcmp(command,"Request") == 0)
            {
                char fileBuffer[MAXLINE];
                char directory[100] = "u_file/";
                strcat(directory,file);
                //Read the file to be sent
                FILE * filePointer = fopen(directory,"r");
                fgets(fileBuffer,255,filePointer);
                fclose(filePointer);

                Writen(sockfd,fileBuffer,strlen(fileBuffer));
                bzero(command,sizeof(command));
                bzero(fileBuffer,sizeof(fileBuffer));
                bzero(directory,sizeof(directory));

                //Read the immediate reply.
                Readline(sockfd,fileBuffer,MAXLINE);
                char receiveClientAddress[30];
                Readline(sockfd,receiveClientAddress,MAXLINE);



                strcpy(directory,"d_files/");
                strcat(directory,file);
                FILE * savePointer = fopen(directory,"w+");
                fprintf(savePointer,fileBuffer);
                fclose(savePointer);

                time_t myTime = time(NULL);

                printf("%s has been successfully downloaded from %s on %s",file,receiveClientAddress,ctime(&myTime));
                bzero(file,sizeof(file));
            }
            else
            {
                printf("%s", KYLW);
                fputs(recvline, stdout);
                printf("%s", KWHT);
            }


            //close the connection if a timeout occurs.
            if(strcmp(recvline,"Good Bye\n") == 0)
            {
                close(sockfd);
                FD_CLR(sockfd,&rset);
                FD_ZERO(&rset);
                exit(0);
            }

            printf("%sCLIENT> %s", KGRN, KWHT);
        }

        if(FD_ISSET(fileno(fp),&rset))
        {
          if(Fgets(user_command, MAXLINE, fp) == NULL) {
              stdineof = 1;
              Shutdown(sockfd,SHUT_WR);
              FD_CLR(fileno(fp),&rset);
              continue;
          }
            if (strcmp(user_command, "quit\n") == 0) {
                printf("%sCLIENT TERMINATED%s\n", KGRN, KWHT);
                close(sockfd);
                exit(0);
            } else if (strcmp(user_command, "\n") == 0) {
                memset(user_command, 0, sizeof(user_command));
                goto start;
            } else
            {
                Writen(sockfd, user_command, strlen(user_command));
            }
        }

        bzero(recvline, sizeof(recvline));
        bzero(user_command, sizeof(user_command));

    }

}

