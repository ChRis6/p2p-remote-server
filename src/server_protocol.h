#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

enum{
	SEARCH_REQUEST = 0,
	SEARCH_REPLY,
	ADD_REQUEST,
	ADD_REPLY_POSITIVE,
	ADD_REPLY_NEGATIVE
};


#define MAXBUFLEN 1200                  // Max message size
#define MSGSIZELEN 2					// size of packet length header in bytes

#endif