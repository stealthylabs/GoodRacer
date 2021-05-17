/*
 * Copyright: 2015-2020. Stealthy Labs LLC. All Rights Reserved.
 * Date: 24 May 2020
 * Software: GoodRacer
 */
#ifndef __GOODRACER_UTILS_H__
#define __GOODRACER_UTILS_H__
#include <gpsutils.h>
#include <gpsdata.h>
#include <ssd1306_i2c.h>

#ifndef GRLOG_PTR
#define GRLOG_PTR GPSUTILS_LOG_PTR
#endif
#define GRLOG_LEVEL_SET GPSUTILS_LOGLEVEL_SET
#define GRLOG_ERROR GPSUTILS_ERROR
#define GRLOG_WARN GPSUTILS_WARN
#define GRLOG_INFO GPSUTILS_INFO
#define GRLOG_DEBUG GPSUTILS_DEBUG
#define GRLOG_NONE GPSUTILS_NONE
#define GRLOG_OUTOFMEM GPSUTILS_ERROR_NOMEM
#define GR_FREE GPSUTILS_FREE

#endif /* __GOODRACER_UTILS_H__ */
