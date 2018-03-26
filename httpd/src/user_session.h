#ifndef __REMOTE_GUI_H__
#define __REMOTE_GUI_H__

#include "httpd.h"

#define	HT_GRANTED	0
#define	HT_TIMEUP	1
#define	HT_NEWUSER	2

enum TYPE_USER_SESSION {
	SESSION_LOCAL = 0,
	SESSION_REMOTE,
	SESSION_ALL
};

enum TYPE_USER_REQUEST {
	USER_LOCAL = 0,
	USER_REMOTE,
	SERVICE_REMOTE,
	REMOTE_ALL,
};


extern struct user_session g_user_session[SESSION_ALL];

int check_request_usertype(char* ipstr, int port);

#endif
