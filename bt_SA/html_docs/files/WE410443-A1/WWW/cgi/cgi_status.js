
/*REAL
<% ABS_MAP("wireless_enable","ARC_WLAN_24G_Enable") %>
<% ABS_MAP("model_name","ARC_SYS_ModelName") %>
<% ABS_MAP("languages","ARC_SYS_Language") %>
<% ABS_MAP("lan_gateway_ip","ARC_LAN_x_IP4_Addr"+0) %>
<% ABS_MAP("lan_gateway_mask","ARC_LAN_x_IP4_Netmask"+0) %>

var lan_mac_addr_list = [['<% ABS_GET("ARC_LAN_x_MACaddr+0") %>'],null];
var wlan_mac_addr_list = [['<% ABS_GET("ARC_WLAN_MAC") %>'],null];

var boot_code_version="<% ABS_GET("ARC_SYS_BootVersion") %>";
var runtime_code_version="<% get_firmware_version(); %>";
var runtime_date="<% ABS_GET("ARC_SYS_FWDate") %>";
var hardware_model="<% ABS_GET("ARC_SYS_ModelName") %>";
var hardware_version="<% ABS_GET("ARC_SYS_HWVersion") %>";
var hardware_serial_no="<% ABS_GET("ARC_SYS_SerialNum") %>";
var hardware_Manufacturer="<% ABS_GET("ARC_SYS_ManufacturerName") %>";
var initLedValueSlider="<% ABS_GET("ARC_SYS_LED") %>";
var system_uptime='<% get_system_uptime(); %>';
<% ABS_MAP("led_glbcfg","ARC_SYS_LED") %>
REAL*/

