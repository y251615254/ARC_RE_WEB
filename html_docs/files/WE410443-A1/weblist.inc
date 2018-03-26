#
# ZZ388 web UI 2013/12/10 (Router/Wireless mode)
#
#  including UI data and major UI entry pages
#  and UI framework
#
# default should enable English mode
#UI_LANG_EN=y
#

# Multi-lingual Support for all language, will keep under
# PROJ_UI_LANG_LST is uppercase format like "EN","TW",...
# we do convert to lowercase and prefix as <lang>_xx.js, where <lang> will be "en","tw","de",...

UI_LANG_LST=

#
# main pages
#
PAGES=
PAGES_SKIP=


# Common javascript UI Framwork
PAGES+= index.htm \
	Success.htm \
	Success_upload.htm \
	Success_restore.htm \
	Fail.htm \
	Fail_restore.htm \
	Fail_upgrade.htm \
	help.htm \
	.bypass \
	token_info.htm

# Fail_upload.htm dropped at 2013/12/19

# common JS scripts
PAGES+= js/global.js \
	js/routine.js \
	js/subformvar.js \
	js/menu.js \
	js/jquery-1.6.2.min.js \
	js/jquery.arctransform.js \
	js/init_data.js \
	js/data.js \
	js/supersleight-min.js \
	js/login_data.js \
	js/nat.js

PAGES_SKIP+=js/jquery-1.6.2.min.js .allow

#
# CSS
#
PAGES+=	css/c_voda_top.css \
	css/ie.css \
	css/c_voda_main.css \
	css/arctransform.css
#
# top/left nav menu
PAGES+=	setup_top.htm menu.htm

#
# login
#
PAGES+=	login.htm loginduperr.htm loginpserr.htm

#
# Advance configure
#
PAGES+=	lan.htm \
	lan_dhcp_static.htm \
	lan1.htm \
	advance.htm \
	r_mort.htm \
	route.htm \
	status.htm \
	system_f.htm \
	system_main.htm \
	system_p.htm \
	system_r.htm \
	system_remote_mgmt.htm \
	system_t.htm \
	security_log.htm \
	upnp_main.htm \
	wan_mode.htm \
	wan_eth.htm
	wan_bridge.htm
	wan_multiple.htm \
	wan_multiple_eth.htm
	wan_multiple_adsl.htm
	wan_clone.htm \
	ddns_main.htm \
	firewall_a.htm \
	firewall_d.htm \
	firewall_mac.htm \
	firewall_main.htm \
	firewall_rule.htm \
	firewall_spi_h.htm \
	firewall_u.htm \
	firewall_l2_mac.htm \
	nat.htm \
	nat_show.htm \
	nat_sp.htm \
	nat_v.htm

# firewall_rule_a.htm firewall_a_ed.htm nat_m.htm 

# #############
# WiFi Pages
# #############
ifdef PROJ_UI_WIFI
PAGES+= ssid24g.htm \
	wireless_advance.htm \
	wireless_calibration.htm \
	wireless_e.htm \
	wireless_id.htm \
	wireless_main.htm \
	wireless_WPS.htm \
	wps_showInit.htm \
	wps_showProcess.htm \
	WPS_stautsShow.htm \
	wireless_ap_cli
ifdef PROJ_UI_WLAN_MAC
PAGES+= wireless_mac.htm
endif

#ifdef PROJ_UI_MULTI_SSID
#PAGES+= ssid2.htm \
#	ssid3.htm \
#	ssid4.htm
#endif


# #############
# wifi 5G
# #############
ifdef PROJ_UI_5G
PAGES+= ssid5g.htm
endif

# #############
# wifi WDS
# #############
ifdef PROJ_UI_WDS
PAGES+= wireless_wds.htm
endif

endif

# #############
# Static Route
# #############
ifdef PROJ_UI_STATIC_RT
PAGES+= route_add.htm route_tbl.htm
endif

