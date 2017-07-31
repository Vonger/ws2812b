include $(TOPDIR)/rules.mk

PKG_NAME:=ws2812
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_CONFIG_DEPENDS:=

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/ws2812
  SECTION:=vocore2
  CATEGORY:=VoCore2 Features
  TITLE:=Control WS2812 LEDs
endef

define Package/ws2812/description
  demo used to control ws2812 leds.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Package/ws2812/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ws2812 $(1)/usr/bin
endef

$(eval $(call BuildPackage,ws2812))

