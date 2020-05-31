/*
 * Copyright: 2015-2020. Stealthy Labs LLC. All Rights Reserved.
 * Date: 31 May 2020
 * Software: GoodRacer
 */
#ifndef __GOODRACER_SYSTEM_H__
#define __GOODRACER_SYSTEM_H__

typedef struct gr_sys_t_ gr_sys_t;

/* setup the system event loop and default handlers
 */
gr_sys_t *gr_system_setup();

/* cleanup the system event loop and handlers */
void gr_system_cleanup(gr_sys_t *);

/* run the system event loop */
int gr_system_run(gr_sys_t *);

#endif /* __GOODRACER_SYSTEM_H__ */