# #############
# RIP
# #############
ifdef PROJ_UI_RIP
PAGES+= r_rip.htm
endif

# #############
# QoS
# #############
ifdef PROJ_UI_QOS
PAGES+= qos_addrule.htm \
	qos_main.htm \
	qos_traffic_map.htm \
	qos_wifi.htm
endif

# #############
# USB
# #############
ifdef PROJ_UI_USB
PAGES+= usb.htm nas_addUser.htm
endif

# #############
# IPv6
# #############
ifdef PROJ_UI_IPV6
PAGES+= ipv6.htm
endif

# #############
# LTE
# #############
ifdef PROJ_UI_LTE
PAGES+= hspa_modem.htm lte_main.htm
endif

# #############
# SIP
# #############
ifdef PROJ_UI_SIP_PROXY
PAGES+= sip.htm \
	sip_call_log.htm \
	sip_ex_account.htm \
	sip_ex_account_add.htm \
	sip_in_account.htm \
	sip_in_account_add.htm \
	sip_in_mapping.htm \
	sip_out_mapping.htm \
	sip_proxy.htm \
	sip_status.htm
endif

# #############
# Vocie
# #############
ifdef PROJ_UI_VOICE
PAGES+= voice_add_plan.htm \
	voice_add_sip.htm \
	voice_add_umts.htm \
	voice_connection.htm \
	voice_in.htm \
	voice_main.htm \
	voice_out.htm \
	voice_phone.htm \
	voice_plan.htm \
	voice_status.htm
endif

# #############
# TR69
# #############
ifdef PROJ_UI_TR69
PAGES+= tr69.htm tr69dbg.htm
endif

# #############
# ADSL
# #############
ifdef PROJ_UI_ADSL
PAGES+= adsl.htm
endif

# #############
# NAT ALG
# #############
ifdef PROJ_UI_NAT_ALG
PAGES+= nat_alg.htm
endif

# #############
# Diagnostic Tools
# #############
ifdef PROJ_UI_DIAGNOSTIC
PAGES+= tools_ping_tr.htm \
	tools_wan_diag.htm
endif

#
# #############
# hidden
# #############
#
PAGES+= hidden.htm \
	wireless_wmm.htm

#
# CGI
#
PAGES+= cgi/cgi_lan.js \
	cgi/cgi_lan_dhcp_static.js \
	cgi/cgi_clients.js \
	cgi/cgi_wan_eth.js \
	cgi/cgi_wan_bridge.js \
	cgi/cgi_wan_mode.js \
	cgi/cgi_wan_multiple.js \
	cgi/cgi_wanmac.js \
	cgi/cgi_ddns_main.js \
	cgi/cgi_firewall_a.js \
	cgi/cgi_firewall_d.js \
	cgi/cgi_firewall_mac.js \
	cgi/cgi_firewall_main.js \
	cgi/cgi_firewall_rule.js \
	cgi/cgi_firewall_rule_a.js \
	cgi/cgi_firewall_spi_h.js \
	cgi/cgi_firewall_u.js \
	cgi/cgi_firewall_l2_mac.js \
	cgi/cgi_nat.js \
	cgi/cgi_nat_show.js \
	cgi/cgi_nat_sp.js \
	cgi/cgi_nat_v.js \
	cgi/cgi_ntp.js \
	cgi/cgi_r_mort.js \
	cgi/cgi_security_log.js \
	cgi/cgi_st.js \
	cgi/cgi_status.js \
	cgi/cgi_sys_bk.js \
	cgi/cgi_sys_p.js \
	cgi/cgi_sys_r.js \
	cgi/cgi_system_info.js \
	cgi/cgi_upnp_main.js \
	cgi/cgi_from.js \
	cgi/cgi_init.js\
	cgi/cgi_login.js

