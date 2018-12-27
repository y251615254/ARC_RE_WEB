LangM.push({
'94':'Static IP',
'hlp283':'top',
'hlp284':'GLOSSARY OF TERMS',
'hlp285':'Help',
'300':'Self-Healing is an automatic maintenance function that keeps your network running smoothly and efficiently without the need for any user interaction. The router reinitializes itself at a set period of time on the days of your choosing, this frees up the routers memory and ensures the user is not required to restart or switch off the router. Clients may lose their connection to the router for a few seconds during the self healing cycle, although all should reconnect once it\'s complete. That\'s why it\'s best to select a time the network is less busy or not in use.',
'357':'MAC Address',
'hlp392':'Local Domain Name',
'416':'192.168.y.x (where y is anything between 0 and 255, and x is anything between 1 and 254.)',
'417':'10.y.y.x (where y is anything between 0 and 255, and x is anything between 1 and 254.)',
'418':'172.z.y.x (where z is anything from 16 to 31, and y is anything between 0 and 255, and x is anything between 1 and 254.)',
'696':'PPTP',
'698':'Telstra BigPond',
'790':'Wireless Mode',
'794':'Broadcast SSID',
'797':'Protected Mode',
//protect mode
'798':'NOTE: In most situations, best performance (throughput) is achieved with Protected Mode OFF. If you are operating in an environment with HEAVY 802.11b traffic or interference, best performance may be achieved with Protected Mode ON.',
'829':'Use as Access Point',
// QoS
'875':'QoS (Quality of Service) Configuration',
'876':'QoS prioritizes important data on your network such as multimedia content and voice-over-IP (VoIP) so it will not be interfered with by other data being sent over the network. Based on 802.11e, this feature can be turned on or off and you can choose the acknowledgement mode you want to use. If you plan to stream multimedia content or use VoIP on your network, the QoS feature should be enabled.<br>',
// Guest access
'2702':'Guest Access allows access to the Internet thru the WAN port, but limits guests from accessing the internal network, LAN and WLAN. The feature is supported on 2.4GHz only to simplify use.',
// mac
'883':'Setting MAC Address Control',
'884':'The MAC address filter is a powerful security feature that allows you to specify which computers are allowed on the wireless network. Note: This list applies only to wireless computers. This list can be configured so any computer attempting to access the wireless network that is not specified in the filter list will be denied access. When you enable this feature, you must enter the MAC address of each client (computer) to which you want to allow network access. The \"Block\" feature lets you turn on and off access to the network easily for any computer without having to add and remove the computer\'s MAC address from the list.',
'885':'Setting up an Allow Access List',
'886':'1 Click \"Allow\" to start to set a list of computers that can connect to a wireless network.<br>',
'887':'2. Next, in the \"MAC Address\" field that is blank (3), type in the MAC address of the wireless computer you want to be able to access the wireless network, then click \"Add\" (4).',
'888':'3. Continue to do this until all of the computers you want to add have been entered.<br>',
'889':'4. Click \"SAVE SETTING\" (5) to finish.',
'890':'Setting up a Deny Access List ',
'891':'The \"Deny Access\" list lets you specify computers that you DO NOT want to access the network. Any computer in the list will not be allowed access to the wireless network. All others will.',
'892':'1.Select the \"Deny\" radio button (2) to begin setting up a list of computers to be denied access to the wireless network.<br>',
'893':'2. Next, in the \"MAC Address\" field that is blank (3), type in the MAC address of the wireless computer you want to deny access to the wireless network, then click \"Add\" (4).<br>',
'894':'3. Continue to do this until all of the computers you want to deny access to have been entered.<br>',
'895':'4. Click \"SAVE SETTING\" (5) to finish.<br>',
//wds
'hlp900':'Wireless Distribution System',
'hlp901':'It allows a wireless network to be expanded using multiple access points without the traditional requirement for a wired backbone to link them.',
// lan ip
'hlp909':'The \"IP address\" is the Internal IP address of the Router. To access the advanced setup interface, type this IP address into the address bar of your browser. This address can be changed if needed. To change the IP address, type in the new IP address and click \"SAVE SETTING\". The IP address you choose should be a non-routable IP. Examples of a non-routable IP are:',
// lan mask
'hlp917':'There is no need to change the subnet mask. It is possible to change the subnet mask if necessary. Only make changes to the subnet mask if you specifically have a reason to do so.',
// DNS
'hlp920':'DNS is an acronym for Domain Name Server. A Domain Name Server is a server located on the Internet that translates URL\'s (Uniform Resource Locator) like www.belkin.com to IP addresses. Many ISP\'s do not require you to enter this information into the Router. The \"Automatic from ISP\" checkbox should be checked if your ISP did not give you a specific DNS address. If you are using a Static IP connection type, then you may need to enter a specific DNS address and secondary DNS address for your connection to work properly. If your connection type is Dynamic or PPPoE, it is likely that you do not have to enter a DNS address. To enter the DNS address settings, uncheck the \"Automatic from ISP\" checkbox and enter your DNS entries in the spaces provided. Click \"SAVE SETTING\" to save the settings.',
//dhcpserver
'hlp929':'The DHCP server function makes setting up a network very easy by assigning IP addresses to each computer on the network. The DHCP Server can be turned off if necessary. Turning off the DHCP server will require you to manually set a Static IP address in each computer on your network. The IP pool is the range of IP addresses set aside for dynamic assignment to the computers on your network. The default is 2-100 (99 computers) if you want to change this number, you can by entering a new starting and ending IP address and clicking on \"SAVE SETTING\".',
//local_domain_name
'hlp935':'You can set a local domain name (network name) for your network. There is no need to change this setting unless you have a specific advanced need to do so. You can name the network anything you want such as \"MY NETWORK\".',
//dynamicip
'hlp938':'Dynamic IP',
'hlp939':'A dynamic connection type is the most common connection type found with cable modems. Setting the connection type to dynamic in many cases is enough to complete the connection to your ISP. Some dynamic connection types may require a Host Name. You can enter your Host Name in the space provided if you were assigned one. Your Host Name is assigned by your ISP. Some dynamic connections may require that you clone the MAC address of the PC that was originally connected to the modem. To do so, click on the \"Change WAN MAC address\" link in the screen. The Internet Status indicator will read, \"Connected\" if your Router is set up properly.',
//staticip
'946':'A Static IP address connection type is less common than other connection types. If your ISP uses static IP addressing, you will need your IP address, Subnet Mask, and ISP gateway address. This information is available from your ISP or on the paperwork that your ISP left with you. Type in your information then click \"SAVE SETTING\". After you apply the changes, the Internet Status indicator will read, \"Connected\" if your Router is set up properly.',
//bridge
'950':'Modem only (Disable Internet Sharing)',
'951':'Modem Only Mode disables the routing functionality of your wireless router and makes it behave like a standard Ethernet modem. Enabling this functional will disable the DHCP function of your router and will not allow multiple computers to connect to the internet. This option should only be used if you wish to bypass the routers firewall and have a direct connection.',
// pppoE
'954':'Most DSL providers use PPPoE as the connection type. If you use a DSL modem to connect to the Internet, your ISP may use PPPoE to log you into the service. If you have an Internet connection in your home or small office that doesn\'t require a modem, you may also use PPPoE. Your connection type is PPPoE if:',
'959':'1) Your ISP gave you a user name and password which is required to connect to the Internet',
'960':'2) Your ISP gave you software such as WinPOET, Enternet300 that you use to connect to the Internet',
'961':'3) You have to double-click on a desktop Icon other than your browser to get on the Internet',
'962':'To set the Router to use PPPoE, type in your User Name and Password in the spaces provided. If you do not have a Service Name or do not know it, leave the Service Name field blank. After you have typed in your information, click \"SAVE SETTING\". After you apply the changes, the Internet Status indicator will read, \"Connected\" if your Router is set up properly. For more details on configuring your Router to use PPPoE, see the user manual.',
//PPTP
'967':'Some ISPs require a connection using PPTP protocol. This sets up a direct connection to the ISP\'s system. Type in the information provided by your ISP in the space provided. When you have finished, click \"SAVE SETTING\". After you apply the changes, the Internet Status indicator will read \"Connected\" if your Router is set up properly.',
// Bigpond
'971':'Your user name and password are provided to you by Telstra BigPond. Enter this information below. Choosing your state will automatically fill in your Login Server IP address. If your Login Server address is different than one provided here, you may manually enter the Login Server IP address. When you have entered your information, click \"SAVE SETTING\". After you apply the changes, the Internet Status indicator, will read \"Connected\" if your Router is set up properly.',
//MTU
'978':'MTU Setting',
'979':'The MTU setting should never be changed unless your ISP gives you a specific MTU setting. Making changes to the MTU setting can cause problems with your Internet connection including disconnection from the Internet, slow Internet access and problems with Internet applications working properly.',
//disconnect
'982':'Disconnect after X...',
'983':'The Disconnect feature is used to automatically disconnect the router from your ISP when there is no activity for a specified period of time. For instance, placing a checkmark next to this option and entering 5 into the minute field will cause the router to disconnect from the Internet after 5 minutes of no Internet activity. This option should be used if you pay for your Internet service by the minute.',
//macaddr
'988':'MAC is an acronym for Media Access Controller. All network components including cards, adapters, and routers, have a unique \"serial number\" called a MAC address. Your ISP may record the MAC address of your computer\'s adapter and only let that particular computer connect to the Internet service. When you install the router, the Router\'s own MAC address will be \"seen\" by the ISP and may cause the connection not to work. Belkin has provided the ability to clone (copy) the MAC address of the computer into the router. This MAC address, in turn, will be seen by the ISP\'s system as the original MAC address and will allow the connection to work. If you are not sure if your ISP needs to see the original MAC address, simply clone the MAC address of the computer that was originally connected to the modem. Cloning the address will not cause any problems with your network.',
'998':'To Clone your MAC address, make sure that you are using the computer which was ORIGINALLY CONNECTED to your modem before the Router was installed. Click the \"Clone MAC address\" button. Click \"SAVE SETTING\". Your MAC address is now cloned to the router.',
//channel_ssid
'1001':'To change the channel of operation of the Router, select the desired channel from the drop-down menu and select your channel. Click \"SAVE SETTING\" to save the setting. You can also change the SSID. The SSID is the equivalent to the wireless network\'s name. You can make the SSID anything you want to. If there are other wireless networks in your area, you should give your wireless network a unique name. The default is belkin.xxx. To change the SSID, click inside of the SSID box and type in a new name. Click \"SAVE SETTING\" to make the change.',
//broad_ssid
'1005':'It is possible to make your wireless network nearly invisible. By turning off the broadcast of the SSID, your network will not appear in a site survey. Site Survey is a feature of many wireless network adapters on the market today. It will scan the \"air\" for any available network and allow the computer to select the network from the site survey. Turning off the broadcast of the SSID will help increase security.',
//Access_Point
'1010':'When using the Router as an Access Point, you must specify an IP address for the Access Point. This IP address must fall into the same range as the network that you will be connecting it to. To access the advanced setup interface of the Router again, type in the IP address in the web browser and login.',
//wireless_mode
'1012':'This switch allows you to set the Router\'s wireless modes.',
'1013':'There are four modes in 2.4GHz band.',
'1014':'Using the Wireless Mode Switch',
'1015':'There are four modes.',
'1016':'802.11b & 802.11g<br>Setting the Router to this mode will allow 802.11b and 802.11g-compliant devices to join the network.',
'1017':'Off<br>This mode will turn OFF the Router\'s access point, so no wireless devices can join the network. Turning off the wireless function of your Router is a great way to secure your network when you are away from home for a long period of time, or don\'t want to use the wireless feature of the Router at a certain time.',
'1018':'802.11g only<br>Setting the Router to this mode will allow only 802.11g-compliant devices to join the network, keeping out any slower 802.11b devices.',
'1019':'802.11n only<br>Setting the Router to this mode will allow only 802.11n-compliant devices to join the network, keeping out any slower 802.11b and 802.11g devices.',
'1020':'802.11n & 802.11g & 802.11b<br>Setting the Router to this mode will let 802.11n-, 802.11g-, and 802.11b-compliant devices to join the network.',
//5g
'2600':'There are four modes in 5GHz band.',
'2601':'802.11a only<br>Setting the Router to this mode will only allow 802.11a-compliant devices to join the network.',
'2602':'802.11a & 802.11n<br>Setting the Router to this mode will allow 802.11a and 802.11n-compliant devices to join the network.',
'2603':'802.11n only<br>Setting the Router to this mode will allow only 802.11n-compliant devices to join, keeping out slower 802.11a devices.',
'2604':'Off<br>This mode will turn OFF the Router\'s Wi-Fi, so no wireless devices can join the network. Turning off the wireless function of your Router is a great way to secure your network when you are away from home for a long period of time, or don\'t want to use the wireless feature of the Router at a certain time.',
// Vertial Server
'1023':'This function will allow you to route external (Internet) calls for services such as a web server (port 80), FTP server (Port 21), or other applications through your Router to your internal network. Since your internal computers are protected by a firewall, machines from the Internet cannot get to them because they cannot be \"seen\". If you need to configure the Virtual Server function for a specific application, a list of common applications has been provided. If your application is not listed, you will need to contact the application vendor to find out which port settings you need. To select from the provided list, select your application from the drop-down list. Select the row that you want to copy the settings to from the drop-down list next to \"to row\", then click \"Enter\". The settings will be transferred to the row you specified. Click \"SAVE SETTING\" to save the setting for that application. To manually enter settings, enter the IP address in the space provided for the internal (server) machine, the port(s) required to pass (use a hyphen between multiple ports), select the port type (TCP or UDP) and click \"SAVE SETTING\". You can only pass one port per internal IP address. Opening ports in your firewall can pose a security risk. You can enable and disable settings very quickly. It is recommended that you disable the settings when you are not using a specific application.',
//ip_filter
'1040':'The Router can be configured to restrict access to the Internet, e-mail or other network services at specific days and times. Restriction can be set for a single computer, a range of computers, or multiple computers. To restrict Internet access to a single computer for example, enter the IP address of the computer you wish to restrict access to in the IP fields. Next enter 80 and 80 in the Port fields. Select TCP. Select Block. You can also select Always to block access all of the time. Select the day to start on top, the time to start on top, the day to end on the bottom and the time to stop on the bottom. Click \"SAVE SETTING\". The computer at the IP address you specified will now be blocked from Internet access at the times you specified. Note: be sure you have selected the correct time zone under Utilities> System Settings> Time Zone.',
//mac_filtering
'1050':'MAC Address Filtering',
'1051':'The MAC Address Filter is a powerful security feature that allows you to specify which computers are allowed on the network. Any computer attempting to access the network that is not specified in the filter list will be denied access. When you enable this feature, you must enter the MAC address of each client on your network to allow network access to each. To enable this feature, select \"Enable MAC Address Filtering\". Next, enter the MAC address of each computer on your network by clicking \"Add\" and entering the MAC address in the space provided. Click \"SAVE SETTING\" to save the settings. To delete a MAC address from the list, simply click \"Delete\" next to the MAC address you wish to delete. Click \"SAVE SETTING\" to save the settings.',
'1062':'Note: you will not be able to delete the MAC address of the computer you are using to access the Router\'s administrative functions. (The computer you are using now).',
// Access control
'1763':'The Router can be configured to restrict/allow access to the Internet, email, or other network services at specific days and times. Restriction can be set for a single computer, a range of computers, or multiple computers. Press \"Add\"  button to use this feature.',
// DMZ
'1064':'The DMZ feature allows you to specify one computer on your network to be placed outside of the NAT firewall. This may be necessary if the NAT feature is causing problems with an application such as a game or video conferencing application. Use this feature on a temporary basis. The computer in the DMZ is not protected from hacker attacks. To put a computer in the DMZ, enter the last digits of its IP address in the IP field and select \"Enable\". Click \"SAVE SETTING\" for the change to take effect.',
// ICMP blocking
'1076':'Block ICMP Ping',
'1077':'Computer hackers use what is known as \"Pinging\" to find potential victims on the Internet. By pinging a specific IP address and receiving a response from the IP address, a hacker can determine that something of interest might be there. The Router can be set up so it will not respond to an ICMP Ping from the outside. This heightens the level of security of your Router. To turn off the ping response, select \"Block ICMP Ping\" and click \"SAVE SETTING\". The router will not respond to an ICMP ping.',
// System Log
'1078':'Security Log',
'1079':'System Log provide system level message.',
'1080':'Firewall Log displays any illegal attempts to access your network.',
//admin_password
'1083':'Administrator Password',
'1084':'The Router ships with NO password entered. If you wish to add a password for more security, you can set a password here. Keep your password in a safe place, as you will need this password if you need to log into the router in the future. It is also recommended that you set a password if you plan to use the Remote management feature of this Router.',
'1089':'The login timeout option allows you to set the period of time that you can be logged into the Router\'s advanced setup interface. The timer starts when there has been no activity. For example, you have made some changes in the advanced setup interface, then left your computer alone without clicking \"Logout\". Assuming the timeout is set to 10 minutes, then 10 minutes after you leave, the login session will expire. You will have to login to the router again to make any more changes. The login timeout option is for security purposes and the default is set to 10 minutes. As a note, only one computer can be logged into the Router\'s advanced setup interface at one time.',
//time_zone
'1097':'Time and Time Zone',
'1098':'The Router keeps time by connecting to a Simple Network Time Protocol (SNTP) server. This allows the Router to synchronize the system clock to the global Internet. The synchronized clock in the Router is used to record the security log and control client filtering. Select the time zone that you reside in. If you reside in an area that observes Daylight Saving, then place a checkmark in the box next to \"Enable Daylight Saving\". The system clock may not update immediately. Allow at least 15 minutes for the router to contact the time servers on the Internet and get a response. You cannot set the clock yourself.',
//remote_mgmt
'1106':'Remote Management',
'1107':'Before you enable this function,',
'1108':'MAKE SURE YOU HAVE SET THE ADMINISTRATOR PASSWORD.',
'1109':'Remote management allows you to make changes to your Router\'s settings from anywhere on the Internet. There are two methods of remotely managing the router. The first method is to allow access to the router from anywhere on the Internet by selecting \"Any IP address can remotely manage the router\". By typing in your WAN IP address from any computer on the Internet, you will be presented with a login screen where you need to type in the password of your router. The Second method is to allow a specific IP address only to remotely manage the router. This is more secure, but less convenient. To use this method, enter the IP address you know you will be accessing the Router from in the space provided and select \"Only this IP address can remotely manage the Router\". Before you enable this function, it is STRONGLY RECOMMENDED that you set your administrator password. Leaving the password empty will potentially open your router to intrusion.',
// upnp
'1131':'UPnP (Universal Plug-and-Play) is a technology that offers seamless operation of voice messaging, video messaging, games, and other applications that are UPnP compliant. Some applications require the Router\'s firewall to be configured in a specific way to operate properly. This usually requires opening TCP and UDP ports and in some instances setting trigger ports. An application that is UPnP compliant has the ability to communicate with the Router, basically \"telling\" the Router which way it needs the firewall configured. The Router ships with the UPnP feature disabled. If you are using any applications that are UPnP compliant, and wish to take advantage of the UPnP features, you can enable the UPnP feature. Simply select \"Enable\" in the \"UPnP Enabling\" section of the Utilities page. Click \"SAVE SETTING\" to save the change.',
//autoupdate
'1141':'Automatic Firmware Update Notification',
'1142':'The Router has the capability built-in to automatically check for a new version of firmware and alert you that the new firmware is available. When you log into the Router advanced interface, the router will perform a check to see if new firmware is available. If new firmware is available, you will be notified. You can choose to download the new version or ignore it. The router ships with this feature disabled If you want to enable it, select \"Enable\" and click \"SAVE SETTING\".',
// wifi sec
'1148':'Wireless Security',
'1149':'Using Encryption can help secure your wireless network. Only one type of security may be selected at a time. Therefore the customer must select a mode that is supported on all network devices on the wireless network. This Belkin product has 4 possible Security settings:',
'1153':'Disabled. No encryption is enabled in this mode. Open networks where all users are welcome sometimes prefer not to enable encryption.',
'1154':'             welcome sometimes prefer to not enable encryption.',
'1155':'WPA/WPA2-Personal(PSK). WPA/WPA2 (Wireless protected Access) PSK is a recent ',
'1156':'standards-based security technique where each packet of information is',
'1157':'             encrypted with a different code, or key. Since the key is constantly changing, ',
'1158':'             WPA/WPA2 is very secure.',
/*OPT#UI_WPA_RADIUS*/
'1159':'There are two types of WPA, WPA-PSK (Pre-shared Key), and WPA-Radius Server. Obviously the difference being that one requires a server and one does not.',
/*END_OPT*/
//WPA-PSK
'1160':'WPA-PSK(Pre-shared Key) is for home and small business users who do not have a server. The PSK encryption key is generated automatically from a string of characters or Pass Phrase. Obviously, the biggest security risk in WPA PSK is if someone finds out your Pass Phrase.',
// TKIP
'1164':'a. TKIP verses AES. WPA uses TKIP as the encryption method while WPA2 uses AES. AES is a new encryption technique based on 802.11i standard and is the only security method allowed when using 802.11n.',
// Pre-shared
'1172':'b. Pre-shared Key. The key must be between <b>8</b> and <b>63</b> characters long and can include spaces and symbols, or <b>64</b> Hex (0-F) only. The same PSK must also be used for every other wireless network device on the network. Watch out for upper and lower case differences ( \"n\" is different than \"N\"). Remember, the easiest way to break your security is for someone to guess your PSK.',
// WEP128
'1176':'128-bit WEP. Until recently, 128-bit WEP (Wired Equivalent Privacy) was the standard for wireless encryption. If not all of your wireless devices support WPA, 128bit WEP still offers very good security option. It will require you to enter hex numbers, or you can generate them automatically.',
// WEP64
'1180':'64-bit WEP. Belkin only recommends 64-bit mode on networks where some devices do not support either WPA or 128bit WEP.',
//WPA
/*OPT#UI_WPA_RADIUS*/
'1182':'WPA - Radius Server. (This mode is accessed from the Advanced Button). WPA server is only for networks using a Radius Server. All parameter for this mode should be obtained from the administrator of your Radius Server. Unlike WPA PSK WPA server passes the key from the server to the clients instead of generating it automatically.',
/*END_OPT*/
// DDNS
'1186':'Using Dynamic DNS',
'1187':'The Dynamic DNS service allows you to alias a dynamic IP address to a static host name in any of the many domains DynDNS.org offers, allowing your network computers to be more easily accessed from various locations on the Internet. DynDNS.org provides this service, for up to five host names, free to the Internet community.',
'1188':'The Dynamic DNSSM service is ideal for a home website, file server, or to make it easy to access your home PC and stored files while you\'re at work. Using the service can ensure that your host name always points to your IP address, no matter how often your ISP changes it. When your IP address changes, your friends and associates can always locate you by visiting yourname.dyndns.org instead!',
'1190':'To register free for your Dynamic DNS host name, please visit http://www.dyndns.org.',
'1191':'<b>Setting up the Router\'s Dynamic DNS Update Client</b>',
'1192':'You must register with DynDNS.org\'s free update service before using this feature. Once you have your registration, follow the directions below.',
'1193':'1. Enter your DynDNS.org user name in the \"User Name\" field (1).',
'1194':'2. Enter your DynDNS.org password in the \"Password\" field (2).',
'1195':'3. Enter the DynDNS.org domain name you set up with DynDNS.org in the \"Domain Name\" field (3).',
'1196':'4. Click \"Update Dynamic DNS\" to update your IP address.',
'1197':'Whenever your IP address assigned by your ISP changes, the Router will automatically update DynDNS.org\'s servers with your new IP address. You can also do this manually by clicking the \"Update Dynamic DNS\" button.',
'':null});
