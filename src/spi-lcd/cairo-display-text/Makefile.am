GITCOMMIT:= $(shell git describe --abbrev=12 --dirty --always)

cairo_display_text_SOURCES = main.c lcd-display.c
cairo_display_text_CPPFLAGS = -DGITCOMMIT="\"${GITCOMMIT}\"" $(PANGO_CFLAGS) $(CAIRO_CFLAGS) $(CONFIGCONFIG_CFLAGS)
cairo_display_text_LDFLAGS = $(PANGO_LIBS) $(CIARO_LIBS) $(FONTCONFIG_LIBS)

bin_PROGRAMS = cairo-display-text
