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
 * http_alias.c: Stuff for dealing with directory aliases
 * 
 * Rob McCool
 *
 */
#include "httpd.h"

extern alias_t *aliases;

/* Try to find an unused alias */
int get_free_alias(void)
{
	int i;

	for (i=0; i<MAX_ALIAS; i++) {
		if (aliases[i].fake[0] == '\0' && 
			aliases[i].real[0] == '\0')
			return i;
	}

	return -1;
}


/*
 * Can return
 *   BAD_URL         Malformed URL
 *   REDIRECT_URL    URL matched a Redirect.
 *   SCRIPT_CGI      URL matches a ScriptAlias path
 *   SCRIPT_NCSA     URL matches an OldScriptAlias path
 *   STD_DOCUMENT    any other URL
 */
int translate_name(char *name)
{
	int i;
	register int l;

	getparents(name);
	if (name[0] != '/' && name[0] != '\0')
		return BAD_URL;

	for (i=0; i<MAX_ALIAS && is_null_string(aliases[i].fake)==0; i++) {

		l=strlen(aliases[i].fake);

		/* ht_dbg("name=[%s], aliases[%d].fake=[%s]\n", name, i, aliases[i].fake); */
		if (!strncmp(name, aliases[i].fake, l) &&
			(aliases[i].fake[l-1] == '/'|| l == strlen(name) || name[l] == '/')) {

			strsubfirst(l, name, aliases[i].real);
			return aliases[i].script;
		}
	}

	/* no alias, add document root */
	strsubfirst(0, name, document_root);

	return STD_DOCUMENT;
}

void unmunge_name(char *name) 
{
	register int l;
	int i;

	l=strlen(document_root);

	if(!strncmp(name,document_root,l)) {
		strsubfirst(l,name,"");
		if(!name[0]) {
			name[0] = '/';
			name[1] = '\0';
		}

		return;
	}

	for (i=0; aliases[i].fake != NULL; i++)
	{
		l = strlen(aliases[i].real);
		if(!strncmp(name, aliases[i].real, l)) {
			strsubfirst(l, name, aliases[i].fake);
			if(!name[0]) {
				name[0] = '/';
				name[1] = '\0';
			}

			return;
		}
	}
}
