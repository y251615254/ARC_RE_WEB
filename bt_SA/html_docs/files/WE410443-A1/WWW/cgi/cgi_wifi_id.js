var SSID_NUM=0;
var SSID_No=0;
var Bind_NUM=0;
var Bind_No=0;
var VLAN_Group=1;
var SSID_LIST_MAX=3;
var VLANBIND_LIST_MAX=3;
var MAX_CLIENT_NUM=32;

var chan_list=['1','2','3','4','5','6','7','8','9','10','11',null];
var bond_list=['1','9','5','13',null];

var SUPPORTED_CHANNEL_LO=chan_list[0]; 
var SUPPORTED_CHANNEL_HI=0;
var SUPPORTED_LOWER_MIN_CHANNEL=bond_list[0];
var SUPPORTED_LOWER_MAX_CHANNEL=bond_list[1];
var SUPPORTED_UPPER_MIN_CHANNEL=bond_list[2];
var SUPPORTED_UPPER_MAX_CHANNEL=bond_list[3];
var chan5g_list=['36','40','44','48','52','56','60','64','100','104','108','112','116','120','124','128','132','136',null];
var SUPPORTED_CHANNEL_HI5g=0;

/*DEMO*/

addCfg("radiomode", "1966190000", "bgn");
addCfg("wchan", "1966210000", "0");
addCfg("bandwidth", "1966200000", "0");
addCfg("wps_enable", "1966260000", "1");
addCfg("name0", "1966300100", "Lantiq"); 
addCfg("name1", "1966300200", "Lantiq-1"); 
addCfg("name2", "1966300300", "Lantiq-2"); 
addCfg("name3", "1966300400", "Lantiq-3"); 

addCfg("en0", "1966280100", "1"); 
addCfg("en1", "1966280200", "0"); 
addCfg("en2", "1966280300", "0"); 
addCfg("en3", "1966280400", "0"); 

addCfg("bcast0", "1966290100", "0"); 
addCfg("bcast1", "1966290200", "0"); 
addCfg("bcast2", "1966290300", "0"); 
addCfg("bcast3", "1966290400", "0"); 

addCfg("radiomode5g", "1966420000", "an");
addCfg("wchan5g", "1966440000", "0");
addCfg("bandwidth5g", "1966430000", "2");
addCfg("wps_enable5g", "1966490000", "");
addCfg("name5g", "1966530100", "Lantiq_5G");
addCfg("en5g", "1966510100", "1");
addCfg("bcast5g", "1966520100", "0");
/*END_DEMO*/
/*REAL
<% ABS_MAP("radiomode","ARC_WLAN_24G_RadioMode") %>
<% ABS_MAP("wchan","ARC_WLAN_24G_Channel") %>
<% ABS_MAP("bandwidth","ARC_WLAN_24G_ChannelBandwidth") %>
<% ABS_MAP("wps_enable","ARC_WLAN_24G_WPS_Enable") %>
<% ABS_ARR("name","ARC_WLAN_24G_SSID_x_ESSID") %>
<% ABS_ARR("en","ARC_WLAN_24G_SSID_x_Enable") %>
<% ABS_ARR("bcast","ARC_WLAN_24G_SSID_x_Hide") %>

<% ABS_MAP("radiomode5g","ARC_WLAN_5G_RadioMode") %>
<% ABS_MAP("wchan5g","ARC_WLAN_5G_Channel") %>
<% ABS_MAP("bandwidth5g","ARC_WLAN_5G_ChannelBandwidth") %>
<% ABS_MAP("wps_enable5g","ARC_WLAN_5G_WPS_Enable") %>
<% ABS_MAP("name5g","ARC_WLAN_5G_SSID_x_ESSID+0") %>
<% ABS_MAP("en5g","ARC_WLAN_5G_SSID_x_Enable+0") %>
<% ABS_MAP("bcast5g","ARC_WLAN_5G_SSID_x_Hide+0") %>

REAL*/
