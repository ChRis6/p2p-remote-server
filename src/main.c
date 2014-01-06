#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "group_list.h"
#include "server_protocol.h"

#define MYPORT "5000"    


/* PROTOTYPES */
int handle_request( char* msg , struct sockaddr_in *addr , socklen_t addr_len , GROUP_LIST* groups);


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{

    
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];



    GROUP_LIST* groups = NULL;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }
    freeaddrinfo(servinfo);


    /* Init server */

    groups = create_group_list();
    if( !groups ){
        fprintf(stderr, "OUT OF MEMORY\n");
        close(sockfd);
        return -1;
    }

    addr_len = sizeof their_addr;
    fprintf(stderr, "SERVER:Beginning Service...\n" );
    /* MAIN LOOP */
    while (1){


        /* Receive message */
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
        
        }
        /* Handle Request */
        if( !handle_request(buf , (struct sockaddr_in *)&their_addr, addr_len , groups) )
            continue;



        /* Reply */
    }


/*
    printf("listener: got packet from %s\n",
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s));
*/

    printf("listener: packet is %d bytes long\n", numbytes);
    buf[numbytes] = '\0';
    printf("listener: packet contains \"%s\"\n", buf);

    close(sockfd);

    return 0;
}





/**
 * Handle request and set reply in msg
 *
 * Return: 1 on success
 *         0 on failure
 */
int handle_request( char* msg , struct sockaddr_in *addr , socklen_t addr_len , GROUP_LIST* groups){

    unsigned short msg_len;
    char msg_type;
    

    /* Read size and message type */
    memcpy(&msg_len , msg ,MSGSIZELEN);
    msg_len = ntohs(msg_len);
    msg_type = msg[2];          

    if( SEARCH_REQUEST == msg_type ){
        /* search request */
        

    }
    else if( ADD_REQUEST == msg_type ){
        /* add the user to a group list */


    }
    else{
        /* Protocol doesn't define a diffrent type of message */
        return 0;
    }


    return 1;
}