#	cgi/cgi_firewall_a_ed.js  2013/12/18: drop
# cgi/cgi_nat_m.js
# cgi/cgi_sys.js
# cgi/cgi_dns.js no work

# #############
# WiFi CGI
# #############
ifdef PROJ_UI_WIFI
PAGES+=	cgi/cgi_wifi_e.js \
	cgi/cgi_wifi_id.js \
	cgi/cgi_wifi_mac.js \
	cgi/cgi_wifi_main.js \
	cgi/cgi_wifi_24g.js \
	cgi/cgi_wireless_wps.js \
	cgi/cgi_wps_statusShow.js \
	cgi/cgi_wifi_ap_cli.js 
	
endif
# #############
# Static Route CGI
# #############
ifdef PROJ_UI_WIFI
PAGES+=	cgi/cgi_route_tbl.js

endif
# #############
# RIP CGI
# #############
ifdef PROJ_UI_RIP
PAGES+=	cgi/cgi_r_rip.js
endif

# #############
# QoS CGI
# #############
ifdef PROJ_UI_QOS
PAGES+= cgi/cgi_qos_addrule.js \
	cgi/cgi_qos_main.js \
	cgi/cgi_qos_traffic_map.js \
	cgi/cgi_qos_wifi.js
endif
# #############
# USB CGI
# #############
ifdef PROJ_UI_USB
PAGES+=	cgi/cgi_usb.js \
	cgi/cgi_usb_basic.js \
	cgi/cgi_sharedFolder.js \
	cgi/cgi_nas_admin.js \
	cgi/cgi_disk.js
endif

# #############
# IPv6
# #############
ifdef PROJ_UI_IPV6
PAGES+= cgi/cgi_ipv6.js
endif

# #############
# LTE
# #############
ifdef PROJ_UI_LTE
PAGES+= cgi/cgi_hspa.js \
	cgi/cgi_lte_main.js
endif

	
# #############
# SIP
# #############
ifdef PROJ_UI_SIP_PROXY
PAGES+= cgi/cgi_sip.js \
	cgi/cgi_sip_call_log.js \
	cgi/cgi_sip_ex_account.js \
	cgi/cgi_sip_in_account.js \
	cgi/cgi_sip_in_mapping.js \
	cgi/cgi_sip_log.js \
	cgi/cgi_sip_out_mapping.js \
	cgi/cgi_sip_st.js
endif

# #############
# Vocie
# #############
ifdef PROJ_UI_VOICE
PAGES+= cgi/cgi_voice_connection.js \
	cgi/cgi_voice_in_out.js \
	cgi/cgi_voice_phone.js \
	cgi/cgi_voice_plan.js \
	cgi/cgi_voice_status.js
endif

# #############
# TR69 CGI
# #############
ifdef PROJ_UI_TR69
PAGES+= cgi/cgi_tr69.js \
	cgi/cgi_tr69dbg.js
endif

# #############
# ADSL CGI
# #############
ifdef PROJ_UI_ADSL
PAGES+= cgi/cgi_adsl.js
endif

# #############
# NAT ALG CGI
# #############
ifdef PROJ_UI_NAT_ALG
PAGES+= cgi/cgi_nat_alg.js
endif

#
# #############
# hidden
# #############
#
PAGES+= cgi/cgi_hidden.js \
	cgi/cgi_wifi_wmm.js

# ?3G
# cgi/cgi_3g.js
# cgi/cgi_3g_st.js
# cgi/cgi_dsl.js
# cgi/cgi_dsl_status.js
# cgi/cgi_rt_ststus_var_list.js
# cgi/cgi_atmint.js
# cgi/cgi_atmpvc.js
# cgi/sysinfo.js

