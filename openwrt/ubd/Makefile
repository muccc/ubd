# This is free software, licensed under s

include $(TOPDIR)/rules.mk

PKG_REV:=c9637eed0c5e7d679fae958c6816de2584eeff53
PKG_NAME:=ubd
PKG_VERSION:=0.2
PKG_RELEASE:=6

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_REV).tar.gz
PKG_SOURCE_URL:=https://github.com/schneider42/ubd.git

#PKG_FIXUP:=autoreconf
#PKG_REMOVE_FILES=aclocal.m4

include $(INCLUDE_DIR)/package.mk

define Package/ubd
 SECTION:=base
 CATEGORY:=Network
 DEFAULT:=n
 TITLE:=The ubd is a home automation project of the muccc
 URL:=https://wiki.muc.ccc.de/uberbus:start
 MAINTAINER:=Franz Nord <fnord@127.0.0.1>
 DEPENDS:=+mxml +glib2 +libavahi-client +zlib
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
endef

$(eval $(call BuildPackage,ubd))
