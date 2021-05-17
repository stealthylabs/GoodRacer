/*
 * Copyright: 2015-2020. Stealthy Labs LLC. All Rights Reserved.
 * Date: 19 May 2020
 * Software: GoodRacer
 */
#include <goodracer_config.h>
#ifdef GOODRACER_HAVE_POPT
#include <popt.h>
#endif
#include <goodracer_utils.h>
#include <goodracer_system.h>

typedef struct {
    uint32_t gps_baud_rate;
    char gps_device[PATH_MAX];
    char i2c_device[PATH_MAX];
    uint8_t i2c_addr;
    uint8_t i2c_width;
    uint8_t i2c_height;
} gr_args_t;

static struct poptOption gr_args_table[] = {
    {
        .longName = "gps-baud-rate",
        .shortName = 'B',
        .argInfo = POPT_ARG_INT,
        .arg = NULL,
        .val = 'B',
        .descrip = "Set the GPS baud rate. Default is 9600 bps",
        .argDescrip = "9600 | 19200 | 38400 | 57600 | 115200"
    },
    {
        .longName = "gps-device",
        .shortName = 'd',
        .argInfo = POPT_ARG_STRING,
        .arg = NULL,
        .val = 'd',
        .descrip = "Set the GPS device path. Default is /dev/serial0.",
        .argDescrip = "/dev/serial0 | /dev/ttyUSB0 | /dev/ttyS0 etc."
    },
    {
        .longName = "i2c-device",
        .shortName = 'i',
        .argInfo = POPT_ARG_STRING,
        .arg = NULL,
        .val = 'i',
        .descrip = "Set the I2C OLED device path. Default is /dev/i2c-1.",
        .argDescrip = "/dev/i2c-1 | /dev/i2c-0 etc."
    },
    {
        .longName = "i2c-addr",
        .shortName = 'a',
        .argInfo = POPT_ARG_INT,
        .arg = NULL,
        .val = 'a',
        .descrip = "Set the I2C OLED device address byte in hexadecimal. Default is 0x3c",
        .argDescrip = "0x3c | 0x3d"
    },
    {
        .longName = "i2c-width",
        .shortName = 'W',
        .argInfo = POPT_ARG_INT,
        .arg = NULL,
        .val = 'W',
        .descrip = "Set the I2C OLED device width in pixels. Default is 128.",
        .argDescrip = "128 | 96"
    },
    {
        .longName = "i2c-height",
        .shortName = 'H',
        .argInfo = POPT_ARG_INT,
        .arg = NULL,
        .val = 'H',
        .descrip = "Set the I2C OLED device height in pixels. Default is 32.",
        .argDescrip = "32 | 64"
    },
    {
        .longName = "version",
        .shortName = 'V',
        .argInfo = POPT_ARG_NONE,
        .arg = NULL,
        .val = 'V',
        .descrip = "Show version message",
        .argDescrip = NULL
    },
    {
        .longName = "verbose",
        .shortName = 'v',
        .argInfo = POPT_ARG_NONE,
        .arg = NULL,
        .val = 'v',
        .descrip = "Do verbose logging",
        .argDescrip = NULL
    },
    {
        .longName = "help",
        .shortName = 'h',
        .argInfo = (POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN),
        .arg = NULL,
        .val = '?',
        .descrip = "Show this help message",
        .argDescrip = NULL
    },
    POPT_AUTOHELP
    POPT_TABLEEND
};

void gr_args_init(gr_args_t *args)
{
    if (args) {
        memset(args, 0, sizeof(*args));
        args->gps_baud_rate = 9600;
        snprintf(args->gps_device, sizeof(args->gps_device), "/dev/serial0");
        snprintf(args->i2c_device, sizeof(args->i2c_device), "/dev/i2c-1");
        args->i2c_addr = 0x3c;
        args->i2c_width = 128;
        args->i2c_height = 32;
    }
}

void gr_args_cleanup(gr_args_t *args)
{
    if (args) {
        memset(args, 0, sizeof(*args));
    }
}

