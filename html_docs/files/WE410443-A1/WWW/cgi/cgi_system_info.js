/*DEMO*/

addCfg("wireless_enable", "1966180000", "1");
addCfg("wireless_enable_5g", "1966180001", "1");

var system_now_time="08/01/2003 00:01:42 am";
addCfg("dhcp_client_num",123,"1");

var lan_mac_addr_list = [['00-25-2C-A1-FE-2B'],null];
var wan_mac_addr_list=[['88-25-2C-A1-FE-2B'],['88-25-2C-A1-FE-2C'],['88-25-2C-A1-FE-2D'],null];
var wlan_mac_addr_list = [['00-25-2C-A1-FE-2B'],null];

var runtime_code_version="v1.00.00";
var boot_code_version="0.06";
var adsl_code_version="3.4.4.10.0.1A";

var hardware_version="01";
var hardware_serial_no="J042034491";
/*END_DEMO*/
/*REAL
<% ABS_MAP("wireless_enable","ARC_WLAN_24G_Enable") %>
<% ABS_MAP("wireless_enable_5g","ARC_WLAN_5G_Enable") %>

system_now_time="<% localtime(); %>";
addCfg("dhcp_client_num",123,"1");

var lan_mac_addr_list = [['<% ABS_GET("ARC_LAN_x_MACaddr+0") %>'],null];
var wan_mac_addr_list=[['<% ABS_GET("ARC_WAN_MACBase") %>'],null];
var wlan_mac_addr_list = [['<% ABS_GET("ARC_WLAN_MAC") %>'],null];

var runtime_code_version="<% get_firmware_version(); %>";
var adsl_code_version="0.0";

var boot_code_version="<% ABS_GET("ARC_SYS_BootVersion") %>";
var hardware_version="<% ABS_GET("ARC_SYS_HWVersion") %>";
var hardware_serial_no="<% ABS_GET("ARC_SYS_SerialNum") %>";
REAL*/
