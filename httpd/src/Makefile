# Makefile for httpd

TOP_DIR  := $(shell pwd)
-include $(TOP_DIR)/.config

PKG_NAME:=httpd
PKG_RELEASE:=1

VPATH=$(DEV_DIR)/usr/lib

## arc-ssl cover ssl implement
SSL_LIBS = 
ifeq ($(CONFIG_arc-ssl), y)
SSL_LIBS += -ldl -larc-ssl
endif

DESTDIR =	$(CONFIG_PREFIX)
TARGET_WWW = target_www


BINDIR =	$(DESTDIR)/usr/sbin
#CC =		gcc
OFLAGS =	-O
HTTP_CFLAGS = $(CFLAGS) -I$(TOP_DIR)/include -I$(DEV_DIR)/usr/include
HTTP_CFLAGS += -g ${OFLAGS} -I.

HTTP_LDFLAGS =  $(LDFLAGS)
HTTP_LDFLAGS +=   -Wl -L$(DEV_DIR)/usr/lib			## -g

LDLIBS =	-lpthread -lmidcore -lmng ${SSL_LIBS} -larc-shared -lm -lmapi_ccfgsal

ifeq ($(CONFIG_PACKAGE_pc_list), y)
LDLIBS += -lArcCom -lmapi_common-net -lpclist -lumng
endif

ifeq ($(CONFIG_PACKAGE_arc_syslog), y)
LDLIBS += -larcsyslog
HTTP_CFLAGS += -D_SYSLOG
endif

ifeq ($(CONFIG_PACKAGE_arc-middle-mod-qcsapi), y)
LDLIBS += -lmapi_qcsapi 
endif

### features
ifeq ($(CONFIG_HNAP_SUPPORT), y)
LDLIBS +=	-lhnap
endif

ifeq ($(CONFIG_PACKAGE_broadcom-libwlvisualization),y)
HTTP_CFLAGS += -DCONFIG_WL_VISUALIZATION
LDLIBS += -L$(TOPDIR)/extern/target-install/lib -lwlvisualization -lwlbcmshared -lwlbcmcrypto -lnvram -lwlcsm
endif

OBJS:= access.o alias.o linux_fs.o get.o htconf.o ssi.o ssi_handler.o log.o post.o \
	   request.o script.o timer.o util.o weekday.o mime.o cgi.o upload.o

ifeq ($(CONFIG_HTTPD_TOKEN_CHECK_SUPPORT), y)
OBJS += httoken.o httoken_ihashtable.o
HTTP_CFLAGS += -DCONFIG_HTTPD_TOKEN_CHECK_SUPPORT
endif

ifeq ($(CONFIG_HTTPD_SUPPORT_TOKEN_DISABLE), y)
HTTP_CFLAGS += -DCONFIG_HTTPD_SUPPORT_TOKEN_DISABLE
endif

ifeq ($(CONFIG_HTTPD_SUPPORT_AES256), y)
OBJS += aes256cbc.o
HTTP_CFLAGS += -DCONFIG_HTTPD_SUPPORT_AES256
endif

ifeq ($(CONFIG_HTTPD_HTTPS_SUPPORT), y)
HTTP_CFLAGS += -DCONFIG_HTTPD_HTTPS_SUPPORT
endif

ifeq ($(CONFIG_HTTPD_Security_Attack_Protect), y)
HTTP_CFLAGS += -DCONFIG_HTTPD_Security_Attack_Protect
endif

ifeq ($(PROJ_UI_LOGIN_USR), y)
HTTP_CFLAGS += -DHTTPD_SUPPORT_USERNAME
endif

ifeq ($(CONFIG_NO_INTERNET_REDIRECT_SUPPORT), y)
HTTP_CFLAGS += -DCONFIG_NO_INTERNET_REDIRECT_SUPPORT
endif
HDR_FILES:= httpd.h timer.h fs.h util.h cgi.h ssi_files_list.h

### vendor-specified
ifeq ($(PROJ_CGI_VENDOR_INCLUDE),)
VENDOR:= $(shell cat $(TOP_DIR)/.config |grep CONFIG_TARGET |grep =y |awk 'BEGIN{FS="=";} {print $$1}' \
	| awk 'BEGIN{FS="_";} {print $$3}')
PROJ_INCLUDE:=n
else
PROJ_INCLUDE:=y
VENDOR:=$(shell echo $(PROJ_CGI_VENDOR_INCLUDE))
endif

MODEL:= $(shell cat $(TOP_DIR)/.config |grep CONFIG_TARGET |grep =y |awk 'BEGIN{FS="=";} {print $$1}' \
	| awk 'BEGIN{FS="_";} {print $$4}')

