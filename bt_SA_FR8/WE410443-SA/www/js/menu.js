//FEATURE LIST
var operation_func=8;
var vlan_func_enable=0;
var custom_page_flag =2;
var my_usb=1;

//MENU LIST

var MENU_LVL=0;
var MENU_TITL=1;
var MENU_NAME=2;
var MENU_URL=3;
var MENU_ON=4;
var MENU_TARGET=5;

var MENU_GRP=6;
var MENU_ELM=7;

var MenuList=[
	// Start
	[ '0',		"m_START",								"start",				'status.htm',		1],
		['1',		'400000',								'status',				'status.htm'],
		['1',		'400005',							'discs',	'discs.htm'],
		['1',		'400100',							'status_lan_device',	'status_lan_device.htm'],

	//	Data
	[ '0',			"m_DTAT",							"wireless_bt",			'ssid_bt.htm',		operation_func],
	['1',		'140010',								'wireless_bt',			'ssid_bt.htm',		operation_func],	
	['1',		'140020',								'wireless_guest',			'ssid_guest.htm',		operation_func],	
	['1',		'140030',								'access_control',			'access_control.htm',		operation_func],	
		['1',		"130000",							'lan',					'lan1.htm'],
		['1',		'400900',								'led',					'led.htm'],
		['1',		'401000',								'internet',				'internet.htm'],
		//['1',		'400101',								'Mac Address',				'firewall_mac.htm'],
			//['2',	"130100",							'lan_dhcp_static',		'lan_dhcp_static.htm'],
			//['2',	"130400",							'stp',				'stp.htm'],
			//['2',	"130200",							'v_lan',				'v_lan.htm', 		vlan_func_enable],
			//['2',	"130300",							'lan_portbase_vlan',		'lan_portbase_vlan.htm'],
		//['1',		'140000',								'wireless',			'wireless_main.htm',	operation_func],
				
		//['2',	"140100",							'wireless_24g',		'ssid24g.htm',		operation_func],	
/*OPT#PROJ_UI_MULTI_SSID*/
			//['2',	"140200",							'wireless_ssid2',		'ssid2.htm',		operation_func],
			//['2',	"140300",							'wireless_ssid3',		'ssid3.htm',		operation_func],
			//['2',	"140400",							'wireless_ssid4',		'ssid4.htm',		operation_func],
/*END_OPT*/
/*OPT#PROJ_UI_5G*/
			//['2',	"141000",							'wireless_5g',			'ssid5g.htm',		operation_func],
/*END_OPT*/
			//['2',	"140600",							'wireless_wps',			'wireless_WPS.htm' ],
			//['2',	"140603",							'wireless_advanced',	'wireless_advanced.htm' ],
			//['2',	"140601",							'wireless_wmm',			'wireless_wmm.htm' ],
			//['2',	"140602",							'wireless_wmm',			'wireless_wmm_5G.htm' ],
/*OPT#PROJ_UI_WLAN_MAC*/
			//['2',	"140700",							'wireless_mac',			'wireless_mac.htm' ,	operation_func],
/*END_OPT*/
/*OPT#PROJ_UI_WDS*/
			//['2',	"140710",							'wireless_wds',			'wireless_wds.htm' ,	operation_func],
/*END_OPT*/
			//['2',	"140900",							'wireless_ap_cli',		'wireless_ap_cli.htm' ,	operation_func],
			//['2',	"140900",							'wireless_apcli_main',		'wireless_apcli_main.htm' ,	operation_func],
		//['1',	"140900",							'wireless_apcli_status',		'wireless_apcli_status.htm' ,	operation_func],
			//['2',	"140910",							'wireless_apcli_main_24g',		'wireless_apcli_main_24g.htm' ,	operation_func],
			//['2',	"140920",							'wireless_apcli_main_5g',		'wireless_apcli_main_5g.htm' ,	operation_func],
			//['2',	"140920",							'apcli_wps_pin_method',		'apcli_wps_pin_method.htm' ,	operation_func],
		//['1',		"251000",							'tr64',					'tr64_main.htm'],
	//	Extras
	[ '0',			"m_EXTRA",							"system",				'system_main.htm',			1],
		['1',		"110700",							"system",				'system_main.htm',			1],
		['1',		"110200", 							'system_pt', 			'system_p.htm'],
		['1',		"230200",			 				'system_f',				'system_f.htm'],
		//['1',		"230201",			 				'system_fw_polling',	'system_fw_polling.htm'],
		['1',		"230400",			 				'system_r', 			'system_r.htm'],
		['1',		"110500", 							'systemlog', 			'security_log.htm'],
		//['1',		"110800", 							'debug_bt', 			'debug_bt.htm'],
		//['1',		"110900", 							'debug_arc', 			'debug_arc.htm'],
		null
];


