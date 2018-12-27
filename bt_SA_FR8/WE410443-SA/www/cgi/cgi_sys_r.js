/*DEMO*/
addCfg("rm_en",403,1);
// format is [ip]
addCfg("rm_start_ip",404,"0.0.0.0");
addCfg("rm_end_ip",405,"0.0.0.1");
addCfg("rm_list_en",600500,"0");  //1/0: enable/disable management control list
/*END_DEMO*/
/*REAL
<% ABS_MAP("rm_en", "ARC_UI_WEB_REMOTEMN_Enable") %>
<% ABS_MAP("rm_ip", "ARC_UI_WEB_REMOTEMN_ExposeToIP") %>
<% ABS_MAP("rm_anyip", "ARC_UI_WEB_REMOTEMN_ExposeToAny") %>
<% ABS_MAP("rm_port", "ARC_UI_WEB_REMOTEMN_Port") %>
<% ABS_MAP("rm_list_en", "ARC_UI_WEB_REMOTEMN_CtrlList") %>
<% ABS_MAP("rm_http_en", "ARC_UI_WEB_REMOTEMN_Http_Enable") %>
<% ABS_MAP("rm_telnet_en", "ARC_UI_WEB_REMOTEMN_Telnet_Enable") %>
<% ABS_MAP("rm_ftpd_en", "ARC_UI_WEB_REMOTEMN_Ftp_Enable") %>
<% ABS_MAP("ftpserver_en", "ARC_USB_FTP_Enable") %>
REAL*/