#
PROJ_WEB_CONFIG:=$(TOPDIR)/config/$(MODEL)/.config_web


ifeq ($(CONFIG_HTTPD_SUPPORT_LOGIN_SHA512), y)
OBJS += sha512.o
HTTP_CFLAGS += -DCONFIG_HTTPD_SUPPORT_LOGIN_SHA512
endif

ifeq (${PROJ_WEB_CONFIG}, $(wildcard ${PROJ_WEB_CONFIG}))
-include $(PROJ_WEB_CONFIG)
endif

# if no found any define, use old way (e.g.: VENDOR)
ifeq ($(CONFIG_HTTPD_HOT_FIX_SW1),y)
#
# hugh 2014/1/22 we dirty set under config_web under project-based
#  hard code here to fix password fail and incorrect upgrade/restire file
#
# hugh 2014/1/24: we need dirty add CONFIG_HTTPD_LTE_UPGRADE, may waiting project-based build option is available for HTTPD
#                 then move those out!
HTTP_CFLAGS += -DCONFIG_HTTPD_HOT_FIX_SW1=y -DCONFIG_HTTPD_LTE_UPGRADE=y -DHTTPD_TOKEN_CHECK_SUPPORT=y -DHTTPD_SUPPORT_TOKEN_DISABLE=y
endif

# Velmurugan 05292015> For ASUS DSL-AC87U & ZZ
ifeq ($(CONFIG_TARGET_asus_VRV9510RWAC34-1-B-ASU), y)
HTTP_CFLAGS+= -DVRV9510RWAC
endif

ifeq ($(CONFIG_TARGET_zz_VRV9510RWAC34-1-B-ZZ), y)
HTTP_CFLAGS+= -DVRV9510RWAC
endif

ifeq ($(CONFIG_TARGET_telstra_VGV9519AWAC23-A-TA), y)
HTTP_CFLAGS+= -DVRV9510RWAC
endif

ifeq ($(CONFIG_TARGET_telstra_VGV951ARWAC34-A-TA), y)
HTTP_CFLAGS+= -DVRV9510RWAC
endif

ifeq ($(CONFIG_TARGET_telstra_VR7516QW22-A-TA), y)
HTTP_CFLAGS+= -DVR7516QW22
endif

ifeq ($(CONFIG_TARGET_telmex_VR7516RW22-A-TE), y)
HTTP_CFLAGS+= -DVR7516RW22
endif

ifeq ($(CONFIG_TARGET_zz_WE410443-ZZ), y)
HTTP_CFLAGS+= -DWE410443
HTTP_CFLAGS+= -DWE410443_ZZ
HTTP_CFLAGS+= -DCHECK_LOGIN_BROWSER_SUPPORT
endif

ifeq ($(CONFIG_TARGET_pldt_VR9517PAC22-A-PP), y)
#same as other project based on MTK platform
HTTP_CFLAGS+= -DVR7516RW22
HTTP_CFLAGS+= -DVR9517PAC22_A_PP
endif

ifeq ($(CONFIG_TARGET_pldt_AR7516AOW22-A-PP), y)
#same as other project based on MTK platform
HTTP_CFLAGS+= -DVR7516RW22
HTTP_CFLAGS+= -DAR7516AOW22_A_PP
endif

ifeq ($(CONFIG_TARGET_bt_WE410443-SA), y)
HTTP_CFLAGS+= -DWE410443
HTTP_CFLAGS+= -DWE410443_SA
HTTP_CFLAGS+= -DCHECK_LOGIN_BROWSER_SUPPORT
endif

ifeq ($(CONFIG_TARGET_bt_WE5202243-SA), y)
HTTP_CFLAGS+= -DWE410443
HTTP_CFLAGS+= -DWE5202243_SA
HTTP_CFLAGS+= -DCHECK_LOGIN_BROWSER_SUPPORT
endif

ifeq ($(CONFIG_TARGET_bt_WE410443-6DX), y)
HTTP_CFLAGS+= -DWE410443
HTTP_CFLAGS+= -DWE410443_6DX
HTTP_CFLAGS+= -DCHECK_LOGIN_BROWSER_SUPPORT
endif

ifeq ($(CONFIG_TARGET_bt_WE410223-SA), y)
HTTP_CFLAGS+= -DWE410443
HTTP_CFLAGS+= -DWE410223_SA
HTTP_CFLAGS+= -DCHECK_LOGIN_BROWSER_SUPPORT
endif

