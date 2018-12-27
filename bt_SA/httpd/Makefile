# 
# Copyright (C) 2011 Arcadyan
#
#

include $(TOPDIR)/rules.mk

PKG_NAME:=httpd
PKG_REV:=master
PKG_VERSION:=0.1
PKG_RELEASE:=1

#PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL:=git://192.168.106.197/httpd
PKG_MAINTAINER:=linghong_tan@sva-arcadyan.com.cn

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
PKG_BUILD_DEPENDS:=arcadyan-utility lib-arc-cfg arc-middle-mod-ccfgsal lib-arc-com

EX_PKG_DEPENDS:=

PROJECT_NAME:=$(shell grep "CONFIG_WEBJSMIN_PROJ_CGI_VENDOR_INCLUDE" $(TOPDIR)/.config | cut -d '=' -f 2)

# Add additional dependes in zz project
#ifeq ($(PROJECT_NAME), "zz")
#PKG_BUILD_DEPENDS+= arc-middle-mod-aunet arc-middle-mod-voip sip-brn

#SIP_PKG_NAME=$(shell grep "CONFIG_PACKAGE_sip-brn" $(TOPDIR)/.config | grep "=y" |sed 's/=y//g' | awk 'BEGIN{FS="_";} {print $$3}' )

#ifneq ($(SIP_PKG_NAME),)
#PKG_BUILD_DEPENDS+= $(SIP_PKG_NAME)
#endif

#endif # PROJECT_NAME zz

MKDIR:=mkdir -p
RM:=rm -rf

include $(INCLUDE_DIR)/package.mk

ifeq ($(__target_inc),)
  include $(INCLUDE_DIR)/target.mk
endif

include $(TOPDIR)/.config

ifeq ($(CONFIG_PACKAGE_pc_list),y)
PKG_BUILD_DEPENDS+= pc_list
EX_PKG_DEPENDS+= +pc_list +lib-arc-com +libumng
ifeq ($(CONFIG_PACKAGE_arc-middle-mod-qcsapi), y)
EX_PKG_DEPENDS+= +arc-middle-mod-qcsapi
endif
endif

ifeq ($(CONFIG_PACKAGE_arc_syslog),y)
EX_PKG_DEPENDS+= +arc_syslog
endif

EX_PKG_DEPENDS+= +arc-middle-mod-ccfgsal

ifeq ($(CONFIG_PACKAGE_broadcom-libwlvisualization),y)
EX_PKG_DEPENDS+= +broadcom-libwlbcmcrypto +broadcom-libwlbcmshared +broadcom-libwlvisualization
endif

define Package/httpd
  SUBMENU:=Network
  SECTION:=net
  CATEGORY:=Arcadyan
  TITLE:=Web server for Arcadyan projects
  URL:=http://www.arcadyan.com/
  DEPENDS:= +libpthread +arcadyan-utility +lib-arc-cfg +html_docs $(EX_PKG_DEPENDS)
endef

define Package/httpd/config
  menu "Httpd Configuration"
  depends on PACKAGE_httpd
  source "$(SOURCE)/config/Kconfig"
  endmenu
endef

#+FEATURE_ARCADYAN_arc-ssl-openssl:libopenssl \
#+FEATURE_ARCADYAN_arc-ssl-polarssl:libpolarssl

#define Feature/arc-ssl-openssl
  #TARGET_NAME:=ARCADYAN
  #TARGET_TITLE:=Arcadyan Middle Layer
  #NAME:=openssl
#endef

#define Feature/arc-ssl-polarssl
  #TARGET_NAME:=ARCADYAN
  #TARGET_TITLE:=Arcadyan Middle Layer
  #NAME:=polarssl
#endef

define Package/httpd/description
 The web server from Arcadyan Corporation for embedded Linux.
 The application is designed to support multiple projects.
endef

#define Package/httpd/config
	##source "$(SOURCE)/Config.in"
#endef

