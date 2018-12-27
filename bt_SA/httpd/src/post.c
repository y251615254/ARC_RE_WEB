
/*-
 * Copyright (c) 1995 The Apache Group. All rights reserved.
 *
 *
 * Apache httpd license
 * ====================
 *
 *
 * This is the license for the Apache Server. It covers all the
 * files which come in this distribution, and should never be removed.
 *
 * The "Apache Group" has based this server, called "Apache", on
 * public domain code distributed under the name "NCSA httpd 1.3".
 *
 * NCSA httpd 1.3 was placed in the public domain by the National Center
 * for Supercomputing Applications at the University of Illinois
 * at Urbana-Champaign.
 *
 * As requested by NCSA we acknowledge,
 *
 *  "Portions developed at the National Center for Supercomputing
 *   Applications at the University of Illinois at Urbana-Champaign."
 *
 * Copyright on the sections of code added by the "Apache Group" belong
 * to the "Apache Group" and/or the original authors. The "Apache Group" and
 * authors hereby grant permission for their code, along with the
 * public domain NCSA code, to be distributed under the "Apache" name.
 *
 * Reuse of "Apache Group" code outside of the Apache distribution should
 * be acknowledged with the following quoted text, to be included with any new
 * work;
 *
 * "Portions developed by the "Apache Group", taken with permission
 *  from the Apache Server   http://www.apache.org/apache/   "
 *
 *
 * Permission is hereby granted to anyone to redistribute Apache under
 * the "Apache" name. We do not grant permission for the resale of Apache, but
 * we do grant permission for vendors to bundle Apache free with other software,
 * or to charge a reasonable price for redistribution, provided it is made
 * clear that Apache is free. Permission is also granted for vendors to
 * sell support for for Apache. We explicitly forbid the redistribution of
 * Apache under any other name.
 *
 * The "Apache Group" makes no promises to support "Apache". Users and
 * sellers of Apache support, and users of "Apache Group" code, should be
 * aware that they use "Apache" or portions of the "Apache Group" code at
 * their own risk. While every effort has been made to ensure that "Apache"
 * is robust and safe to use, we will not accept responsibility for any
 * damage caused, or loss of data or income which results from its use.
 *
 */



/*
 * http_post.c: Handles POST
 *
 * Rob McCool
 *
 */
#include "httpd.h"

void process_post(struct request_rec *r)
{
	int s = 0;

	ht_dbg("path: %s, args: %s\n", r->url, r->args);

	s = translate_name(r->url);

#ifdef VRV9518SWAC33
	s = o2_redirect(s, r);
#endif

	switch(s) {
		case STD_DOCUMENT:
			if (find_script(r))
				return;
			else
				/* Maybe, someday, do group annotation here */
				answer(r, NOT_IMPLEMENTED,"POST to non-script");
			break;

		case REDIRECT_URL:
			answer(r, REDIRECT, r->url);
			break;

		case SCRIPT_CGI:
			exec_cgi_script(r);
			break;

#ifdef HNAP_SUPPORT
		case HNAP_CGI:
			hnap_cgi_stub(r);
			return;
#endif

		case BAD_URL:
			answer(r, BAD_REQUEST, r->url);
			break;
	}

	return ;
}