ifeq ($(CONFIG_TARGET_ts_WE410443-TS), y)
HTTP_CFLAGS+= -DWE410443
HTTP_CFLAGS+= -DWE410443_TS
HTTP_CFLAGS+= -DCHECK_LOGIN_BROWSER_SUPPORT
endif

ifeq ($(CONFIG_TARGET_ta_WE410443-TA), y)
HTTP_CFLAGS+= -DWE410443
HTTP_CFLAGS+= -DWE410443_TA
HTTP_CFLAGS+= -DCHECK_LOGIN_BROWSER_SUPPORT
endif

ifeq ($(CONFIG_TARGET_a1_WE410443-A1), y)
HTTP_CFLAGS+= -DWE410443
HTTP_CFLAGS+= -DWE410443_A1
HTTP_CFLAGS+= -DCHECK_LOGIN_BROWSER_SUPPORT
endif

## default ##
ifeq ($(VENDOR),)
VENDOR:=arcadyan
endif

## default ##
ifeq ($(MODEL),)
MODEL:=generic
endif

ifeq ($(CONFIG_NN3_0),y)
LANG_OBJS=
SOURCE_WWW=www_NN3.0
else
SOURCE_WWW=www
LANG_OBJS += $(VENDOR)/lang_cn.o $(VENDOR)/lang_dutch.o $(VENDOR)/lang_eng.o $(VENDOR)/lang_french.o $(VENDOR)/lang_german.o \
		$(VENDOR)/lang_italian.o $(VENDOR)/lang_jap.o $(VENDOR)/lang_korea.o $(VENDOR)/lang_span.o $(VENDOR)/lang_tw.o
endif

ifeq ($(VENDOR), belkin)
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/ssi_qos_handler.o
OBJS += $(LANG_OBJS)
OBJS += $(VENDOR)/md5.o $(VENDOR)/autofw.o $(VENDOR)/aeskeywrap.o $(VENDOR)/prf.o $(VENDOR)/sha1.o \
		$(VENDOR)/rijndael-alg-fst.o $(VENDOR)/belkin_config_handlers.o

HDR_FILES += $(VENDOR)/opencrypto.h $(VENDOR)/aes.h $(VENDOR)/aeskeywrap.h $(VENDOR)/belkin_config_handlers.h $(VENDOR)/prf.h $(VENDOR)/rijndael-alg-fst.h $(VENDOR)/sha1.h
LDLIBS += -lArcCom -larccfg

else ifeq ($(VENDOR), linksys)
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/ssi_log_handler.o 
HDR_FILES += 
LDLIBS += -lArcCom -larccfg

else ifeq ($(VENDOR), reliance)
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/md5.o
HDR_FILES += 
LDLIBS += -lArcCom -larccfg

else ifeq ($(VENDOR), echostar)
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/md5.o $(VENDOR)/cJSON.o $(VENDOR)/vwap_api_json.o
HDR_FILES +=
LDLIBS += -lArcCom -larccfg

else ifeq ($(VENDOR), quanta)
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/ssi_qos_handler.o
OBJS += $(LANG_OBJS)
OBJS += $(VENDOR)/md5.o $(VENDOR)/autofw.o $(VENDOR)/aeskeywrap.o $(VENDOR)/prf.o $(VENDOR)/sha1.o \
		$(VENDOR)/rijndael-alg-fst.o $(VENDOR)/quanta_config_handlers.o
HDR_FILES += $(VENDOR)/opencrypto.h $(VENDOR)/aes.h $(VENDOR)/aeskeywrap.h $(VENDOR)/quanta_config_handlers.h $(VENDOR)/prf.h $(VENDOR)/rijndael-alg-fst.h $(VENDOR)/sha1.h
LDLIBS += -lArcCom -larccfg

else ifeq ($(VENDOR), zz)
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/md5.o
LDLIBS += -larccfg 
HTTP_CFLAGS += -DCONFIG_HTTPD_PLC_UPGRADE=y
HTTP_CFLAGS += -DCONFIG_HTTPD_ZZ_UPGRADE=y

else ifeq ($(VENDOR), asus)
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/md5.o
LDLIBS += -larccfg
HTTP_CFLAGS += -DCONFIG_HTTPD_PLC_UPGRADE=y
HTTP_CFLAGS += -DCONFIG_HTTPD_ZZ_UPGRADE=y

else ifeq ($(VENDOR), plc)
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/md5.o
LDLIBS += -larccfg 
HTTP_CFLAGS += -DCONFIG_HTTPD_PLC_UPGRADE=y


