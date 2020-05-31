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

struct gr_sys_t_ {
    struct ev_loop *loop;
    size_t num_signals;
    ev_signal *signals;
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
