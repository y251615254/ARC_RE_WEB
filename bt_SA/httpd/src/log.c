/*
 * http_log.c: Dealing with the logs and errors
 *
 * Rob McCool
 *
 */

/*
 * log.c: logs error message and transaction in memory buffer.
 *
 * C.H. Lin @ Technology Service Division of Institute for Information
 *		Industry
 */
#include "httpd.h"

char *get_wkday(int);
#define PROC_LOG

void log_error(char *err) 
{
	printf("[%lu] %s\n", (unsigned long)pthread_self(), err);
#ifdef SYSLOG_ENHANCED
	SetSystemLogFmt(LOG_TYPE_APP, LOG_LEVEL_TYPE_ERROR, LOG_APP_MESSAGE_TYPE_http, "HTTP-[%lu] %s", (unsigned long)pthread_self(), err);
#endif
}

void error_head(struct request_rec *r, char *err) 
{
	if (!r->assbackwards) {
		r->content_type = "text/html";
		send_http_header(r);
	}

	if (!r->header_only) {
		so_printf(r, "<HEAD><TITLE>%s</TITLE></HEAD>%c", err, LF);
		so_printf(r, "<BODY><H1>%s</H1>%c", err, LF);
	}
}

void title_html(struct request_rec *r, char *msg) 
{
	so_printf(r,"<HEAD><TITLE>%s</TITLE></HEAD>%c",msg,LF);
	so_printf(r,"<BODY><H1>%s</H1>%c",msg,LF);
}

void answer(struct request_rec *req, int type, char *err_string)
{
	struct request_rec *r = req;

	if (!r)
		return;

	/* The following takes care of Apache redirects to custom response URLs */

	ht_dbg("[%d]: url - %s, type - %d, error msg - %s\n", r->index, r->url, type, err_string);
    switch(type) 
	{
		case DOCUMENT_FOLLOWS:
			r->status = DOCUMENT_FOLLOWS;
			r->content_type = "text/html";
			/*
			 * Alpha Liu 2012.11.27 issue for PR711AAW
			 * Issue description: httpd send an pure 200 OK response
			 *										with content length is not 0
			 * Solution: Reset content length
			*/
			r->content_length = 0; //reset the content_length
			/* end Alpha Liu 2012.11.27 issue for PR711AAW */

			send_http_header(r);

			r->header_only = 1;

			break;

		case REDIRECT:
			r->status = REDIRECT;

			if (!r->assbackwards) {
				r->content_type = "text/plain";
				snprintf(r->location, sizeof(r->location)-1, "%s", err_string);
				r->location[sizeof(r->location)-1] = '\0';

				send_http_header(r);
			}
			if (r->header_only) break;

			title_html(r,"302 Document moved");
			so_printf(r,"This document has moved <A HREF=\"%s\">here</A>.<P>%c", err_string, LF);

			break;

		case USE_LOCAL_COPY:

			r->status = USE_LOCAL_COPY;
			r->content_type = "text/html";
			send_http_header(r);

			r->header_only = 1;

			break;

		case AUTH_REQUIRED:
			r->status = AUTH_REQUIRED;

			if (!r->assbackwards) {
				r->content_type = "text/html";

				r->oheader_len = strlen(err_string)+100;
				r->oheader = malloc(r->oheader_len);
				if (r->oheader == NULL)
					return;

				snprintf(r->oheader, r->oheader_len, "WWW-Authenticate: %s\r\n", err_string); 
				send_http_header(r);

				free(r->oheader);
				r->oheader = NULL;
			}

			if (r->header_only) break;

			title_html(r, "401 Authorization Required");
			so_printf(r, "Browser not authentication-capable or %c", LF);
			so_printf(r, "authentication failed.%c", LF);
			break;

		case BAD_REQUEST:
			r->status = BAD_REQUEST;
			error_head(r, "400 Bad Request");
			if (r->header_only) break;

			so_printf(r, "Your client sent a query that this server could not%c", LF);
			so_printf(r, "understand.<P>%c", LF);
			so_printf(r, "Reason: %s<P>%c", err_string,LF);
			break;

		case FORBIDDEN:
		case (FORBIDDEN+0x41000):
		case (FORBIDDEN+0x41001):
		case (FORBIDDEN+0x41002):
		case (FORBIDDEN+0x41003):
		case (FORBIDDEN+0x42000):
			r->status = FORBIDDEN;
			error_head(r, "403 Forbidden");
			if (r->header_only) break;

			so_printf(r,"Your client does not have permission to get this page.");
			if (type != FORBIDDEN)
				so_printf(r, "(code=%x) ", type);

			so_printf(r,"from this server.<P>%c",LF);
			break;

		case NOT_FOUND:
			r->status = NOT_FOUND;
			error_head(r, "404 Not Found");
			if (r->header_only) break;

			so_printf(r, "The requested page was not found on this server.<P>%c", LF);
			break;

		case SERVER_ERROR:
			r->status = SERVER_ERROR;
			error_head(r, "500 Server Error");
//			log_error(err_string);
			if(r->header_only)
				break;

			so_printf(r, "The server encountered an internal error or%c", LF);
			so_printf(r, "misconfiguration and was unable to complete your%c", LF);
			so_printf(r, "request.<P>%c", LF);
			so_printf(r, "Please contact the server administrator, %c", LF);
			so_printf(r, " %s ", server_admin());
			so_printf(r, "and inform them of the time the error occurred, and%c", LF);
			so_printf(r, "anything you might have done that may have caused%c", LF);
			so_printf(r, "the error.<P>%c", LF);
			break;

		case NOT_IMPLEMENTED:
			r->status = NOT_IMPLEMENTED;
			error_head(r, "501 Not Implemented");
			if (r->header_only) break;

			so_printf(r, "We are sorry to be unable to perform the method %s", err_string);
			so_printf(r, " at this time.<P>%c",LF);
			so_printf(r, "If you would like to see this capability in future%c", LF);
			so_printf(r, "releases, send the method which failed, why you%c", LF);
			so_printf(r, "would like to have it, and the server version %s%c", SERVER_VERSION,LF);
			so_printf(r, "to <ADDRESS>%s</ADDRESS><P>%c",SERVER_SUPPORT,LF);
			break;

		case NO_MEMORY:
			log_error("httpd: memory exhausted");
			r->status = SERVER_ERROR;
			error_head(r, "500 Server Error");
			if (r->header_only) break;

			so_printf(r, "The server has temporarily run out of resources for%c", LF);
			so_printf(r, "your request. Please try again at a later time.<P>%c", LF);
			break;
	}

	if (!r->header_only)
		so_printf(r, "</BODY>%c", LF);

}