else ifeq ($(VENDOR), buffalo)
OBJS += $(VENDOR)/$(MODEL)/vendor_hook.o $(VENDOR)/$(MODEL)/vendor_cgi.o $(VENDOR)/$(MODEL)/vendor_ssi_handler.o $(VENDOR)/$(MODEL)/md5.o $(VENDOR)/$(MODEL)/CryptExec.o $(VENDOR)/$(MODEL)/blowfish.o
LDLIBS += -lArcCom -larccfg
#HTTP_CFLAGS += -I$(DEV_DIR)/usr/include/voip
HTTP_CFLAGS += -I$(VENDOR)/$(MODEL)

else ifeq ($(VENDOR), dt)
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/md5.o
HDR_FILES += 
LDLIBS += -lArcCom -larccfg

else ifeq ($(VENDOR), arcadyan)		### generic
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/md5.o
HDR_FILES += 
LDLIBS += -lArcCom -larccfg

else ifeq ($(VENDOR), singtel)
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/md5.o
LDLIBS += -larccfg
HTTP_CFLAGS += -DCONFIG_HTTPD_PLC_UPGRADE=y
HTTP_CFLAGS += -DCONFIG_HTTPD_ZZ_UPGRADE=y

else ifeq ($(VENDOR), telstra)
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/md5.o
LDLIBS += -larccfg
HTTP_CFLAGS += -DCONFIG_HTTPD_PLC_UPGRADE=y
HTTP_CFLAGS += -DCONFIG_HTTPD_ZZ_UPGRADE=y

else ifeq ($(VENDOR), telstraP)
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/md5.o
LDLIBS += -larccfg
HTTP_CFLAGS += -DCONFIG_HTTPD_PLC_UPGRADE=y
HTTP_CFLAGS += -DCONFIG_HTTPD_ZZ_UPGRADE=y

else ifeq ($(VENDOR), telmex)
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/md5.o
LDLIBS += -larccfg
HTTP_CFLAGS += -DCONFIG_HTTPD_PLC_UPGRADE=y
HTTP_CFLAGS += -DCONFIG_HTTPD_ZZ_UPGRADE=y

else ifeq ($(VENDOR), pldt)
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/md5.o
LDLIBS += -larccfg
HTTP_CFLAGS += -DCONFIG_HTTPD_PLC_UPGRADE=y
HTTP_CFLAGS += -DCONFIG_HTTPD_ZZ_UPGRADE=y

else ifeq ($(VENDOR), ap)
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/md5.o
LDLIBS += -larccfg
HTTP_CFLAGS += -DCONFIG_HTTPD_PLC_UPGRADE=y
HTTP_CFLAGS += -DCONFIG_HTTPD_ZZ_UPGRADE=y
HTTP_CFLAGS += -DCONFIG_HTTPD_MTK_UPGRADE=y

else ifeq ($(VENDOR), bt)
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/md5.o
LDLIBS += -larccfg
HTTP_CFLAGS += -DCONFIG_HTTPD_PLC_UPGRADE=y
HTTP_CFLAGS += -DCONFIG_HTTPD_ZZ_UPGRADE=y
HTTP_CFLAGS += -DCONFIG_HTTPD_MTK_UPGRADE=y

else ifeq ($(VENDOR), ts)
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/md5.o
LDLIBS += -larccfg
HTTP_CFLAGS += -DCONFIG_HTTPD_PLC_UPGRADE=y
HTTP_CFLAGS += -DCONFIG_HTTPD_ZZ_UPGRADE=y
HTTP_CFLAGS += -DCONFIG_HTTPD_MTK_UPGRADE=y

else ifeq ($(VENDOR), ta)
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/md5.o
LDLIBS += -larccfg
HTTP_CFLAGS += -DCONFIG_HTTPD_PLC_UPGRADE=y
HTTP_CFLAGS += -DCONFIG_HTTPD_ZZ_UPGRADE=y
HTTP_CFLAGS += -DCONFIG_HTTPD_MTK_UPGRADE=y

else ifeq ($(VENDOR), a1)
OBJS += $(VENDOR)/vendor_hook.o $(VENDOR)/vendor_cgi.o $(VENDOR)/vendor_ssi_handler.o $(VENDOR)/md5.o
LDLIBS += -larccfg
HTTP_CFLAGS += -DCONFIG_HTTPD_PLC_UPGRADE=y
HTTP_CFLAGS += -DCONFIG_HTTPD_ZZ_UPGRADE=y
HTTP_CFLAGS += -DCONFIG_HTTPD_MTK_UPGRADE=y

endif

