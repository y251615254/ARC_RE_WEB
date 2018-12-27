/*DEMO*/
var system_now_time="08/01/2003 00:01:42 am";

var runtime_code_version='1.00.25 (Apr 20 2010 13:48:18)'; //'1.00.10 (Dec 22 2009 17:59:32)';

var boot_code_version='V1.02';

var hardware_version='0A';
var hardware_serial_no='120942H1100006';
var hardware_model="MGL7016AW-22-A1";

var cgi_ntp_conn =parseInt('1');
/*END_DEMO*/
/*REAL
var system_now_time="<% localtime(); %>";
var runtime_code_version="<% get_firmware_version(); %> <% compile_date(); %>";
var runtime_version="<% get_firmware_version(); %>";
var hardware_serial_no="<%ABS_GET("ARC_SYS_SerialNum")%>";

var boot_code_version="<%ABS_GET("ARC_SYS_BootVersion")%>";
var hardware_version="<%ABS_GET("ARC_SYS_HWVersion")%>";
var hardware_model="<% ABS_GET("ARC_SYS_ModelName") %>";
var cgi_ntp_conn =parseInt('1');

REAL*/
