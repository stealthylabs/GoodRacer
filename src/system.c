/*
 * Copyright: 2015-2020. Stealthy Labs LLC. All Rights Reserved.
 * Date: 31 May 2020
 * Software: GoodRacer
 */
#include <goodracer_config.h>
#ifdef GOODRACER_HAVE_FEATURES_H
#include <features.h>
#endif
#ifdef GOODRACER_HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef GOODRACER_HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef GOODRACER_HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef GOODRACER_HAVE_STDBOOL_H
#include <stdbool.h>
#endif
#ifdef GOODRACER_HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef GOODRACER_HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef GOODRACER_HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef GOODRACER_HAVE_STRING_H
#include <string.h>
#endif
#ifdef GOODRACER_HAVE_EV_H
#include <ev.h>
#endif
#ifdef GOODRACER_HAVE_SYS_EVENTFD_H
#include <sys/eventfd.h>
#endif
#ifdef GOODRACER_HAVE_SYS_TIMERFD_H
#include <sys/timerfd.h>
#endif
#ifdef GOODRACER_HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef GOODRACER_HAVE_SYS_SIGNALFD_H
#include <sys/signalfd.h>
#endif
#ifndef GOODRACER_HAVE_STRSIGNAL
#define strsignal(A) "function not defined"
#endif
#ifdef GOODRACER_HAVE_EXECINFO_H
#include <execinfo.h>
#endif
#ifdef GOODRACER_HAVE_UCONTEXT_H
#include <ucontext.h>
#endif
#include <goodracer_utils.h>
#include <goodracer_system.h>

struct gr_gps_t_ {
    int fd;
    uint32_t baud_rate;
    gpsdata_parser_t *parser;
    volatile int _ref; //reference counted
};

static void gr_gps_inc_ref(gr_gps_t *gps)
{
    if (gps) {
        SSD1306_ATOMIC_INCREMENT(&(gps->_ref));
    }
}

struct gr_sys_t_ {
    struct ev_loop *loop;
    size_t num_signals;
    ev_signal *signals;
    gr_disp_t *disp;
    /* GPS watcher */
    gr_gps_t *gps;
    ev_io gps_watcher;
    gr_gps_on_read_t gps_io_read_cb;
    gr_gps_on_error_t gps_io_error_cb;
};

/* if we ever need more complex backtraces, we can use libbacktrace */
static void gr_system_print_backtrace()
{
    static void *_pvt_gr_sys_btstack[64];
    int num_stack = backtrace(_pvt_gr_sys_btstack, 64);
    if (num_stack > 0) {
        /* we want to use this function since on an embedded system the
         * memory available may be low, so it may just be prudent to directly
         * print the backtrace to the file descriptor.
         * note that the signal handler may also be in the trace,
         * and the context is unavailable
         */
        // the array entry [1] in the symbols is the signal handler and not the
        // caller where the function crashed. signalfd() which libev uses does
        // not necessarily return the context and libev definitely does not
        int fd = fileno(GRLOG_PTR);
        GRLOG_DEBUG("Printing backtrace to fd %d\n", fd);
        GRLOG_NONE("---------- BEGIN BACKTRACE ----------\n");
        backtrace_symbols_fd(_pvt_gr_sys_btstack, num_stack, fd);
        GRLOG_NONE("---------- END BACKTRACE ----------\n");
    } else {
        GRLOG_WARN("No backtrace stack available to print\n");
    }
}

static void gr_system_signal_cb(EV_P_ struct ev_signal *ev, int revents)
{
    int sig = ev ? ev->signum : 0;
    if (revents & EV_SIGNAL) {
        GRLOG_ERROR("Signal %d (%s) was thrown\n", sig, strsignal(sig));
    } else {
        GRLOG_ERROR("Unknown Signal was thrown\n");
    }
    if (sig != SIGTERM && sig != SIGINT) {
        gr_system_print_backtrace();
    }
    ev_break(EV_A_ EVBREAK_ALL);
}

static int gr_system_watch_signals(gr_sys_t *sys)
{
    /* we use libev's signal handler setup.
     * we could technically use sigaction() and get the ucontext() returned
     * correctly but it is unclear if that works nicely with ev-loop
     */
    if (sys && sys->loop) {
        int sigs[] = {
            SIGSEGV, SIGINT, SIGABRT, SIGHUP, SIGILL,
            SIGTERM, SIGQUIT, SIGPWR, SIGFPE
        };
        sys->num_signals = sizeof(sigs) / sizeof(int);
        sys->signals = calloc(sys->num_signals, sizeof(ev_signal));
        if (!sys->signals) {
            GRLOG_OUTOFMEM(sys->num_signals * sizeof(ev_signal));
            return -1;
        }
        for (size_t i = 0; i < sys->num_signals; ++i) {
            ev_signal_init(&(sys->signals[i]), gr_system_signal_cb, sigs[i]);
            sys->signals[i].data = (void *)sys;
            ev_signal_start(sys->loop, &(sys->signals[i]));
            ev_unref(sys->loop);// long running watcher
        }
        GRLOG_DEBUG("All signal event handlers added\n");
        return 0;
    }
    return -1;
}