## 2015-0623, Added by Vergil Wei.
ifeq ($(PROJ_UI_SIP), y)
	OBJS += $(VENDOR)/ssi_voip_handler.o
else ifeq ($(PROJ_UI_VOICE),y)
	OBJS += $(VENDOR)/ssi_voip_handler.o
else ifeq ($(PROJ_UI_SIP_PROXY),y)
	OBJS += $(VENDOR)/ssi_voip_handler.o
endif

all:	httpd CGI

httpd:	httpd.o ${OBJS}
	${CC} ${HTTP_CFLAGS} ${HTTP_LDFLAGS} ${LDLIBS} -Wl,-Map,httpd.map -o $@ $^

${OBJS}: ${HDR_FILES}
%.o: %.c
	$(CC) $(HTTP_CFLAGS) -c $< -o $@

CGI:
ifeq ($(CONFIG_EXTERN_CGI_SUPPORT), y)
	$(MAKE) -C $(VENDOR)/cgi-src/
endif

ssi_files_list.h:
	#./create_ssi_file_list.sh $(TOP_DIR)/vendor/$(VENDOR)/$(MODEL)/rootfs/$(SOURCE_WWW)/ $@

	## FIXME: linghong_tan 2013-11-07
	## Ugly. Since www exist in arcadyan-utility, we have to locate arcadyan-utility before scanning. 
ifeq ($(VENDOR), ap)
	bash ./create_ssi_file_list.sh $(TOPDIR)/package/arcadyan-utility/html_docs/files/zz/$(MODEL)/www/ $@
else
	bash ./create_ssi_file_list.sh $(TOPDIR)/package/arcadyan-utility/html_docs/files/$(VENDOR)/$(MODEL)/www/ $@
endif

install:	all uninstall
	-mkdir -p ${BINDIR}
	cp httpd  ${BINDIR}

	
	#############
	echo $(CONFIG_PREFIX)
	# tools
	$(INSTALL_DIR)  $(CONFIG_PREFIX)/usr/sbin
	$(INSTALL_BIN)  httpd	$(CONFIG_PREFIX)/usr/sbin/
ifeq ($(VENDOR), buffalo)
	$(INSTALL_DATA) $(VENDOR)/*.pem	$(CONFIG_PREFIX)/usr/sbin/
else ifeq ($(VENDOR), bt)
	$(INSTALL_DATA) $(VENDOR)/httpd.crt	$(CONFIG_PREFIX)/usr/sbin/
	$(INSTALL_DATA) $(VENDOR)/httpd.key	$(CONFIG_PREFIX)/usr/sbin/
else ifeq ($(VENDOR), ts)
	$(INSTALL_DATA) $(VENDOR)/httpd.crt	$(CONFIG_PREFIX)/usr/sbin/
	$(INSTALL_DATA) $(VENDOR)/httpd.key	$(CONFIG_PREFIX)/usr/sbin/
else ifeq ($(VENDOR), ta)
	$(INSTALL_DATA) $(VENDOR)/httpd.crt	$(CONFIG_PREFIX)/usr/sbin/
	$(INSTALL_DATA) $(VENDOR)/httpd.key	$(CONFIG_PREFIX)/usr/sbin/
else ifeq ($(VENDOR), a1)
	$(INSTALL_DATA) $(VENDOR)/httpd.crt	$(CONFIG_PREFIX)/usr/sbin/
	$(INSTALL_DATA) $(VENDOR)/httpd.key	$(CONFIG_PREFIX)/usr/sbin/	
else ifeq ($(VENDOR), ap)
	$(INSTALL_DATA) $(VENDOR)/httpd.crt	$(CONFIG_PREFIX)/usr/sbin/
	$(INSTALL_DATA) $(VENDOR)/httpd.key	$(CONFIG_PREFIX)/usr/sbin/	
else
	$(INSTALL_DATA) *.pem	$(CONFIG_PREFIX)/usr/sbin/
endif

ifeq ($(CONFIG_EXTERN_CGI_SUPPORT), y)
	$(MAKE) -C $(VENDOR)/cgi-src HTTP_DIR="$(shell pwd)" install
	$(INSTALL_DIR)  $(CONFIG_PREFIX)/usr/sbin/cgi-bin
	$(INSTALL_BIN)	cgi-bin/*	$(CONFIG_PREFIX)/usr/sbin/cgi-bin/
endif

uninstall:
	rm -f ${BINDIR}/httpd

clean:
	rm -f httpd *.o core core.* *.core *.map $(VENDOR)/*.o

depend:

.PHONY: all clean depend install

