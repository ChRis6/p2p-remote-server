#ifndef GROUP_LIST_H
#define GROUP_LIST_H

#include <arpa/inet.h>


/* Member object */
typedef struct member {
	
	/* IP address */
	char ip[INET_ADDRSTRLEN];
	/* port number in host byte order */
	short port;

	struct member* next;
}MEMBER_T;

/* List of members */
typedef struct member_list{
	/* pointer to first member */
	MEMBER_T* head;
	/* number of members on the list */
	unsigned int n;
}MEMBER_LIST;


/* p2p group object */
typedef struct group{
	/* p2p group name */
	char* group_name;
	/* list of members in the group */
	MEMBER_LIST* members;

	struct group* next;

}GROUP_T;


typedef struct group_list{
	/* pointer to first group */
	GROUP_T* head;
	/* number of groups */
	unsigned int n;
}GROUP_LIST;


/* ============ MEMBER MANAGEMENT ==============*/

/**
 * Makes an empty list of members
 *
 * Returns: pointer to an empty list
 *			NULL when out of memory
 */
MEMBER_LIST* create_member_list();


/**
 * Makes a member with a specified ip address and port number
 * 
 * Returns : 1) A pointer to a newly created member
 *					or
 *			 2) Null when out of memory
 */
MEMBER_T* create_member( char* ip , short port);

/**
 * Inserts a member to the members list
 * 
 * Returns: 1 when successful
 *			2 when failed
 */

int add_member( MEMBER_LIST* list , MEMBER_T* member);


/**
 * Print members on screen.Can be easily modified to
 * print on files
 */
void print_member_list(MEMBER_LIST* list);




/*======== GROUP MANAGEMENT ===============*/


/**
 * Makes an empty list of groups
 *
 * Returns: pointer to an empty list
 *			NULL when out of memory
 */
GROUP_LIST* create_group_list();


/**
 * Makes a group with a specified name and list of members
 * 
 * Returns : 1) A pointer to a newly created group
 *					or
 *			 2) Null when out of memory
 */
GROUP_T* create_group(char* name , MEMBER_LIST* member_list);


/**
 * Adds a group to the group list
 * 
 * Returns: 1 when successful
 *			2 when failed
 */
int add_group( GROUP_LIST* list , GROUP_T* group);

/**
 * Function that searches groups by name and returns a list of members
 *
 * Returns: pointer to list of the requested group
 *			NULL when no such group exists
 */
MEMBER_LIST* get_members_from_group( GROUP_LIST* group_list , char* g_name);
#endif