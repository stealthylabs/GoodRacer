AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = $(ACLOCAL_FLAGS)

bin_PROGRAMS=goodracer

goodracer_SOURCES=main.c
goodracer_CFLAGS=$(POPT_CFLAGS) $(SOCKETCAN_CFLAGS)
goodracer_CFLAGS+=-I$(top_srcdir)/libgps_mtk3339/include
goodracer_CFLAGS+=-I$(top_srcdir)/libgps_mtk3339/src
goodracer_CFLAGS+=-I$(top_srcdir)/libssd1306/include
goodracer_CFLAGS+=-I$(top_srcdir)/libssd1306/src
goodracer_LDADD=$(POPT_LIBS) $(SOCKETCAN_LIBS)
goodracer_LDADD+=$(top_srcdir)/libgps_mtk3339/src/libgps_mtk3339.la
goodracer_LDADD+=$(top_srcdir)/libssd1306/src/libssd1306_i2c.la
if HAVE_LIBEV
goodracer_CFLAGS+=$(LIBEV_CFLAGS)
goodracer_LDADD+=$(LIBEV_LIBS)
endif
