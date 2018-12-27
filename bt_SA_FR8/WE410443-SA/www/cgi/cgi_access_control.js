/*REAL
//var toplogy_info = <% dump_toplogy_map_info(); %>;
var station_info = <% dump_toplogy_station_info(); %>;
var access_control_profile = <% get_access_control_profile_info(); %>;
REAL*/

/*DEMO*/

//var toplogy_info = { "nodes": [ { "device_mac": "D4:63:FE:D0:6D:D1", "device_name": "RE1", "device_id": "D4:63:FE:D0:6D:D6", "device_ip": "192.168.1.3", "bssid_2g": "D4:63:FE:D0:6D:D2", "essid_2g": "HOMEZZAP_2222", "bssid_5g": "", "essid_5g": "", "parent_id": "NULL", "child_id": "00:0C:43:28:80:9C", "child_num": "1", "sta_num": "0", "connect_type": "", "model_name": "NULL" }, { "device_mac": "00:0C:43:28:80:97", "device_name": "RE2", "device_id": "00:0C:43:28:80:9C", "device_ip": "192.168.1.4", "bssid_2g": "00:0C:43:28:80:98", "essid_2g": "HOMEZZAP_1111", "bssid_5g": "", "essid_5g": "", "parent_id": "D4:63:FE:D0:6D:D6", "child_id": "NULL", "child_num": "0", "sta_num": "0", "connect_type": "5G", "model_name": "NULL" }, ] };
var station_info={ "stations": [ 
									{ 
										"station_mac": "9C:2A:70:5E:97:D9", 
										"station_name": "arcadyan-PC", 
										"alias_name": "NULL", 
										"station_ip": "NULL", 
										"parent_id": "00:00:00:00:00:00", 
										"connect_type": "Ether", 
										"link_rate": "0Mbps", 
										"signal_strength": "0", 
										"profile_id": "NULL", 
										"online": "0", 
										"last_connect": "1496367250" 
									}, 
									{ 
										"station_mac": "B0:C0:90:B1:FB:DD", 
										"station_name": "DESKTOP-GLDMJFG", 
										"alias_name": "NULL", 
										"station_ip": "NULL", 
										"parent_id": "00:00:00:00:00:00", 
										"connect_type": "Ether", 
										"link_rate": "0Mbps", 
										"signal_strength": "0", 
										"profile_id": "NULL", 
										"online": "0", 
										"last_connect": "1496372047" 
									}, 
									{ 
										"station_mac": "AA:BB:CC:00:00:00", 
										"station_name": "pseudo0", 
										"alias_name": "NULL", 
										"station_ip": "192.168.1.0", 
										"parent_id": "E4:3E:D7:80:F0:9A", 
										"connect_type": "Ether", 
										"link_rate": "123Mbps", 
										"signal_strength": "-40", 
										"profile_id": "group0001", 
										"online": "0", 
										"last_connect": "1477958530" 
									}, 
									{ 
										"station_mac": "AA:BB:CC:01:01:01", 
										"station_name": "pseudo1", 
										"alias_name": "NULL", 
										"station_ip": "192.168.1.1", 
										"parent_id": "E4:3E:D7:80:F0:9A", 
										"connect_type": "2.4G", 
										"link_rate": "124Mbps", 
										"signal_strength": "-39", 
										"profile_id": "group0001", 
										"online": "1", 
										"last_connect": "0" 
									}, 
									{ 
										"station_mac": "AA:BB:CC:02:02:02", 
										"station_name": "pseudo2", 
										"alias_name": "NULL", 
										"station_ip": "192.168.1.2", 
										"parent_id": "E4:3E:D7:80:F0:9A", 
										"connect_type": "5G", 
										"link_rate": "125Mbps", 
										"signal_strength": "-38", 
										"profile_id": "block", 
										"online": "0", 
										"last_connect": "1477958444" 
									}, 
									{ 
										"station_mac": "AA:BB:CC:03:03:03", 
										"station_name": "pseudo3", 
										"alias_name": "NULL", 
										"station_ip": "192.168.1.3", 
										"parent_id": "E4:3E:D7:80:F0:9A", 
										"connect_type": "5G", 
										"link_rate": "126Mbps", 
										"signal_strength": "-37", 
										"profile_id": "station0001", 
										"online": "1", 
										"last_connect": "0" 
									}, 
									{ 
										"station_mac": "AA:BB:CC:04:04:04", 
										"station_name": "pseudo4", 
										"alias_name": "NULL", 
										"station_ip": "192.168.1.4", 
										"parent_id": "E4:3E:D7:80:F0:9A", 
										"connect_type": "2.4G_guest", 
										"link_rate": "127Mbps", 
										"signal_strength": "-36", 
										"profile_id": "guest0001", 
										"online": "0", 
										"last_connect": "1477958278" 
									}, 
									{ 
										"station_mac": "AA:BB:CC:05:05:05", 
										"station_name": "pseudo5", 
										"alias_name": "NULL", 
										"station_ip": "192.168.1.5", 
										"parent_id": "E4:3E:D7:80:F0:9A", 
										"connect_type": "Ether", 
										"link_rate": "128Mbps", 
										"signal_strength": "-35", 
										"profile_id": "guest0001", 
										"online": "1", 
										"last_connect": "0" 
									} ] };
									
							
var access_control_profile={"profile":[
										{
											"profile_id":"block",
											"profile_name":"",
											"configs":[
														{
															"id":"00000",
															"enable":"1",
															"type":"0",
															"day":"0000000",
															"s_time":"0",
															"e_time":"0",
															"note":""
														}]
										},
										{
											"profile_id":"guest0001",
											"profile_name":"",
											"configs":[
														{
															"id":"00001",
															"enable":"1",
															"type":"2",
															"day":"1000000",
															"s_time":"1800",
															"e_time":"3600",
															"note":""
														},
														{
															"id":"00002",
															"enable":"1",
															"type":"2",
															"day":"0000010",
															"s_time":"3600",
															"e_time":"7200",
															"note":""
														}
													]
										},
										{
											"profile_id":"group0001",
											"profile_name":"",
											"configs":[
														{
															"id":"00001",
															"enable":"1",
															"type":"2",
															"day":"1000000",
															"s_time":"1800",
															"e_time":"3600",
															"note":""
														},
														{
															"id":"00001",
															"enable":"1",
															"type":"1",
															"day":"0000010",
															"s_time":"7200",
															"e_time":"10800",
															"note":""
														}
													]
										},
										{
											"profile_id":"station0001",
											"profile_name":"",
											"configs":[
														{
															"id":"00001",
															"enable":"1",
															"type":"2",
															"day":"1000000",
															"s_time":"1800",
															"e_time":"3600",
															"note":""
														},
														{
															"id":"00001",
															"enable":"1",
															"type":"2",
															"day":"0000010",
															"s_time":"7200",
															"e_time":"16800",
															"note":""
														}
													]
										}]};

/*END_DEMO*/