#define gr_args_parse_uint8(A,B) gr_args_parse_integer(A,B,10,true,sizeof(uint8_t))
#define gr_args_parse_uint16(A,B) gr_args_parse_integer(A,B,10,true,sizeof(uint16_t))
#define gr_args_parse_uint32(A,B) gr_args_parse_integer(A,B,10,true,sizeof(uint32_t))
#define gr_args_parse_uint64(A,B) gr_args_parse_integer(A,B,10,true,sizeof(uint64_t))
#define gr_args_parse_int8(A,B) gr_args_parse_integer(A,B,10,true,sizeof(int8_t))
#define gr_args_parse_int16(A,B) gr_args_parse_integer(A,B,10,true,sizeof(int16_t))
#define gr_args_parse_int32(A,B) gr_args_parse_integer(A,B,10,true,sizeof(int32_t))
#define gr_args_parse_int64(A,B) gr_args_parse_integer(A,B,10,true,sizeof(int64_t))
#define gr_args_parse_hex_uint8(A,B) gr_args_parse_integer(A,B,16,true,sizeof(uint8_t))
#define gr_args_parse_hex_uint16(A,B) gr_args_parse_integer(A,B,16,true,sizeof(uint16_t))
#define gr_args_parse_hex_uint32(A,B) gr_args_parse_integer(A,B,16,true,sizeof(uint32_t))
#define gr_args_parse_hex_uint64(A,B) gr_args_parse_integer(A,B,16,true,sizeof(uint64_t))
#define gr_args_parse_hex_int8(A,B) gr_args_parse_integer(A,B,16,true,sizeof(int8_t))
#define gr_args_parse_hex_int16(A,B) gr_args_parse_integer(A,B,16,true,sizeof(int16_t))
#define gr_args_parse_hex_int32(A,B) gr_args_parse_integer(A,B,16,true,sizeof(int32_t))
#define gr_args_parse_hex_int64(A,B) gr_args_parse_integer(A,B,16,true,sizeof(int64_t))

static int gr_args_parse_integer(const char *str, void *valp,
                    int base, bool is_unsigned, size_t vsz)
{
    if (str && valp) {
        char *endp = NULL;
        errno = 0;
        if (is_unsigned) {
            unsigned long val = strtoul(str, &endp, base);
            if (vsz == sizeof(uint32_t)) {
                *(uint32_t *)valp = (uint32_t)(val & 0xFFFFFFFF);
            } else if (vsz == sizeof(uint64_t)) {
                *(uint64_t *)valp = val;
            } else if (vsz == sizeof(uint16_t)) {
                *(uint16_t *)valp = (uint16_t)(val & 0xFFFF);
            } else if (vsz == sizeof(uint8_t)) {
                *(uint8_t *)valp = (uint8_t)(val & 0xFF);
            } else {
                GRLOG_DEBUG("Unacceptable size parameter: %zd, using unsigned long\n", vsz);
                return -1;
            }
        } else {
            long val = strtol(str, &endp, base);
            if (vsz == sizeof(int32_t)) {
                *(int32_t *)valp = (int32_t)val;
            } else if (vsz == sizeof(int64_t)) {
                *(int64_t *)valp = val;
            } else if (vsz == sizeof(int16_t)) {
                *(int16_t *)valp = (int16_t)val;
            } else if (vsz == sizeof(int8_t)) {
                *(int8_t *)valp = (int8_t)val;
            } else {
                GRLOG_DEBUG("Unacceptable size parameter: %zd, using long\n", vsz);
                return -1;
            }
        }
        if (errno != 0 || (str == endp)) {
            errno = 0;
            GRLOG_ERROR("Cannot parse unsigned integer from '%s'\n", str);
        } else {
            return 0;
        }
    }
    return -1;
}


