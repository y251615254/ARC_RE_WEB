/*
 * http_get.c: Handles things associated with GET
 *
 * http_get.c ==> get.c ( Multitask! version)
 * modified by Chun-Hong Lin
 *
 */

#include "httpd.h"
#include "httoken.h"

#include <sys/types.h>
#include <sys/stat.h>

extern void set_content_type(struct request_rec *r);
extern int FSstat(struct request_rec *r, char *path, FILE_INFO_t *finfo);
extern void	* FSopen(struct request_rec *r, char *fname);
extern int FSstat(struct request_rec *r, char *path, FILE_INFO_t *finfo);
extern int FSclose(void *fp);

/* tlhhh. routine for provide download service, eg: .txt/.cfg etc. */
void download_file(struct request_rec *r)
{
	char path[FNAME_SIZE];
	int c;
	FILE *fp;
	struct stat st;

	snprintf(path, sizeof(path), "%s", r->url);
	ht_dbg("Request to download file [%s]\n", path);

	if ((fp = fopen(path, "r")) == NULL)
	{
		return;
	}

	if( stat( path, &st ) < 0 )
	{
		if(fp != NULL)
		{
			fclose(fp);
			fp = NULL;
		}
		return;
	}

	r->status = 200;
	r->bytes_sent = 0;
	r->content_length = st.st_size;

	send_http_header(r);

	while ((c = getc (fp)) != EOF)
	{
		so_putc (c, r);
		r->bytes_sent++;
	}

	if (st.st_size > r->bytes_sent) {
		if (ferror(fp)) {
			printf("stream is error\n");
		}
		if (feof(fp)) {
			printf("stream met EOF\n");
		}
		so_flush(r);
	}

	fclose(fp);

	return; 
}

void send_file(register struct request_rec *r)
{
	set_content_type(r);

	r->content_length = -1;

	ht_dbg("%d: url=[%s], content_type=[%s], content_encoding=[%s]\n", 
			r->index, r->url, r->content_type, r->content_encoding);

	if (!strcmp(r->content_type, CGI_MAGIC_TYPE)) {
		exec_cgi_script(r);
		return;
	}

	if (!r->content_encoding[0] &&
		(!strcmp(r->content_type, INCLUDES_MAGIC_TYPE)
		 || !strcmp(r->content_type, JAVASCRIPT_TYPE)
		 || !strcmp(r->content_type, XML_TYPE)
		)) {

		r->status = 200;
		r->bytes_sent = 0;

		send_parsed_file(r);

		return;
	}

	r->fd = FSopen(r, r->url);

	if (r->fd == (FILE_t)NULL) {
		unmunge_name(r->url);
		/* we've already established that it exists */
		log_error("Url error");
		answer(r, SERVER_ERROR, r->url);
		return;
	}

	r->status = 200;
	r->bytes_sent = 0;


	if (!r->assbackwards) {

		r->content_length = r->finfo.size;

		if (set_last_modified(r) < 0)
			return;

		send_http_header(r);
	}

	if (!r->header_only) {
		send_fd(r);
	}

	if (FSclose(r->fd) != 0) {
		perror("fclose");
	}

	r->fd = (FILE_t) NULL;
}

void send_node(struct request_rec *r)
{
	char index_ptr[INDEX_NAMES_SIZE], path[INDEX_NAMES_SIZE]; 
	char *prev = index_ptr, *next = index_ptr;
	int done = 0;

	/* Do initial stat(); get PATH_INFO for CGI, SHTML(SSI docs) */
#if 0
	ret = get_path_info(r);
	if (ret == -1) 
	{
		goto not_found;
	}
#endif

	/*
 	pete_zhang 2013-12-20	ticket-1167

	description: GUI security enhancement
 	*/
#ifdef CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
#if defined(VR9517PAC22_A_PP) || defined(AR7516AOW22_A_PP)	//for PLDT project	
	if (strstr(r->url, "/api/") || (strstr(r->url, "/cgi/") && !strstr(r->url, "cgi_traffic_24h.js")) || strstr(r->url, "/tmp/"))
#else
	if (strstr(r->url, "/api/") || (strstr(r->url, "/cgi/") && !strstr(r->url, "fw_download_prg.js") && !strstr(r->url, "autofw_version.js")&& !strstr(r->url, "check_optimising.js") && !strstr(r->url, "setpwd_result.js")&& !strstr(r->url, "get_wps_status.js")  && !strstr(r->url, "fwupgrade_status.js")) || strstr(r->url, "/tmp/") )
#endif
	{
		ht_dbg("check token before getting data\n");

		if ( !url_token_pass(r, 0) ) //failed to pass token checking	
		{
			ht_dbg("failed to pass token checking\n");
			goto not_found;
		}
	}
#endif //CONFIG_HTTPD_TOKEN_CHECK_SUPPORT
	/*end pete_zhang        2013-12-20*/

	if (FSstat(r, r->url, &r->finfo) < 0)
		goto not_found;

	ht_dbg("r->args=[%s], r->url=[%s], fifo.size=[%lld]\n", r->args, r->url, r->finfo.size);

	
	if (r->finfo.is_dir) {
		char ifile[MAX_STRING_LEN];

		memset(index_ptr, 0, sizeof(index_ptr));
		memset(path, 0, sizeof(path));

		snprintf(index_ptr, sizeof(index_ptr)-1, "%s", index_names);
		index_ptr[sizeof(index_ptr)-1] = '\0';

		while (!done) {

			memset(path, 0, sizeof(path));

			if ((next=strchr(prev, ' ')) != NULL) {
				//ht_dbg("next = [%s]\n", next);

				memcpy(path, prev, (int)(next-prev));
				
				/* skip space */
				prev = next+1;
			}
			else {
				done = 1;
				strcpy(path, prev);
			}

			make_full_path(r->url, path, ifile);
			/* ht_dbg("ifile = [%s]\n", ifile); */

			if (FSstat(r, ifile, &r->finfo) == 0) {
				strcpy(r->url, ifile);

#if 0
				/* AUTH the client who is trying to access the dir */
				if (evaluate_access(r->url, 0, r))
					return;
#endif
				send_file(r);
				return;
			}
		}

	} else {
		//evaluate_access(r->url, 0, r);
		send_file(r);
		return;
	}

not_found:
	unmunge_name(r->url);
	answer(r, NOT_FOUND, r->url);
	return ;
}

void process_get(struct request_rec *r)
{
	int s = 0, clen, rlen;
	char dummy_body[8];

	// bypass some useless body in the GET message.  pert, 2008/03/24
	clen = r->content_length;

	while (clen > 0) {
		if (clen > 8) 
			rlen = 8;
		else 
			rlen = clen;

		ht_dbg("read dummy...., len to be read %d\n", clen);
#ifdef CONFIG_HTTPD_HTTPS_SUPPORT
		if (r->ishttps)
			rlen = as_read(r->ssrv, dummy_body, rlen);
		else
#endif
		rlen = recv(r->sd, dummy_body, rlen, MSG_DONTWAIT);
		//recv return -1 for error, 0 for the peer has performed an orderly shutdown
		if (rlen <= 0) {
			break;
		}
		clen -= rlen;
	}

	s = translate_name(r->url);
	
#ifdef VRV9518SWAC33
	s = o2_redirect(s, r);
#endif

	switch(s) {
		case STD_DOCUMENT:
			send_node(r);
			break;

		case REDIRECT_URL:
			answer(r, REDIRECT, r->url);
			break;

		case SCRIPT_CGI:
			exec_cgi_script(r);
			//answer(r, NOT_IMPLEMENTED, "GET to script");
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

