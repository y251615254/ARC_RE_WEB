#include "httpd.h"
#include "utils.h"
#include "ssi_handler.h"
#include "vendor_ssi_handler.h"

/* linghong_tan 2013-09-10 LTE ssi handlers */
#if 1
int iprintf(const char* sFmt, ...) {
	va_list vlVars;
	char sBuf[512];
	int iRet;
	FILE* fnOut;

	if (sFmt == NULL)
		return 0;

#if defined(x86_64) || defined(i386)
	fnOut = fopen( "/dev/tty", "w" );
#else
	fnOut = fopen( "/dev/console", "w" );
#endif
	if (fnOut == NULL)
		return 0;

	sBuf[sizeof(sBuf)-1] = '\0';

	va_start(vlVars,sFmt);

	iRet = vsnprintf(sBuf,sizeof(sBuf),sFmt,vlVars);

	va_end(vlVars);

	if (sBuf[sizeof(sBuf)-1] != '\0') {
		fprintf( fnOut, "NOTE: my_printf() overflow!!!\n");
	}

	sBuf[sizeof(sBuf)-1] = '\0';

	fprintf( fnOut, "%s", sBuf );
	fflush( fnOut );
	fclose( fnOut );

	return iRet;
}
#endif

int ssi_get_lte_status(struct request_rec *r, int argc, char **argv)
{
	struct aunetctl_generic_s gr_status;
	char *mode;
	int ifno = T_COM1_INT;

	if (fmt_args(argc, argv, "%s", &mode) < 1) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

	if (!strcmp(mode, "UMTS"))
		ifno = T_COM3_INT;
	else
		ifno = T_COM1_INT;

	memset(&gr_status, 0, sizeof(struct aunetctl_generic_s));

	if (mapi_aunet_get_status_generic(ifno, &gr_status) < 0)
		/* something bad happened */
		return 0;

	r->bytes_sent = so_printf(r, "%s", gr_status.deviceStateStr);

	return r->bytes_sent;
}

int ssi_get_lte_strength(struct request_rec *r, int argc, char **argv)
{
	struct aunetctl_rf_s rf_status;
	char *mode;
	int ifno = T_COM1_INT;

	if (fmt_args(argc, argv, "%s", &mode) < 1) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

	if (!strcmp(mode, "UMTS"))
		ifno = T_COM3_INT;
	else
		ifno = T_COM1_INT;

	memset(&rf_status, 0, sizeof(struct aunetctl_rf_s));

	if (mapi_aunet_get_status_rf(ifno, &rf_status) < 0) {
		/* something bad happened */
		//return 0;
	}

	/* make sure network is attached */
	if (rf_status.noNetworkAttached == 0) {
		/* strengthP, strengthD, strengthV in percent*/
		r->bytes_sent = so_printf(r, "%d", rf_status.strengthP/10);
	}
	else
		r->bytes_sent = so_printf(r, "%d", 0);

	return r->bytes_sent;
}

int ssi_get_lte_operator(struct request_rec *r, int argc, char **argv)
{
#if 0
	struct aunetctl_celllist_s cl_status;

	memset(&cl_status, 0, sizeof(struct aunetctl_celllist_s));

	if (mapi_aunet_get_status_celllist(T_COM1_INT, &cl_status) < 0)
		/* something bad happened */
		return 0;

	/* FIXME: we need API to get all providers from driver. Use the 1st one. */
	r->bytes_sent = so_printf(r, "%s", cl_status.info_longName[0]);
#endif

	struct aunetctl_rf_s rf_status;
	char *mode;
	int ifno = T_COM1_INT;
	int i;
	unsigned char *c;

	if (fmt_args(argc, argv, "%s", &mode) < 1) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

	if (!strcmp(mode, "UMTS"))
		ifno = T_COM3_INT;
	else
		ifno = T_COM1_INT;

	memset(&rf_status, 0, sizeof(struct aunetctl_rf_s));

	if (mapi_aunet_get_status_rf(ifno, &rf_status) < 0) {
		/* something bad happened */
	}
#ifdef CONFIG_HTTPD_HOT_FIX_SW1
	c=rf_status.providerL;

	for (;*c; c++)
	{
		//if (isprint((int) *c) && *c != '"' && *c != '&' && *c != '<' && *c != '>' && *c != '\'' && *c != '\\')
		//		r->bytes_sent = so_printf(r, "%c",*c);
		//else
				r->bytes_sent = so_printf(r, "%%%02X",(unsigned char)(*c));
	}
#else
	r->bytes_sent = so_printf(r, "%s", rf_status.providerL);
#endif
	return r->bytes_sent;
}

int ssi_get_sim_imsi(struct request_rec *r, int argc, char **argv)
{
	struct aunetctl_sim_status_s sim_status;
	char *mode;
	int ifno = T_COM1_INT;

	if (fmt_args(argc, argv, "%s", &mode) < 1) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

	if (!strcmp(mode, "UMTS"))
		ifno = T_COM3_INT;
	else
		ifno = T_COM1_INT;

	memset(&sim_status, 0, sizeof(struct aunetctl_sim_status_s));

	if (mapi_aunet_get_status_sim(ifno, &sim_status) < 0)
		/* something bad happened */
		return 0;

	/* make sure network is attached */
	if (sim_status.noModemExist == 0)
		r->bytes_sent = so_printf(r, "%s", sim_status.currentIMSI);

	return r->bytes_sent;
}

int ssi_get_sim_pin_status(struct request_rec *r, int argc, char **argv)
{
	struct aunetctl_sim_status_s sim_status;
	char *mode;
	int ifno = T_COM1_INT;

	if (fmt_args(argc, argv, "%s", &mode) < 1) {
		so_printf(r, "%s", "Insufficient args\n");
		return -1;
	}

	if (!strcmp(mode, "UMTS"))
		ifno = T_COM3_INT;
	else
		ifno = T_COM1_INT;

	memset(&sim_status, 0, sizeof(struct aunetctl_sim_status_s));

	if (mapi_aunet_get_status_sim(ifno, &sim_status) < 0)
		/* something bad happened */
		return 0;

	r->bytes_sent = so_printf(r, "%s", sim_status.PIN_StateStr);

	return r->bytes_sent;
}

int ssi_get_umts_info(struct request_rec *r, int argc, char **argv)
{
	struct aunetctl_pdp_s pdp_status;

	memset(&pdp_status, 0, sizeof(struct aunetctl_pdp_s));

	/* UMTS always use T_COM3_INT */
	mapi_aunet_get_status_pdp(T_COM3_INT, &pdp_status);

	r->bytes_sent = so_printf(r, "\"%lu\", \"%lu\", \"%lu\", \"%lu\", \"%lu\", \"%lu\", \"%lu\"",
			pdp_status.rx_qos_rate, pdp_status.tx_qos_rate,	/* max download/upload rate */
			pdp_status.rx_rate, pdp_status.tx_rate, /* current download/upload rate */
			pdp_status.rx_flow, pdp_status.tx_flow, pdp_status.ds_time); /* download/upload bytes and connection time */

	return r->bytes_sent;
}


