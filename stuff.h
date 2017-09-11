#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <wait.h>
#include <signal.h>
#include <math.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>

#ifndef ASSIGNMENT_STUFF_H
#define ASSIGNMENT_STUFF_H

#endif //ASSIGNMENT_STUFF_H

#define SERV_TCP_PORT 25
#define SA struct sockaddr
#define MAXLINE 1024
#define MAXSIZE 256
struct timeval tv = {0,0};
#define KRED  "\x1B[31m"
#define KWHT  "\x1B[37m"
#define KGRN  "\x1B[32m"
#define KYLW  "\x1b[33m"

#define	LISTENQ	1024

/**
 * Write functions
 * @param fd
 * @param vptr
 * @param n
 * @return
 */
ssize_t						/* Write "n" bytes to a descriptor. */
writen(int fd, void *vptr, size_t n)
{
    size_t		nleft;
    ssize_t		nwritten;
    const char	*ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
            if (errno == EINTR)
                nwritten = 0;		/* and call write() again */
            else
                return(-1);			/* error */
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }
    return(n);
}
/* end writen */

void
Writen(int fd, void *ptr, size_t nbytes)
{
    if (writen(fd, ptr, nbytes) != nbytes)
    {
        perror("Written error\n");
        exit(1);
    }
}

//

/**
 * Read functions
 * @param fd
 * @param ptr
 * @return
 */
static ssize_t
my_read(int fd, char *ptr)
{
    static int	read_cnt = 0;
    static char	*read_ptr;
    static char	read_buf[MAXLINE];

    if (read_cnt <= 0) {
        again:
        if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
            if (errno == EINTR)
                goto again;
            return(-1);
        } else if (read_cnt == 0)
            return(0);
        read_ptr = read_buf;
    }

    read_cnt--;
    *ptr = *read_ptr++;
    return(1);
}

ssize_t
readline(int fd, void *vptr, size_t maxlen)
{
    int		n, rc;
    char	c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
        if ( (rc = my_read(fd, &c)) == 1) {
            *ptr++ = c;
            if(c == ',')
                *ptr++ = '\n';
            else if (c == '\n')
                break;	/* newline is stored, like fgets() */
        } else if (rc == 0) {
            if (n == 1)
                return(0);	/* EOF, no data read */
            else
                break;		/* EOF, some data was read */
        } else
            return(-1);		/* error, errno set by read() */
    }

    *ptr = 0;	/* null terminate like fgets() */
    return(n);
}
/* end readline */

ssize_t
Readline(int fd, void *ptr, size_t maxlen)
{
    ssize_t		n;

    if ( (n = readline(fd, ptr, maxlen)) < 0)
        perror("readline error");
    return(n);
}


int
Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
       struct timeval *timeout)
{
    int		n;

    if ( (n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0)
        perror("Select failed");
    return(n);		/* can return 0 on timeout */
}

char *
Fgets(char *ptr, int n, FILE *stream)
{
    char	*rptr;

    if ( (rptr = fgets(ptr, n, stream)) == NULL && ferror(stream))
        perror("fgets error");

    return (rptr);
}

void
Shutdown(int fd, int how)
{
    if (shutdown(fd, how) < 0) {
        perror("shutdown error");
        exit(1);
    }
}