gr_sys_t *gr_system_setup()
{
    int rc = 0;
    gr_sys_t *sys = calloc(1, sizeof(gr_sys_t));
    if (!sys) {
        GRLOG_OUTOFMEM(sizeof(gr_sys_t));
        return NULL;
    }
    do {
        sys->loop = ev_default_loop(EVFLAG_SIGNALFD);
        if (gr_system_watch_signals(sys) < 0) {
            GRLOG_WARN("Signal handling setup failed, not running without it");
            rc = -1;
            break;
        }
    } while (0);
    if (rc < 0) {
        gr_system_cleanup(sys);
        sys = NULL;
    }
    return sys;
}

void gr_system_cleanup(gr_sys_t *sys)
{
    if (sys) {
        if (sys->gps) {
            ev_io_stop(sys->loop, &(sys->gps_watcher));
            memset(&(sys->gps_watcher), 0, sizeof(sys->gps_watcher));
            sys->gps_io_read_cb = NULL;
            sys->gps_io_error_cb = NULL;
            gr_gps_cleanup(sys->gps);
            sys->gps = NULL;
        }
        if (sys->disp) {
            gr_display_cleanup(sys->disp);
            sys->disp = NULL;
        }
        if (sys->loop) {
            if (sys->signals) {
                for (size_t i = 0; i < sys->num_signals; ++i) {
                    ev_ref(sys->loop);// as per ev.h documentation
                    ev_signal_stop(sys->loop, &(sys->signals[i]));
                }
            }
            // force loop exit
            ev_loop_destroy(sys->loop);
            GRLOG_DEBUG("Event loop destroyed\n");
        }
        GR_FREE(sys->signals);
    }
    GR_FREE(sys);
}

int gr_system_run(gr_sys_t *sys)
{
    int rc = 0;
    if (sys && sys->loop) {
        GRLOG_DEBUG("Event loop run started\n");
        rc = ev_run(sys->loop, 0);
        if (rc < 0) {
            GRLOG_ERROR("Event loop returned %d\n", rc);
        }
    } else {
        GRLOG_ERROR("Invalid system object, cannot run loop\n");
        rc = -1;
    }
    return rc;
}

gr_disp_t *gr_display_i2c_setup(const char *dev, uint8_t addr, uint8_t width, uint8_t height)
{
    int rc = 0;
    gr_disp_t *disp = NULL;
    do {
        if (!dev) {
            GRLOG_ERROR("I2C display device path cannot be NULL\n");
            rc = -1;
            break;
        }
        disp = calloc(1, sizeof(*disp));
        if (!disp) {
            GRLOG_OUTOFMEM(sizeof(*disp));
            rc = -1;
            break;
        }
        /* connect the I2C OLED */
        disp->oled = ssd1306_i2c_open(dev, addr, width, height, NULL);
        if (!disp->oled) {
            GRLOG_ERROR("Failed to setup the I2C OLED device");
            rc = -1;
            break;
        }
        if (ssd1306_i2c_display_initialize(disp->oled) < 0) {
            GRLOG_ERROR("Failed to initialize the display. Check if it is connected\n");
            rc = -1;
            break;
        }
        /* clear the display */
        ssd1306_i2c_display_clear(disp->oled);
        /* create the framebuffer */
        disp->fbp = ssd1306_framebuffer_create(disp->oled->width, disp->oled->height, disp->oled->err);
        if (!disp->fbp) {
            GRLOG_ERROR("Failed to create framebuffer object, cannot proceed");
            rc = -1;
            break;
        }
        SSD1306_ATOMIC_ZERO(&(disp->_ref));
        SSD1306_ATOMIC_INCREMENT(&(disp->_ref));
    } while (0);
    if (rc < 0) {
        gr_display_cleanup(disp);
        disp = NULL;
    }
    return disp;
}

void gr_display_inc_ref(gr_disp_t *disp)
{
    if (disp) {
        SSD1306_ATOMIC_INCREMENT(&(disp->_ref));
    }
}

void gr_display_cleanup(gr_disp_t *disp)
{
    if (disp) {
        int zero = 0;
        SSD1306_ATOMIC_DECREMENT(&(disp->_ref));
        if (SSD1306_ATOMIC_IS_EQUAL(&(disp->_ref), &zero)) {
            if (disp->fbp) {
                ssd1306_framebuffer_destroy(disp->fbp);
                disp->fbp = NULL;
            }
            if (disp->oled) {
                ssd1306_i2c_close(disp->oled);
                disp->oled = NULL;
            }
            GR_FREE(disp);
        }
    }
}

