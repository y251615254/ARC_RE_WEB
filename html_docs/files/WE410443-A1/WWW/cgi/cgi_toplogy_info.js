/*REAL
var toplogy_info = <% dump_toplogy_map_info(); %>;
var station_info = <% dump_toplogy_station_info(); %>;
var mask_dns_gateway_status = <% get_mask_dns_gateway_status(); %>;

var wifi_radio_info = <% get_wifi_radio_info(); %>;
var wifi_inface_info = <% get_wifi_inface_info(); %>;


//var toplogy_info = { "nodes": [ { "device_mac": "D4:63:FE:D0:6D:D1", "device_name": "", "device_id": "D4:63:FE:D0:6D:D6", "device_ip": "192.168.1.3", "bssid_2g": "D4:63:FE:D0:6D:D2", "essid_2g": "HOMEZZAP_2222", "bssid_5g": "", "essid_5g": "", "parent_id": "NULL", "child_id": "00:0C:43:28:80:9C", "child_num": "1", "sta_num": "0", "connect_type": "", "model_name": "NULL" }, { "device_mac": "00:0C:43:28:80:97", "device_name": "", "device_id": "00:0C:43:28:80:9C", "device_ip": "192.168.1.4", "bssid_2g": "00:0C:43:28:80:98", "essid_2g": "HOMEZZAP_1111", "bssid_5g": "", "essid_5g": "", "parent_id": "D4:63:FE:D0:6D:D6", "child_id": "NULL", "child_num": "0", "sta_num": "0", "connect_type": "5G", "model_name": "NULL" }, ] };
//var station_info = { "stations": [ { "station_mac": "11:11:11:11:11:11", "station_name": "PC1", "station_ip": "192.168.1.101", "parent_id": "D4:63:FE:D0:6D:D6", "connect_type": "5G" }, { "station_mac": "22:22:22:22:22:22", "station_name": "PC2", "station_ip": "192.168.1.102", "parent_id": "D4:63:FE:D0:6D:D6","connect_type": "5G" }, { "station_mac": "33:33:33:33:33:33", "station_name": "PC3", "station_ip": "192.168.1.103", "parent_id": "D4:63:FE:D0:6D:D6","connect_type": "5G" }, { "station_mac": "44:44:44:44:44:44", "station_name": "PC4", "station_ip": "192.168.1.104", "parent_id": "00:0C:43:28:80:9C","connect_type": "5G" }, { "station_mac": "55:55:55:55:55:55", "station_name": "PC5", "station_ip": "192.168.1.105", "parent_id": "00:0C:43:28:80:9C","connect_type": "5G" } ] };
REAL*/
/*DEMO*/
var toplogy_info={ "nodes": [ { "hw_ver": "R0A", "sw_ver": "v1.01.12 build06", "fw_ver": "v1.01.12 build06", "sn": "DWS123456d58d", "device_mac": "48:8D:36:18:AD:58", "device_name": "Wi-Fi Disc 57", "device_id": "48:8D:36:18:AD:57", "device_ip": "192.168.1.101", "device_netmask": "255.255.255.0", "eth_mac": "48:8D:36:18:AD:5A", "bssid_2g": "48:8D:36:18:AD:58", "essid_2g": "BTRepeaterAP24G_368", "bssid_5g": "48:8D:36:18:AD:59", "essid_5g": "BTRepeaterAP5G_368", "parent_id": "NULL", "child_id": "D0:05:2A:7B:B2:0A", "child_num": "1", "sta_num": "1", "connect_type": "Ether", "connect_rssi": "-127", "model_name": "WE410443-A1", "product_id": "WE410443-A1", "node_lvid": "1", "uptime": "5509", "cpuU": "2", "cpuS": "10", "cpuI": "88", "memT": "122160", "memF": "57096", "memU": "65064", "connected_role": "configured_master", "connected_rootap": "1", "linkrate": "100Mbps", "linkmode": "0" }, { "hw_ver": "R0A", "sw_ver": "v1.01.12 build06", "fw_ver": "v1.01.12 build06", "sn": "DWS1234565846", "device_mac": "D0:05:2A:7B:B2:0B", "device_name": "Wi-Fi Disc 0a", "device_id": "D0:05:2A:7B:B2:0A", "device_ip": "192.168.1.102", "device_netmask": "255.255.255.0", "eth_mac": "D0:05:2A:7B:B2:0D", "bssid_2g": "D0:05:2A:7B:B2:0B", "essid_2g": "BTRepeaterAP24G_368", "bssid_5g": "D0:05:2A:7B:B2:0C", "essid_5g": "BTRepeaterAP5G_368", "parent_id": "48:8D:36:18:AD:57", "child_id": "NULL", "child_num": "0", "sta_num": "0", "connect_type": "5G", "connect_rssi": "-44", "model_name": "WE410443-A1", "product_id": "WE410443-A1", "node_lvid": "17", "uptime": "2867", "cpuU": "0", "cpuS": "2", "cpuI": "98", "memT": "122160", "memF": "63936", "memU": "58224", "connected_role": "configured_slave", "connected_rootap": "0", "linkrate": "1733Mbps", "linkmode": "0" } ], "cksum": "f5509194065c6468935cd8f9e7e60b0" };
var station_info={"stations":[{"station_mac":"B0:E2:35:C1:54:E8","station_name":"MI5-xiaojinguang","alias_name":"NULL","station_ip":"192.168.1.106","parent_id":"48:8D:36:18:AD:57","connect_type":"5G","link_rate":"390Mbps","link_rate_max":"0Mbps","mode":"8","signal_strength":"-44","signal_strength_max":"0","signal_strength_min":"-87","pid":"4081655","online":"1","last_connect":"1512543353","ipv6_ip":"","note":"0","as":"1","ldur":"0","lddr":"0","rt":"0","bs":"5713030","br":"1514295","txc":"7979","rxc":"10585","es":"0","rtc":"0","frc":"0","rc":"0","mrc":"0"},{"station_mac":"5C:F9:DD:59:34:37","station_name":"test-PC","alias_name":"NULL","station_ip":"NULL","parent_id":"00:00:00:00:00:00","connect_type":"unknown","link_rate":"0Mbps","link_rate_max":"0Mbps","mode":"0","signal_strength":"0","signal_strength_max":"0","signal_strength_min":"0","pid":"NULL","online":"0","last_connect":"1512541970","ipv6_ip":"","note":"0","as":"0","ldur":"0","lddr":"0","rt":"0","bs":"0","br":"0","txc":"0","rxc":"0","es":"0","rtc":"0","frc":"0","rc":"0","mrc":"0"}]};
var mask_dns_gateway_status="255.255.255.0@192.168.1.1@192.168.1.1";
var wifi_radio_info={ "radiostats": [ { "mac": "48:8D:36:18:AD:58", "wrst": [ 
{ "bid": "0", "bs": "1602839", "br": "55101", "ps": "10436", "pr": "2078", "es": "0", "er": "0", "dps": "0", "dpr": "0", "plcpec": "0", "fcsec": "0", "imacc": "0", "por": "0", "n": "0" }, 
{ "bid": "10", "bs": "30984974", "br": "4496476", "ps": "50037", "pr": "36939", "es": "0", "er": "2", "dps": "0", "dpr": "4609", "plcpec": "0", "fcsec": "0", "imacc": "0", "por": "0", "n": "0" } ] }, 
{ "mac": "D0:05:2A:7B:B2:0B", "wrst": [ { "bid": "0", "bs": "0", "br": "0", "ps": "0", "pr": "0", "es": "0", "er": "0", "dps": "0", "dpr": "0", "plcpec": "0", "fcsec": "0", "imacc": "0", "por": "0", "n": "0" }, 
{ "bid": "10", "bs": "0", "br": "0", "ps": "0", "pr": "0", "es": "0", "er": "0", "dps": "0", "dpr": "0", "plcpec": "0", "fcsec": "0", "imacc": "0", "por": "0", "n": "0" } ] } ] };

