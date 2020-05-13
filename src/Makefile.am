AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = $(ACLOCAL_FLAGS)

bin_PROGRAMS=ezracer

ezracer_SOURCES=main.c
ezracer_CFLAGS=$(POPT_CFLAGS) $(SOCKETCAN_CFLAGS)
ezracer_CFLAGS+=-I$(top_srcdir)/libgps_mtk3339/include
ezracer_CFLAGS+=-I$(top_srcdir)/libgps_mtk3339/src
ezracer_CFLAGS+=-I$(top_srcdir)/libssd1306/include
ezracer_CFLAGS+=-I$(top_srcdir)/libssd1306/src
ezracer_LDADD=$(POPT_LIBS) $(SOCKETCAN_LIBS)
ezracer_LDADD+=$(top_srcdir)/libgps_mtk3339/src/libgps_mtk3339.la
ezracer_LDADD+=$(top_srcdir)/libssd1306/src/libssd1306_i2c.la
if HAVE_LIBEV
ezracer_CFLAGS+=$(LIBEV_CFLAGS)
ezracer_LDADD+=$(LIBEV_LIBS)
endif
