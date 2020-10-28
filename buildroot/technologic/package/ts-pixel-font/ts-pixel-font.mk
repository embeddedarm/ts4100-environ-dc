################################################################################
#
# Pixel font that is 4x5 used for monochrome display
#
################################################################################

TS_PIXEL_FONT_VERSION = 1.0
TS_PIXEL_FONT_SITE_METHOD = file
TS_PIXEL_FONT_SITE = $(BR2_EXTERNAL_TS4100_ENVIRON_PATH)/package/ts-pixel-font
TS_PIXEL_FONT_SOURCE = ts-pixel-4x5-$(TS_PIXEL_FONT_VERSION).zip

TS_PIXEL_FONT_LICENSE = OFL-1.1

define TS_PIXEL_FONT_INSTALL_TARGET_CMDS
	$(INSTALL) -d -m 0755 $(TARGET_DIR)/usr/share/fonts/ts/
	$(INSTALL) -m 0644 -D $(@D)/ts-pixel-4x5.ttf $(TARGET_DIR)/usr/share/fonts/ts/
endef

define TS_PIXEL_FONT_EXTRACT_CMDS
        $(UNZIP) -d $(@D) $(TS_PIXEL_FONT_DL_DIR)/$(TS_PIXEL_FONT_SOURCE)
endef


$(eval $(generic-package))
