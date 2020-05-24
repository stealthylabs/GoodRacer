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

typedef struct {
    uint32_t gps_baud_rate;
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
        args->gps_baud_rate = 9600;
    }
}

void gr_args_cleanup(gr_args_t *args)
{
    if (args) {
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

int main (int argc, char **argv)
{
    int rc = 0;
    gr_args_t args;

    gr_args_init(&args);
    if ((rc = gr_args_parse(argc, (const char **)argv, &args)) < 0) {
        return rc;
    }
    return -1;
}
