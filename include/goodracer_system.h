/*
 * Copyright: 2015-2020. Stealthy Labs LLC. All Rights Reserved.
 * Date: 31 May 2020
 * Software: GoodRacer
 */
#ifndef __GOODRACER_SYSTEM_H__
#define __GOODRACER_SYSTEM_H__

/* opaque system structure */
typedef struct gr_sys_t_ gr_sys_t;

/* setup the system event loop and default handlers
 */
gr_sys_t *gr_system_setup();

/* cleanup the system event loop and handlers */
void gr_system_cleanup(gr_sys_t *);

/* run the system event loop */
int gr_system_run(gr_sys_t *);

/* display related functions */
typedef struct gr_disp_t_ {
    ssd1306_i2c_t *oled;
    ssd1306_framebuffer_t *fbp;
    volatile int _ref; //reference counting
} gr_disp_t;

/* setup the I2C display for SSD1306 OLED display */
gr_disp_t *gr_display_i2c_setup(const char *dev, uint8_t addr,
                    uint8_t width, uint8_t height);

/* cleanup the display object */
void gr_display_cleanup(gr_disp_t *);
/* increment reference count */
void gr_display_inc_ref(gr_disp_t *);

/* opaque GPS struct */
typedef struct gr_gps_t_ gr_gps_t;

/* tell the system about the display */
int gr_system_set_display(gr_sys_t *, gr_disp_t *);

gr_gps_t *gr_gps_setup(const char *dev, uint32_t baud_rate);

void gr_gps_cleanup(gr_gps_t *);

typedef void (* gr_gps_on_read_t)(gr_sys_t *, gr_gps_t *,
            const gpsdata_data_t *, size_t);
typedef void (* gr_gps_on_error_t)(gr_sys_t *, gr_gps_t *);

int gr_system_watch_gps(gr_sys_t *sys, gr_gps_t *gps,
            gr_gps_on_read_t read_cb, /* callback called when data received from GPS */
            gr_gps_on_error_t err_cb /* callback called when error in reading data from GPS */
            );

#endif /* __GOODRACER_SYSTEM_H__ */