define Build/Configure
	- rm -f $(PKG_BUILD_DIR)/.configured*
	- grep 'CONFIG_ARCADYAN_' $(TOPDIR)/.config | sed -e "s,\\(# \)\\?ARCADYAN_\\(.*\\),\\1\\2,g" > $(PKG_BUILD_DIR)/.config
	- grep 'CONFIG_LINUX_2_6' $(TOPDIR)/.config >> $(PKG_BUILD_DIR)/.config
	- grep 'CONFIG_UBOOT_CONFIG_JFFS2_PART' $(TOPDIR)/.config >> $(PKG_BUILD_DIR)/.config
	- grep 'CONFIG_UBOOT_CONFIG_IN_SUPERTASK' $(TOPDIR)/.config >> $(PKG_BUILD_DIR)/.config
	- grep 'CONFIG_UBOOT_CONFIG_NAND_FLASH' $(TOPDIR)/.config >> $(PKG_BUILD_DIR)/.config
	- grep 'CONFIG_FW_XDSL_VDSL_SUPPORT' $(TOPDIR)/.config >> $(PKG_BUILD_DIR)/.config
	- grep 'CONFIG_HTTPD' $(TOPDIR)/.config >> $(PKG_BUILD_DIR)/.config
	grep 'CONFIG_WEBJSMIN_' $(TOPDIR)/.config | sed -e "s,\\(# \)\\?CONFIG_WEBJSMIN_\\(.*\\),\\1\\2,g" >> $(PKG_BUILD_DIR)/.config
	- grep 'CONFIG_PACKAGE_pc_list' $(TOPDIR)/.config >> $(PKG_BUILD_DIR)/.config
	- grep 'CONFIG_PACKAGE_arc_syslog' $(TOPDIR)/.config >> $(PKG_BUILD_DIR)/.config
	- grep 'CONFIG_PACKAGE_arc-middle-mod-qcsapi' $(TOPDIR)/.config >> $(PKG_BUILD_DIR)/.config
	- grep 'CONFIG_PACKAGE_broadcom-libwlvisualization' $(TOPDIR)/.config >> $(PKG_BUILD_DIR)/.config
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(call Build/Prepare/Default)
	if [ -d ./src ] ; then \
		$(CP) ./src/* $(PKG_BUILD_DIR)/. ; \
	fi
endef

define Build/Compile
	### Set 'build/$(SCAN_REPORT_DIR)/$(PKG_NAME)' as the output directory for analyzer reports if CLANG_ANALYZE == yes ###
	if [ "$(CLANG_ANALYZE)" = "yes" ] ; then \
		$(MKDIR) $(SCAN_REPORT_DIR)/$(PKG_NAME); \
		$(CLANG_BUILD) -o $(SCAN_REPORT_DIR)/$(PKG_NAME) $(MAKE) -C $(PKG_BUILD_DIR) CC="$(CLANG_CC)" CCC_CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS) -Wall -I$(STAGING_DIR)/usr/include" \
			CPPFLAGS="$(TARGET_CPPFLAGS)" LD="$(TARGET_CROSS)ld" LDFLAGS="$(TARGET_LDFLAGS) -lrt" \
			STRIP="$(TARGET_CROSS)strip" \
			DEV_DIR="$(STAGING_DIR)" ; \
	else \
	$(MAKE) -C $(PKG_BUILD_DIR) CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS) -Wall -I$(STAGING_DIR)/usr/include" \
		CPPFLAGS="$(TARGET_CPPFLAGS)" LD="$(TARGET_CROSS)ld" LDFLAGS="$(TARGET_LDFLAGS) -lrt"  \
		STRIP="$(TARGET_CROSS)strip" \
		DEV_DIR="$(STAGING_DIR)" ; \
	fi

	$(RM) $(PKG_INSTALL_DIR)
	$(MKDIR) $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) INSTALL_DIR="$(INSTALL_DIR)" INSTALL_BIN="$(INSTALL_BIN)" INSTALL_DATA="$(INSTALL_DATA)" \
		CP="$(CP)" CONFIG_PREFIX="$(PKG_INSTALL_DIR)"  DEV_DIR="$(STAGING_DIR)" install;
endef

define Build/InstallDev
endef

define Package/httpd/install
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

define Build/httpd/clean
        @- [ -e $(PKG_BUILD_DIR)/Makefile ] && $(MAKE) -C $(PKG_BUILD_DIR) clean
        $(call Build/Clean/Default)
endef

#$(eval $(call Feature,arc-ssl-openssl))
#$(eval $(call Feature,arc-ssl-polarssl))
$(eval $(call BuildPackage,httpd))
