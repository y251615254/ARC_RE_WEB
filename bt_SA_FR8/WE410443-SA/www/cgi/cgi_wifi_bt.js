
/*REAL
<% ABS_ARR("24genable","ARC_WLAN_24G_SSID_x_Enable") %>
<% ABS_MAP("guest_enable","ARC_SYS_GUEST_SSID_Enable") %>
<% ABS_MAP("guest_bw_alloc","ARC_SYS_GUEST_SSID_BW_Allocation") %>
addCfg("24gname0","ARC_WLAN_24G_SSID_0_ESSID", "<% get_ssid(24,0); %>");
addCfg("24gname1","ARC_WLAN_24G_SSID_1_ESSID", "<% get_ssid(24,1); %>");
addCfg("24gname2","ARC_WLAN_24G_SSID_2_ESSID", "<% get_ssid(24,2); %>");
addCfg("24gname3","ARC_WLAN_24G_SSID_3_ESSID", "<% get_ssid(24,3); %>");
<% ABS_ARR("24gbcast","ARC_WLAN_24G_SSID_x_Hide") %>
<% ABS_ARR("24gsecurity_mode","ARC_WLAN_24G_SSID_x_SecurityType") %>
<% ABS_ARR("24gCypher_suite", "ARC_WLAN_24G_SSID_x_WPA_Cipher") %>
<% ABS_MAP("24gbandwidth","ARC_WLAN_24G_ChannelBandwidth") %>
<% ABS_MAP("24gcoexistence","ARC_WLAN_24G_HT_BSSCoexistence") %>

addCfg("5gname0","ARC_WLAN_5G_SSID_0_ESSID", "<% get_ssid(5,0); %>");
addCfg("5gname1","ARC_WLAN_5G_SSID_1_ESSID", "<% get_ssid(5,1); %>");
addCfg("5gname2","ARC_WLAN_5G_SSID_2_ESSID", "<% get_ssid(5,2); %>");
addCfg("5gname3","ARC_WLAN_5G_SSID_3_ESSID", "<% get_ssid(5,3); %>");
<% ABS_ARR("5gbcast","ARC_WLAN_5G_SSID_x_Hide") %>
<% ABS_ARR("5gsecurity_mode","ARC_WLAN_5G_SSID_x_SecurityType") %>
<% ABS_ARR("5gCypher_suite", "ARC_WLAN_5G_SSID_x_WPA_Cipher") %>

addCfg("24gwpapsk0","ARC_WLAN_24G_SSID_0_WPA_Passphrase", "<% get_wifi_psk(24,0); %>");
addCfg("24gwpapsk1","ARC_WLAN_24G_SSID_1_WPA_Passphrase", "<% get_wifi_psk(24,1); %>");
addCfg("24gwpapsk2","ARC_WLAN_24G_SSID_2_WPA_Passphrase", "<% get_wifi_psk(24,2); %>");
addCfg("24gwpapsk3","ARC_WLAN_24G_SSID_3_WPA_Passphrase", "<% get_wifi_psk(24,3); %>");
addCfg("5gwpapsk0","ARC_WLAN_5G_SSID_0_WPA_Passphrase", "<% get_wifi_psk(5,0); %>");
addCfg("5gwpapsk1","ARC_WLAN_5G_SSID_1_WPA_Passphrase", "<% get_wifi_psk(5,1); %>");
addCfg("5gwpapsk2","ARC_WLAN_5G_SSID_2_WPA_Passphrase", "<% get_wifi_psk(5,2); %>");
addCfg("5gwpapsk3","ARC_WLAN_5G_SSID_3_WPA_Passphrase", "<% get_wifi_psk(5,3); %>");

REAL*/
  
