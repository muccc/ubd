#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 13844 2009-01-04 00:06:33Z lars $

include $(TOPDIR)/rules.mk

PKG_NAME:=v6test
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_SOURCE:=v6test-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/v6test-$(PKG_VERSION)
PKG_SOURCE_URL:=http://blinkenlichts.net/~schneider/
PKG_FIXUP:=libtool

include $(INCLUDE_DIR)/package.mk

#PKG_INSTALL=1

define Package/v6test
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+gnet2
  TITLE:=v6test 1.0
  URL:=http://www.gtk.org/
endef

define Package/v6test/description
  jhust v6test
endef

TARGET_CFLAGS+= \
	-I$(STAGING_DIR)/usr/lib/libiconv/include \
	-I$(STAGING_DIR)/usr/lib/libintl/include
#	$(FPIC) \

TARGET_LDFLAGS+= \
	-Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
	-L$(STAGING_DIR)/usr/lib/libiconv/lib \
	-L$(STAGING_DIR)/usr/lib/libintl/lib

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR)/ install DESTDIR=$(PKG_INSTALL_DIR)
endef

#define Build/Configure
#    $(call Build/Configure/Default)
#endef

define Package/v6test/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/* \
		$(1)/usr/bin
endef


$(eval $(call BuildPackage,v6test))
