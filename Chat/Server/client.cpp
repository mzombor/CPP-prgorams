#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 707
#define MAXDATASIZE 1024

////////////////////////////////////////////
////// Cient main file, start client
////////////////////////////////////////////

/// Main function
/// Starts client
int main(int argc, char *argv[])
{

    int sockfd;
    char buf[MAXDATASIZE] = "11::jaja";
    struct hostent *he;
		struct sockaddr_in their_addr; /// Servers address

    if (argc != 2) {
			perror("socket");
        fprintf(stderr,"usage: client hostname\n");
        exit(1); }

    if ((he=gethostbyname(argv[1])) == NULL) {
        perror("gethostbyname");
        exit(1); }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        exit(1); }

    their_addr.sin_family = AF_INET; /// host byte order
    their_addr.sin_port = htons(PORT); // short network byte order
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8); /// Zeroing the struct


    if (connect(sockfd, (struct sockaddr *)&their_addr,sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1); }

		if (send(sockfd, buf, sizeof(buf)-1, 0) == -1)
		{
			exit(1);
		}

    close(sockfd);

    return 0;

}
