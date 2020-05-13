AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = $(ACLOCAL_FLAGS)

bin_PROGRAMS=ezracer

ezracer_SOURCES=main.c
ezracer_CFLAGS=$(POPT_CFLAGS)
ezracer_LDADD=$(POPT_LIBS)
if HAVE_LIBEV
ezracer_CFLAGS+=$(LIBEV_CFLAGS)
ezracer_LDADD+=$(LIBEV_LIBS)
endif