var wifi_inface_info={ "ifacestats": [ { "mac": "48:8D:36:18:AD:58", "wis": [ 
{ "if": "100", "bs": "0", "br": "0", "ps": "0", "pr": "0", "es": "0", "rc": "0", "frc": "0", "rtc": "0", "mrc": "0", "afc": "0", "apc": "0", "er": "0", "ups": "0", "upr": "0", "dps": "0", "dpr": "0", "mps": "0", "mpr": "0", "bps": "0", "bpr": "0", "uppr": "0" }, { "if": "200", "bs": "23165646", "br": "4436848", "ps": "29458", "pr": "34691", "es": "0", "rc": "0", "frc": "0", "rtc": "0", "mrc": "0", "afc": "0", "apc": "0", "er": "2", "ups": "0", "upr": "0", "dps": "0", "dpr": "4609", "mps": "0", "mpr": "0", "bps": "0", "bpr": "0", "uppr": "0" }, { "if": "116", "bs": "0", "br": "0", "ps": "0", "pr": "0", "es": "0", "rc": "0", "frc": "0", "rtc": "0", "mrc": "0", "afc": "0", "apc": "0", "er": "0", "ups": "0", "upr": "0", "dps": "0", "dpr": "0", "mps": "0", "mpr": "0", "bps": "0", "bpr": "0", "uppr": "0" }, { "if": "216", "bs": "0", "br": "0", "ps": "0", "pr": "0", "es": "0", "rc": "0", "frc": "0", "rtc": "0", "mrc": "0", "afc": "0", "apc": "0", "er": "0", "ups": "0", "upr": "0", "dps": "0", "dpr": "0", "mps": "0", "mpr": "0", "bps": "0", "bpr": "0", "uppr": "0" } ] }, 
{ "mac": "D0:05:2A:7B:B2:0B", "wis": [ 
{ "if": "100", "bs": "0", "br": "0", "ps": "0", "pr": "0", "es": "0", "rc": "0", "frc": "0", "rtc": "0", "mrc": "0", "afc": "0", "apc": "0", "er": "0", "ups": "0", "upr": "0", "dps": "0", "dpr": "0", "mps": "0", "mpr": "0", "bps": "0", "bpr": "0", "uppr": "0" }, { "if": "200", "bs": "0", "br": "0", "ps": "0", "pr": "0", "es": "0", "rc": "0", "frc": "0", "rtc": "0", "mrc": "0", "afc": "0", "apc": "0", "er": "0", "ups": "0", "upr": "0", "dps": "0", "dpr": "0", "mps": "0", "mpr": "0", "bps": "0", "bpr": "0", "uppr": "0" }, { "if": "116", "bs": "0", "br": "0", "ps": "0", "pr": "0", "es": "0", "rc": "0", "frc": "0", "rtc": "0", "mrc": "0", "afc": "0", "apc": "0", "er": "0", "ups": "0", "upr": "0", "dps": "0", "dpr": "0", "mps": "0", "mpr": "0", "bps": "0", "bpr": "0", "uppr": "0" }, { "if": "216", "bs": "0", "br": "0", "ps": "0", "pr": "0", "es": "0", "rc": "0", "frc": "0", "rtc": "0", "mrc": "0", "afc": "0", "apc": "0", "er": "0", "ups": "0", "upr": "0", "dps": "0", "dpr": "0", "mps": "0", "mpr": "0", "bps": "0", "bpr": "0", "uppr": "0" } ] } ] };
/*END_DEMO*/
