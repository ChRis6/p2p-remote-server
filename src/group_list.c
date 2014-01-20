#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "group_list.h"

/**
 * Makes an empty list of members
 *
 * Returns: pointer to an empty list
 *			NULL when out of memory
 */
MEMBER_LIST* create_member_list(){

	MEMBER_LIST* list;

	list = (MEMBER_LIST*) malloc( sizeof(MEMBER_LIST));
	if( !list )
		return NULL;

	list->head = NULL;
	list->n = 0;

	return list;
}


/**
 * Make a member with a specified ip address and port number
 * 
 * Returns : 1) A pointer to a newly created member
 *					or
 *			 2) Null when out of memory
 */
MEMBER_T* create_member( char* ip , short port){

	MEMBER_T* new_member;

	if( !ip  ||  strlen(ip) != INET_ADDRSTRLEN )
		return NULL;

	new_member = ( MEMBER_T*) malloc( sizeof(MEMBER_T));
	if( !new_member)
		return NULL;

	strcpy(new_member->ip , ip);
	new_member->port = port;
	new_member->next = NULL;

	return new_member;
}

/**
 * Inserts a member to the members list
 * 
 * Returns: 1 when successful
 *			2 when failed
 */

int add_member( MEMBER_LIST* list , MEMBER_T* member){

	if( !list || !member)
		return 0;

	if( list->n == 0 ){
		list->head = member;
		list->n = 1;
	}
	else{
		member->next = list->head;
		list->head = member;
		list->n = list->n + 1 ;
	}

	return 1;
}

/**
 * Print members on screen.Can be easily modified to
 * print on files
 */
void print_member_list(MEMBER_LIST* list){


}



/**
 * Makes an empty list of groups
 *
 * Returns: pointer to an empty list
 *			NULL when out of memory
 */
GROUP_LIST* create_group_list(){

	GROUP_LIST* list;

	list = (GROUP_LIST*) malloc( sizeof(GROUP_LIST));
	if( !list)
		return NULL;

	list->head = NULL;
	list->n = 0; 

	return list;

}


/**
 * Makes a group with a specified name and list of members
 * 
 * Returns : 1) A pointer to a newly created group
 *					or
 *			 2) Null when out of memory
 */
GROUP_T* create_group(char* name , MEMBER_LIST* member_list){

	GROUP_T* group = NULL;
	if( !name || member_list)
		return NULL;

	group = ( GROUP_T*) malloc( sizeof(GROUP_T));
	if( !group)
		return NULL;

	group->group_name = strdup(name);
	group->members = member_list;
	group->next = NULL;

	return group;
}

/**
 * Adds a group to the group list
 * 
 * Returns: 1 when successful
 *			2 when failed
 */
int add_group( GROUP_LIST* list , GROUP_T* group){

	if( !list || !group )
		return 0;

	if( list->n == 0 ){
		list->head = group;
		list->n = 1;
	}
	else {
		group->next = list->head;
		list->head = group;
		list->n = list->n + 1;

	}
	return 1;
}

/**
 * Function that searches groups by name and returns a list of members
 *
 * Returns: pointer to list of the requested group
 *			NULL when no such group exists
 */
MEMBER_LIST* get_members_from_group( GROUP_LIST* group_list , char* g_name){

	GROUP_T* curr;	

	if( !g_name || !group_list || group_list->n < 1 )
		return NULL;	

	for ( curr = group_list->head ; curr ; curr = curr->next)
		if( !strcmp( g_name , curr->group_name) )
			return curr->members;
	
	return NULL;
}