int gr_args_parse(int argc, const char **argv, gr_args_t *args)
{
    bool show_usage = false;
    bool show_version = false;
    int opt = 0, rc = 0;
    if (!args || !argv || argc == 0) {
        GRLOG_ERROR("Invalid arguments\n");
        return -1;
    }
    poptContext ctx = poptGetContext(argv[0], argc, argv, gr_args_table, 0);
    if (!ctx) {
        GRLOG_ERROR("poptGetContext() error\n");
        return -1;
    }
    while ((opt = poptGetNextOpt(ctx)) >= 0 && !show_usage && !show_version) {
        char *argbuf = NULL;
        switch (opt) {
        case 'h':
        case '?':
            show_usage = true;
            break;
        case 'V':
            show_version = true;
            break;
        case 'v':
            GRLOG_LEVEL_SET(DEBUG);
            GRLOG_DEBUG("Setting log level to DEBUG\n");
            break;
        case 'd':
            argbuf = poptGetOptArg(ctx);
            if (argbuf) {
                if (strlen(argbuf) < sizeof(args->gps_device)) {
                    memset(args->gps_device, 0, sizeof(args->gps_device));
                    strncpy(args->gps_device, argbuf, strlen(argbuf));
                    GRLOG_INFO("Using GPS device: %s\n", args->gps_device);
                } else {
                    GRLOG_ERROR("GPS device %s is too long and max size is %zu\n",
                            argbuf, sizeof(args->gps_device));
                    rc = -1;
                }
            }
            break;
        case 'i':
            argbuf = poptGetOptArg(ctx);
            if (argbuf) {
                if (strlen(argbuf) < sizeof(args->i2c_device)) {
                    memset(args->i2c_device, 0, sizeof(args->i2c_device));
                    strncpy(args->i2c_device, argbuf, strlen(argbuf));
                    GRLOG_INFO("Using I2C OLED device: %s\n", args->i2c_device);
                } else {
                    GRLOG_ERROR("I2C device path %s is too long and max size is %zu\n",
                            argbuf, sizeof(args->i2c_device));
                    rc = -1;
                }
            }
            break;
        case 'a':
            argbuf = poptGetOptArg(ctx);
            if (argbuf) {
                if (gr_args_parse_hex_uint8(argbuf, &args->i2c_addr) < 0) {
                    GRLOG_WARN("Invalid value for I2C OLED address: %s. Using default\n", argbuf);
                    args->i2c_addr = 0x3c;
                } else {
                    GRLOG_INFO("Using I2C address byte 0x%x\n", args->i2c_addr);
                }
            }
            break;
        case 'W':
            argbuf = poptGetOptArg(ctx);
            if (argbuf) {
                if (gr_args_parse_uint8(argbuf, &args->i2c_width) < 0) {
                    GRLOG_WARN("Invalid value for I2C OLED width: %s. Using default\n", argbuf);
                    args->i2c_width = 128;
                } else {
                    GRLOG_INFO("Using I2C OLED width %u\n", args->i2c_width);
                }
            }
            break;
        case 'H':
            argbuf = poptGetOptArg(ctx);
            if (argbuf) {
                if (gr_args_parse_uint8(argbuf, &args->i2c_height) < 0) {
                    GRLOG_WARN("Invalid value for I2C OLED height: %s. Using default\n", argbuf);
                    args->i2c_height = 32;
                } else {
                    GRLOG_INFO("Using I2C OLED height %u\n", args->i2c_height);
                }
            }
            break;
        case 'B':
            argbuf = poptGetOptArg(ctx);
            if (argbuf) {
                uint32_t speed = 0;
                if (gr_args_parse_uint32(argbuf, &speed) < 0) {
                    GRLOG_WARN("Invalid value for GPS baud rate: %s\n", argbuf);
                } else {
                    //make sure only acceptable baud rates are allowed
                    switch (speed) {
                    case 9600:
                    case 19200:
                    case 38400:
                    case 57600:
                    case 115200:
                        break;
                    default:
                        GRLOG_WARN("Invalid value for baud rate specified: %d. Using 9600\n", speed);
                        speed = 9600;
                        break;
                    }
                    args->gps_baud_rate = speed;
                    GRLOG_DEBUG("Setting GPS Baud Rate: %d bps\n", args->gps_baud_rate);
                }
            }
            break;
        default:
            GRLOG_ERROR("'%c' is an invalid option\n", (opt & 0xFF));
            show_usage = true;
            break;
        }
        GR_FREE(argbuf);
    }
    if (opt < -1) {
        GRLOG_ERROR("%s %s: %s\n", argv[0], poptBadOption(ctx, POPT_BADOPTION_NOALIAS), poptStrerror(opt));
        rc = -1;
        show_usage = true;
    }
    if (show_version) {
        printf("%s\n", GOODRACER_VERSION);
        rc = -1;
    }
    if (show_usage) {
        poptPrintHelp(ctx, stdout, 0);
        rc = -1;
    }
    poptFreeContext(ctx);
    return rc;
}

static void goodracer_gps_error_cb(gr_sys_t *sys, gr_gps_t *gps)
{
    if (!sys || !gps)
        return;
    GRLOG_ERROR("Error callback invoked");
}

static void goodracer_gps_read_cb(gr_sys_t *sys, gr_gps_t *gps,
        const gpsdata_data_t *datalistp, size_t num)
{
    if (!sys || !gps || !datalistp || num == 0)
        return;
    gpsdata_list_dump(datalistp, GRLOG_PTR);
}

int main (int argc, const char **argv)
{
    int rc = 0;
    gr_args_t args;
    gr_disp_t *disp = NULL;
    gr_gps_t *gps = NULL;

    gr_args_init(&args);
    if ((rc = gr_args_parse(argc, (const char **)argv, &args)) < 0) {
        return rc;
    }
    gr_sys_t *sys = gr_system_setup();
    if (!sys) {
        GRLOG_ERROR("Failed to setup system\n");
        return -1;
    }
    do {
        /* connect the OLED */
        disp = gr_display_i2c_setup(args.i2c_device, args.i2c_addr,
                args.i2c_width, args.i2c_height);
        if (!disp) {
            GRLOG_ERROR("failed to perform I2C OLED screen setup on device path %s", args.i2c_device);
            rc = -1;
            break;
        }
        /* connect the GPS */
        gps = gr_gps_setup(args.gps_device, args.gps_baud_rate);
        if (!gps) {
            GRLOG_ERROR("failed to connect to the GPS via device path %s", args.gps_device);
            rc = -1;
            break;
        }
        rc = gr_system_set_display(sys, disp);
        if (rc < 0) {
            GRLOG_ERROR("Failed to set the display for the system");
            break;
        }
        rc = gr_system_watch_gps(sys, gps, goodracer_gps_read_cb, goodracer_gps_error_cb);
        if (rc < 0) {
            GRLOG_ERROR("Failed to set the I/O watcher for the GPS in the system");
            break;
        }
        /* do other setup stuff here */
        rc = gr_system_run(sys);
    } while (0);
    gr_gps_cleanup(gps);
    gr_display_cleanup(disp);
    gr_system_cleanup(sys);
    gr_args_cleanup(&args);
    return rc;
}