# #######################################
# Multi-language support
# #######################################
# we convert to lower case
UI_LANG_LST=$(shell echo $(PROJ_UI_LANG_LST) | tr A-Z a-z)
# include those packets
PAGES+=$(shell for L in $(UI_LANG_LST); do \
	    echo "lang/$$L/$$L""_menulist.js \
		  lang/$$L/$$L""_help.js \
		  lang/$$L/$$L""_login.js \
		  lang/$$L/$$L""_top.js \
		  lang/$$L/$$L""_advance.js \
		  lang/$$L/$$L""_lan_main.js \
		  lang/$$L/$$L""_dns.js \
		  lang/$$L/$$L""_firewall_a.js \
		  lang/$$L/$$L""_firewall_a_ed.js \
		  lang/$$L/$$L""_firewall_d.js \
		  lang/$$L/$$L""_firewall_mac.js \
		  lang/$$L/$$L""_firewall_main.js \
		  lang/$$L/$$L""_firewall_rule.js \
		  lang/$$L/$$L""_firewall_rule_a.js \
		  lang/$$L/$$L""_firewall_spi_h.js \
		  lang/$$L/$$L""_firewall_u.js \
		  lang/$$L/$$L""_nat.js \
		  lang/$$L/$$L""_nat_show.js \
		  lang/$$L/$$L""_nat_sp.js \
		  lang/$$L/$$L""_nat_v.js \
		  lang/$$L/$$L""_route.js \
		  lang/$$L/$$L""_r_mort.js \
		  lang/$$L/$$L""_status.js \
		  lang/$$L/$$L""_tl.js \
		  lang/$$L/$$L""_tl_fc.js \
		  lang/$$L/$$L""_tl_rst.js \
		  lang/$$L/$$L""_tl_sys.js \
		  lang/$$L/$$L""_tl_sys_c.js \
		  lang/$$L/$$L""_tl_sys_f.js \
		  lang/$$L/$$L""_tl_sys_fw.js \
		  lang/$$L/$$L""_tl_sys_r.js \
		  lang/$$L/$$L""_upnp_main.js \
		  lang/$$L/$$L""_wan.js \
		  lang/$$L/$$L""_wan_main.js \
		  lang/$$L/$$L""_ddns_main.js"; \
	done)

# #############
# WAN MAC clone
# #############
ifdef PROJ_WAN_MAC_CLONE
PAGES+=$(shell for L in $(UI_LANG_LST); do \
	    echo "lang/$$L/$$L""_wan_mac.js"; \
	done)
endif

