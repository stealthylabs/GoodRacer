AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = $(ACLOCAL_FLAGS)

bin_PROGRAMS=ezracer

ezracer_SOURCES=main.c
ezracer_CFLAGS=$(POPT_CFLAGS) $(SOCKETCAN_CFLAGS)
ezracer_LDADD=$(POPT_LIBS) $(SOCKETCAN_LIBS) libssd1306/libssd1306_i2c.la libgps_mtk3339/libgps_mtk3339.la
if HAVE_LIBEV
ezracer_CFLAGS+=$(LIBEV_CFLAGS)
ezracer_LDADD+=$(LIBEV_LIBS)
endif