gr_gps_t *gr_gps_setup(const char *dev, uint32_t baud_rate)
{
    int rc = 0;
    gr_gps_t *gps = NULL;
    do {
        if (!dev) {
            GRLOG_ERROR("GPS device path cannot be NULL\n");
            rc = -1;
            break;
        }
        gps = calloc(1, sizeof(*gps));
        if (!gps) {
            GRLOG_OUTOFMEM(sizeof(*gps));
            rc = -1;
            break;
        }
        gps->parser = gpsdata_parser_create();
        if (!gps->parser) {
            GRLOG_ERROR("Failed to create GPS data parser object\n");
            rc = -1;
            break;
        }
        gps->fd = gpsdevice_open(dev, true);
        if (gps->fd < 0) {
            GRLOG_ERROR("Failed to open GPS device on path %s\n", dev);
            rc = -1;
            break;
        }
        gps->baud_rate = 9600;
        /* set baudrate if not 9600 */
        if (baud_rate != 9600) {
            if (gpsdevice_set_baudrate(gps->fd, baud_rate) < 0) {
                GRLOG_WARN("Unable to set baud rate of %d on the GPS, continuing to use the default\n", baud_rate);

            } else {
                GRLOG_INFO("Explicitly setting GPS Baud rate to %d\n", baud_rate);
                gps->baud_rate = baud_rate;
            }
        }
        gpsdevice_request_antenna_status(gps->fd, true, false);
        gpsdevice_request_firmware_info(gps->fd);
        SSD1306_ATOMIC_ZERO(&(gps->_ref));
        SSD1306_ATOMIC_INCREMENT(&(gps->_ref));
    } while (0);
    if (rc < 0) {
        gr_gps_cleanup(gps);
        gps = NULL;
    }
    return gps;
}

void gr_gps_cleanup(gr_gps_t *gps)
{
    if (gps) {
        int zero = 0;
        SSD1306_ATOMIC_DECREMENT(&(gps->_ref));
        if (SSD1306_ATOMIC_IS_EQUAL(&(gps->_ref), &zero)) {
            gpsdata_parser_free(gps->parser);
            gps->parser = NULL;
            gpsdevice_close(gps->fd);
            gps->fd = -1;
            GR_FREE(gps);
        }
    }
}

int gr_system_set_display(gr_sys_t *sys, gr_disp_t *disp)
{
    if (sys && disp) {
        if (sys->disp) {
            gr_display_cleanup(sys->disp);
            sys->disp = NULL;
        }
        sys->disp = disp;
        gr_display_inc_ref(disp);
        GRLOG_DEBUG("Successfully set the display pointer for the system");
        return 0;
    }
    return -1;
}

static void gr_system_gps_cb(EV_P_ ev_io *w, int revents)
{
    if (w && (revents & EV_READ)) {
        if (w->fd >= 0) {
            char buf[80];// GPS read buffer
            memset(buf, 0, sizeof(buf));
            ssize_t nb = read(w->fd, buf, sizeof(buf));
            gr_sys_t *sys = (gr_sys_t *)(w->data);
            if (nb < 0) {
                int err = errno;
                GRLOG_ERROR("Error reading device fd: %d. Error: %s(%d)\n",
                        w->fd, strerror(err), err);
                if (sys->gps_io_error_cb) {
                    sys->gps_io_error_cb(sys, sys->gps);
                }
                ev_io_stop(EV_A_ w);
                gpsdevice_close(w->fd);
            } else if (nb == 0) {
                GRLOG_DEBUG("No data received from device, waiting...\n");
            } else { // nb > 0
                gr_gps_t *gps = sys->gps;
                if (!gps || !gps->parser) {
                    GRLOG_ERROR("Invalid parser pointer. Closing I/O\n");
                    if (sys->gps_io_error_cb) {
                        sys->gps_io_error_cb(sys, sys->gps);
                    }
                    ev_io_stop(EV_A_ w);
                    gpsdevice_close(w->fd);
                } else {
                    /* parser is valid */
                    size_t onum = 0;
                    gpsdata_data_t *datalistp = NULL;
                    int rc = gpsdata_parser_parse(gps->parser, buf, nb,
                            &datalistp, &onum);
                    if (rc < 0) {
                        GRLOG_WARN("Failed to parse %zd bytes:\n", nb);
                        gpsutils_hex_dump((const uint8_t *)buf, (size_t)nb, GRLOG_PTR);
                        gpsdata_parser_reset(gps->parser);
                    } else {
                        GRLOG_DEBUG("Parsed %zu packets\n", onum);
                        if (sys->gps_io_read_cb) {
                            sys->gps_io_read_cb(sys, gps, datalistp, onum);
                        }
                        gpsdata_list_free(&(datalistp));
                        datalistp = NULL;
                    }
                }
            }
        }
    }
}

int gr_system_watch_gps(gr_sys_t *sys, gr_gps_t *gps,
                gr_gps_on_read_t read_cb,
                gr_gps_on_error_t err_cb)
{
    if (!sys || !gps || gps->fd < 0) {
        GRLOG_ERROR("Invalid system or GPS objects used as parameters");
        return -1;
    }
    memset(&(sys->gps_watcher), 0, sizeof(sys->gps_watcher));
    ev_io_init(&(sys->gps_watcher), gr_system_gps_cb, gps->fd, EV_READ);
    sys->gps = gps;
    gr_gps_inc_ref(gps);
    sys->gps_io_read_cb = read_cb;
    sys->gps_io_error_cb = err_cb;
    sys->gps_watcher.data = (void *)sys;
    ev_io_start(sys->loop, &(sys->gps_watcher));
    return 0;
}