# lang/$$L/$$L""_nat_m.js (no CFG)
# #############
# WiFi Pages LANGUAGE
# #############
ifdef PROJ_UI_WIFI
PAGES+=$(shell for L in $(UI_LANG_LST); do \
	    echo "lang/$$L/$$L""_wifi_e.js \
		  lang/$$L/$$L""_wifi_id.js \
		  lang/$$L/$$L""_wifi_mac.js \
		  lang/$$L/$$L""_wifi_main.js \
		  lang/$$L/$$L""_wifi_wep.js \
		  lang/$$L/$$L""_wireless_1X.js \
		  lang/$$L/$$L""_wireless_advance.js \
		  lang/$$L/$$L""_wireless_wpa.js \
		  lang/$$L/$$L""_wireless_WPS.js \
		  lang/$$L/$$L""_wps_showProcess.js \
		  lang/$$L/$$L""_wifi_ap_cli.js"; \
		  lang/$$L/$$L""_wifi_wmm.js"; \
	done)
endif
# #############
# Static Route LANGUAGE
# #############
ifdef PROJ_UI_STATIC_RT
PAGES+=$(shell for L in $(UI_LANG_LST); do \
	    echo "lang/$$L/$$L""_route_tbl.js"; \
	done)
endif

# #############
# RIP LANGUAGE
# #############
ifdef PROJ_UI_RIP
PAGES+=$(shell for L in $(UI_LANG_LST); do \
	    echo "lang/$$L/$$L""_r_rip.js"; \
	done)
endif
# #############
# QoS LANGUAGE
# #############
ifdef PROJ_UI_QOS
PAGES+=$(shell for L in $(UI_LANG_LST); do \
	    echo "lang/$$L/$$L""_qos_addrule.js \
		  lang/$$L/$$L""_qos_main.js \
		  lang/$$L/$$L""_qos_traffic_map.js \
		  lang/$$L/$$L""_qos_wifi.js"; \
	done)
endif

# #############
# IPv6  LANGUAGE
# #############
ifdef PROJ_UI_IPV6
PAGES+=$(shell for L in $(UI_LANG_LST); do \
	    echo "lang/$$L/$$L""_ipv6.js"; \
	done)
endif
# #############
# LTE LANGUAGE
# #############
ifdef PROJ_UI_LTE
PAGES+=$(shell for L in $(UI_LANG_LST); do \
	    echo "lang/$$L/$$L""_lte.js \
		  lang/$$L/$$L""_lte_main.js \
		  lang/$$L/$$L""_hspa.js"; \
	done)

endif

# #############
# USB LANGUAGE
# #############
ifdef PROJ_UI_USB
PAGES+=$(shell for L in $(UI_LANG_LST); do \
	    echo "lang/$$L/$$L""_usb.js \
		  lang/$$L/$$L""_nas_admin.js"; \
	done)

endif

# #############
# SIP LANGUAGE
# #############
ifdef PROJ_UI_SIP_PROXY
PAGES+=$(shell for L in $(UI_LANG_LST); do \
	    echo "lang/$$L/$$L""_sip_proxy.js"; \
	done)
endif

# #############
# Vocie LANGUAGE
# #############
ifdef PROJ_UI_VOICE
PAGES+=$(shell for L in $(UI_LANG_LST); do \
	    echo "lang/$$L/$$L""_voice_connection.js \
		  lang/$$L/$$L""_voice_in_out.js \
		  lang/$$L/$$L""_voice_main.js \
		  lang/$$L/$$L""_voice_phone.js \
		  lang/$$L/$$L""_voice_plan.js \
		  lang/$$L/$$L""_voice_status.js"; \
	done)
endif


# #############
# TR69 LANGUAGE
# #############
ifdef PROJ_UI_TR69
PAGES+=$(shell for L in $(UI_LANG_LST); do \
	    echo "lang/$$L/$$L""_tr69.js \
		  lang/$$L/$$L""_tr69dbg.js"; \
	done)
endif

# #############
# ADSL LANGUAGE
# #############
ifdef PROJ_UI_ADSL
PAGES+=$(shell for L in $(UI_LANG_LST); do \
	    echo "lang/$$L/$$L""_adsl.js"; \
	done)
endif

# #############
# NAT ALG LANGUAGE
# #############
ifdef PROJ_UI_NAT_ALG
PAGES+=$(shell for L in $(UI_LANG_LST); do \
	    echo "lang/$$L/$$L""_nat_alg.js"; \
	done)
endif

# #############
# Diagnostic Tools
# #############
ifdef PROJ_UI_DIAGNOSTIC
PAGES+=$(shell for L in $(UI_LANG_LST); do \
	    echo "lang/$$L/$$L""_tools_ping_tr.js \
		  lang/$$L/$$L""_tools_wan_diag.js"; \
	done)
endif

#  lang/$$L/$$L""_3g.js 
#  lang/$$L/$$L""_adsl.js 
#  lang/$$L/$$L""_dsl.js 


#
# images
#
PAGES+= images/arrow_right.png \
	images/arrow_up.png \
	images/background.jpg \
	images/lang.png \
	images/logout.png \
	images/logout_gray.png \
	images/menu_gra.png \
	images/menu_gra_hover.png \
	images/menu_sel.png \
	images/menu_shadow_right.png \
	images/nav_left.png \
	images/nav_left_sel.png \
	images/nav_middle.png \
	images/nav_middle_hover.png \
	images/nav_middle_sel.png \
	images/nav_right.png \
	images/nav_right_sel.png \
	images/shadow_bottom.png \
	images/shadow_left.png \
	images/shadow_right.png \
	images/wait.gif \
	images/signal_0.gif \
	images/signal_1.gif \
	images/signal_10.gif \
	images/signal_2.gif \
	images/signal_3.gif \
	images/signal_4.gif \
	images/signal_5.gif \
	images/signal_6.gif \
	images/signal_7.gif \
	images/signal_8.gif \
	images/signal_9.gif \
	images/space.png \
	images/theme1/btn_l.png \
	images/theme1/btn_l_b.png \
	images/theme1/btn_l_g.png \
	images/theme1/btn_l_gy.png \
	images/theme1/btn_l_o.png \
	images/theme1/btn_l_p.png \
	images/theme1/btn_l_pn.png \
	images/theme1/btn_m.png \
	images/theme1/btn_m_b.png \
	images/theme1/btn_m_g.png \
	images/theme1/btn_m_gy.png \
	images/theme1/btn_m_o.png \
	images/theme1/btn_m_p.png \
	images/theme1/btn_m_pn.png \
	images/theme1/btn_r.png \
	images/theme1/btn_r_b.png \
	images/theme1/btn_r_g.png \
	images/theme1/btn_r_gy.png \
	images/theme1/btn_r_o.png \
	images/theme1/btn_r_p.png \
	images/theme1/btn_r_pn.png \
	images/theme1/checkbox_big.png \
	images/theme1/input_text_left.png \
	images/theme1/input_text_right.png \
	images/theme1/radio_big.png \
	images/theme1/select_left.png \
	images/theme1/select_right.png \
	images/theme1/textarea_bl.gif \
	images/theme1/textarea_bm.gif \
	images/theme1/textarea_br.gif \
	images/theme1/textarea_ml.gif \
	images/theme1/textarea_mr.gif \
	images/theme1/textarea_tl.gif \
	images/theme1/textarea_tm.gif \
	images/theme1/textarea_tr.gif \
	images/theme1/textarea-mm.gif \
	images/theme1/textarea-mm-focus.gif \
	images/theme1/textarea-mm-hover.gif \
 	images/icon/arrow_d2.png \
 	images/icon/arrow_d.png \
 	images/icon/arrow_u2.png \
 	images/icon/arrow_u.png \
 	images/icon/arrow_r2.png \
 	images/icon/arrow_r.png \
 	images/icon/arrow_l2.png \
 	images/icon/arrow_l.png


# 	images/folder/browse.gif \
# 	images/folder/disk.gif \
# 	images/folder/folder1.gif \
# 	images/folder/folderb.gif \
# 	images/folder/folderopen1.gif \
# 	images/folder/join.gif \
# 	images/folder/joinbottom.gif \
# 	images/folder/line.gif \
# 	images/folder/minus.gif \
# 	images/folder/minusbottom.gif \
# 	images/folder/plus.gif \
# 	images/folder/plusbottom.gif \
# 	images/folder/text.gif \
# 	images/folder/tv_folder.gif \
# 	images/folder/tv_folderopen.gif \
# 	images/folder/tv_hdisk.gif \
# 	images/folder/tv_margin.gif \
# 	images/folder/tv_minus.gif \
# 	images/folder/tv_plus.gif \
# 	images/folder/tv_space.gif \
# 	images/icon/arrow_d2.png \
# 	images/icon/arrow_d.png \
# 	images/icon/arrow_u2.png \
# 	images/icon/arrow_u.png \
# 	images/icon/chk \
# 	images/icon/copy.png \
# 	images/icon/down.png \
# 	images/icon/edit2.png \
# 	images/icon/edit3.png \
# 	images/icon/edit.png \
# 	images/icon/globe.png \
# 	images/icon/signin.png \
# 	images/icon/signout.png \
# 	images/icon/trash.png \
# 	images/icon/up.png \
#




