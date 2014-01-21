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
static void handle_search_request(GROUP_LIST* group,char* buffer , char* group_name , int name_len);
static void handle_add_request( GROUP_LIST* groups, char* msg, char* ip , unsigned short port_number);


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
        sendto(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr*)&their_addr, addr_len );
    }

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
        handle_search_request( groups, msg , msg + sizeof(unsigned short) + sizeof(char) , msg_len);        

    }
    else if( ADD_REQUEST == msg_type ){
        /* add the user to a group list */
        char ip[INET_ADDRSTRLEN];
        unsigned short port_number;


        // get port number
        port_number = ntohs( addr->sin_port);
        // get ip address
        if( inet_ntop(addr->sin_family,&(addr->sin_addr),ip, sizeof(ip) ) == NULL)
            return 0;   // don't handle bad ip requests

        handle_add_request( groups, msg, ip , port_number);


    }
    else{
        /* Protocol doesn't define a diffrent type of message */
        return 0;
    }


    return 1;
}


static void handle_search_request(GROUP_LIST* groups , char* buffer , char* group_name , int name_len){

    MEMBER_LIST* members = NULL;

    members = get_members_from_group( groups , group_name);
    if(!members || members->n < 1){
        // no members or group not found
        buffer[0] = 0;
        buffer[1] = 0;
        buffer[2] = SEARCH_REPLY;
    }
    else {
        // members found
        int i;
        int howmany_pairs;
        //int pair_offset;
        unsigned short port_num;

        unsigned short numpairs    = MAXBUFLEN / ( sizeof(unsigned short) + INET_ADDRSTRLEN ); 
        unsigned short pairs_avail = members->n;
        if( numpairs > pairs_avail ){
            howmany_pairs = pairs_avail;
            pairs_avail = htons(pairs_avail);
            memcpy(buffer, &pairs_avail, sizeof(unsigned short));
        }
        else{
            howmany_pairs = numpairs;
            numpairs = htons(numpairs);
            memcpy(buffer, &numpairs, sizeof(unsigned short));
        }
        buffer[2] = SEARCH_REPLY; 

        buffer += 3;     // skip the packet header
        MEMBER_T* curr = members->head;
        //pair_offset = INET_ADDRSTRLEN + sizeof( unsigned short);
        for( i = 0; i < howmany_pairs; i++){
            port_num = ntohs(curr->port);

            memcpy(buffer, curr->ip , INET_ADDRSTRLEN);
            buffer += INET_ADDRSTRLEN;
            
            memcpy(buffer, &port_num , sizeof(unsigned short));
            buffer += sizeof(unsigned short); 
            // alternative
            //memcpy( buffer + (i*pair_offset), curr->ip , INET_ADDRSTRLEN);
            //memcpy( buffer + (i*pair_offset + INET_ADDRSTRLEN), &port_num , sizeof(unsigned short)); 
        }
    }
}

static void handle_add_request( GROUP_LIST* groups, char* msg, char* ip , unsigned short port_number){

    unsigned short len;
    int packet_header_offset;
    char* group_name;
    GROUP_T *group;
    GROUP_T* new_group;
    MEMBER_T* new_member;

    packet_header_offset = sizeof(unsigned short) + sizeof(char);

    memcpy(&len, msg, sizeof(unsigned short));
    len = ntohs(len);
    
    group_name = (char*) malloc( sizeof(char) * (len + 1));     // len + 1 bytes including the '\0' 
    if(!group_name){
        /*
         *  Server is out of memory.Send a negative reply.We only need to change
         *  the \type field of the packet's header
         */
        msg[2] = ADD_REPLY_NEGATIVE;
        return ;
    }

    // copy group name from the packet
    strncpy( group_name, msg + packet_header_offset, len );
    group_name[len] = '\0';

    new_member = create_member(ip , port_number);
    if( !new_member){
        // not enough memory
        free(group_name);
        msg[2] = ADD_REPLY_NEGATIVE;
        return;
    }

    group = get_group_by_name(groups, group_name);
    if(!group){

        /*
         * No such group.First create a new members list and add the new member to the 
         * list.Then create a new group and add it to the groups' list
         */
        int flag;

        MEMBER_LIST* member_list = create_member_list();
        if( !member_list){
            free(group_name);
            free(new_member);

            msg[2] = ADD_REPLY_NEGATIVE;
            return ;
        }
        // add member to the list
        flag = add_member(member_list,new_member);
        if( !flag){  
            // not added
            free(member_list);
            free(new_member);
            free(group_name);
            msg[2] = ADD_REPLY_NEGATIVE;
            return ;
        }
        // now create a new group and add it to the groups list
        new_group = create_group( group_name, member_list);
        if( !new_group){

            free(new_member);
            free(member_list);
            free(group_name);

            msg[2] = ADD_REPLY_NEGATIVE;
            return;
        }

        // add it to the groups' list
        add_group(groups, new_group);
        msg[2] = ADD_REPLY_POSITIVE;
        return;
    }
    else{
        // group is found.Add the member to the group
        
        add_member( group->members, new_member);
        // positive reply
        msg[2] = ADD_REPLY_POSITIVE;
    }

    free(group_name);
    return ;
}