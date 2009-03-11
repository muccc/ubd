#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 13844 2009-01-04 00:06:33Z lars $

include $(TOPDIR)/rules.mk

PKG_NAME:=gnet2
PKG_VERSION:=2.0.8
PKG_RELEASE:=1

PKG_SOURCE:=gnet-$(PKG_VERSION).tar.bz2
PKG_BUILD_DIR:=$(BUILD_DIR)/gnet-$(PKG_VERSION)
PKG_SOURCE_URL:=@GNOME/gnet/2.0
PKG_FIXUP:=libtool

include $(INCLUDE_DIR)/package.mk

#PKG_INSTALL=1

define Package/gnet2
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+glib2
  TITLE:=gnet 2.0
  URL:=http://www.gtk.org/
endef

define Package/gnet2/description
  The GLib library of C routines
endef

TARGET_CFLAGS+= \
	-I$(STAGING_DIR)/usr/lib/libiconv/include \
	-I$(STAGING_DIR)/usr/lib/libintl/include
#	$(FPIC) \

TARGET_LDFLAGS+= \
	-Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
	-L$(STAGING_DIR)/usr/lib/libiconv/lib \
	-L$(STAGING_DIR)/usr/lib/libintl/lib

#define Build/Configure
#	mkdir -p $(PKG_BUILD_DIR)/build/$(GNU_HOST_NAME)
#	( cd $(PKG_BUILD_DIR); CONFIG_SITE= \
#		./configure \
#		--target=$(GNU_HOST_NAME) \
#		--host=$(GNU_HOST_NAME) \
#		--build=$(GNU_HOST_NAME) \
#		--prefix=$(PKG_BUILD_DIR)/build/$(GNU_HOST_NAME) \
#		--disable-shared \
#		--enable-static \
#		--enable-debug=no \
#	);
#	$(MAKE) -C $(PKG_BUILD_DIR) install clean
#
#	$(call Build/Configure/Default, \
#		--enable-shared \
#		--enable-static \
#		--enable-debug=no \
#		, \
#		ac_cv_path_GLIB_GENMARSHAL=$(PKG_BUILD_DIR)/build/$(GNU_HOST_NAME)/bin/glib-genmarshal \
#		ac_cv_func_mmap_fixed_mapped=yes \
#	);
#endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR)/ install DESTDIR=$(PKG_INSTALL_DIR)
endef

#define Build/Configure
#    $(call Build/Configure/Default)
#endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/gnet-2.0
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/gnet-2.0/*.h \
					$(1)/usr/include/gnet-2.0
	
	$(INSTALL_DIR) $(1)/usr/lib/gnet-2.0/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/gnet-2.0/include/*.h \
					$(1)/usr/lib/gnet-2.0/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.{so*,a,la} \
			 $(1)/usr/lib/
	
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/* \
					$(1)/usr/lib/pkgconfig/
	
	$(INSTALL_DIR) $(2)/share/aclocal/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/aclocal/*.m4 \
					$(2)/share/aclocal/
endef

define Package/gnet2/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
endef


$(eval $(call BuildPackage,gnet2))
