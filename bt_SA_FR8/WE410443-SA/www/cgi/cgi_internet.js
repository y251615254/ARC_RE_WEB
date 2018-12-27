/*REAL
<% ABS_MAP("internet_enable", "ARC_SYS_Internet") %>
<% ABS_MAP("uplink_status", "ARC_SYS_UPLink_Status") %>
var access_control_block_mac_info = <% get_access_control_block_mac_info()%>;
var station_info = <% dump_toplogy_station_info(); %>;
var internet_control_station_info = <% get_internet_control_status_info(); %>;
var access_control_profile = <% get_access_control_profile_info(); %>;

REAL*/


/*DEMO*/
addCfg("internet_enable",2,"1");
addCfg("uplink_status",235,"1");
//var access_control_block_mac_info = "AA:BB:CC:00:00:00,AA:BB:CC:01:01:01";

var access_control_block_mac_info={"result": 'AA:BB:CC:00:00:00,AA:BB:CC:01:01:01'};

var station_info=
				{
					    "stations": [
					        {
					            "station_mac": "AA:BB:CC:00:00:00",
					            "station_name": "pseudo0",
					            "alias_name": "demo1",
					            "station_ip": "192.168.1.0",
					            "parent_id": "E4:3E:D7:07:11:AA",
					            "connect_type": "Ether",
					            "link_rate": "123Mbps",
					            "signal_strength": "-40",
					            "pid": "pseduo_group01",
					            "online": "0",
					            "last_connect": "1477958612"
					        },
					        {
					            "station_mac": "AA:BB:CC:01:01:01",
					            "station_name": "pseudo1",
					            "alias_name": "demo2",
					            "station_ip": "192.168.1.1",
					            "parent_id": "E4:3E:D7:07:11:AA",
					            "connect_type": "2.4G",
					            "link_rate": "124Mbps",
					            "signal_strength": "-39",
					            "pid": "pseduo_group01",
					            "online": "1",
					            "last_connect": "0"
					        },
					        {
					            "station_mac": "AA:BB:CC:02:02:02",
					            "station_name": "pseudo2",
					            "alias_name": "NULL",
					            "station_ip": "192.168.1.2",
					            "parent_id": "E4:3E:D7:07:11:AA",
					            "connect_type": "5G",
					            "link_rate": "125Mbps",
					            "signal_strength": "-38",
					            "pid": "block",
					            "online": "0",
					            "last_connect": "1477958526"
					        },
					        {
					            "station_mac": "AA:BB:CC:03:03:03",
					            "station_name": "pseudo3",
					            "alias_name": "NULL",
					            "station_ip": "192.168.1.3",
					            "parent_id": "E4:3E:D7:07:11:AA",
					            "connect_type": "5G",
					            "link_rate": "126Mbps",
					            "signal_strength": "-37",
					            "pid": "pseduo_sta01",
					            "online": "1",
					            "last_connect": "0"
					        },
					        {
					            "station_mac": "AA:BB:CC:04:04:04",
					            "station_name": "pseudo4",
					            "alias_name": "NULL",
					            "station_ip": "192.168.1.4",
					            "parent_id": "E4:3E:D7:07:11:AA",
					            "connect_type": "2.4G_guest",
					            "link_rate": "127Mbps",
					            "signal_strength": "-36",
					            "pid": "pseduo_guest01",
					            "online": "0",
					            "last_connect": "1477958360"
					        },
					        {
					            "station_mac": "AA:BB:CC:05:05:05",
					            "station_name": "pseudo5",
					            "alias_name": "NULL",
					            "station_ip": "192.168.1.5",
					            "parent_id": "E4:3E:D7:07:11:AA",
					            "connect_type": "Ether",
					            "link_rate": "128Mbps",
					            "signal_strength": "-35",
					            "pid": "pseduo_guest01",
					            "online": "1",
					            "last_connect": "0"
					        }
					    ]
					};
/*END_DEMO*/
