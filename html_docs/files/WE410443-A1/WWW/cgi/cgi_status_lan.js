
var LAN_NUM=0;
var LAN_LINK=1;
var LAN_SPEED=2;
var LAN_DUPLEX=3;
var STR_LAN_DESC="LAN";
function LANentry(idx){
	if(lan_status_list[idx]){
		a=lan_status_list[idx];
		a[LAN_NUM]=STR_LAN_DESC+a[LAN_NUM];
		a[LAN_LINK]=HTML2str(a[LAN_LINK]);
		a[LAN_SPEED]=HTML2str(a[LAN_SPEED]);
		a[LAN_DUPLEX]=HTML2str(a[LAN_DUPLEX]);
		return a;
	}
	return null;
}


/*REAL
<% ABS_MAP("lan_mac","ARC_LAN_x_MACaddr"+0) %>
<% ABS_MAP("lan_ip","ARC_LAN_x_IP4_Addr"+0) %>
<% ABS_MAP("lan_mask","ARC_LAN_x_IP4_Netmask"+0) %>

//format: [lan],[link],[speed],[duplex]

//var lan_status_list=[['1','UP','100','FULL'],['2','DOWN','100','FULL'],['3','UP','100','FULL'],['4','DOWN','100','FULL'],null];
var lan_status_list=[<% get_lan_status(); %> null];
var lan_statistics_list='<% get_lan_statistics(); %>';

REAL*/


