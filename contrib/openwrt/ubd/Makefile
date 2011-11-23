# This is free software, licensed under s

include $(TOPDIR)/rules.mk

PKG_REV:=bd9081031dda7a61cbb6d18894e34f7f9847b61f
PKG_NAME:=ubd
PKG_VERSION:=0.2.0
PKG_RELEASE:=9

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

#PKG_SOURCE_PROTO:=git
#PKG_SOURCE_SUBDIR:=$(PKG_NAME)
#PKG_SOURCE_VERSION:=$(PKG_REV)
#PKG_SOURCE:=$(PKG_NAME)-$(PKG_REV).tar.gz
#PKG_SOURCE_URL:=https://github.com/schneider42/ubd.git

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=http://web.xtort.eu/~schneider/
#PKG_FIXUP:=libtool


#PKG_FIXUP:=autoreconf
#PKG_REMOVE_FILES=aclocal.m4

include $(INCLUDE_DIR)/package.mk
TARGET_CFLAGS += -L$(STAGING_DIR)/usr/lib/libiconv-full/lib -lz
define Package/ubd
 SECTION:=base
 CATEGORY:=Network
 DEFAULT:=n
 TITLE:=The ubd is a home automation project of the muccc
 URL:=https://wiki.muc.ccc.de/uberbus:start
 MAINTAINER:=Franz Nord <fnord@127.0.0.1>
 DEPENDS:= +libiconv-full +mxml +glib2 +zlib
endef

define Package/ubd/description
 Contains the control application running on the computer providing the IPv6 connectivity.
endef

#define Package/ubd/conffiles
#endef

#define Build/Configure
# $(call Build/Configure/Default)
#endef

define Build/Compile
 $(MAKE) -C $(PKG_BUILD_DIR) DESTDIR="$(PKG_INSTALL_DIR)" install
endef

define Package/ubd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/setname $(1)/usr/sbin/setname
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ubd $(1)/usr/sbin/ubd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ubnetd $(1)/usr/sbin/ubnetd

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ubd.init $(1)/etc/init.d/ubd
	$(INSTALL_BIN) ./files/ubnetd.init $(1)/etc/init.d/ubnetd

	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_BIN) ./files/ubdconfig.xml $(1)/etc/ubdconfig.xml
endef

$(eval $(call BuildPackage,ubd))
