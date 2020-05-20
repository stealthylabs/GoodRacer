#include <goodracer_config.h>
#ifdef GOODRACER_HAVE_POPT
#include <popt.h>
#endif
#include <gpsutils.h>

typedef struct {
    uint32_t gps_baud_rate;
} gr_args_t;

static struct poptOption gr_args_table[] = {
    {
        .longName = "gps-baud-rate",
        .shortName = 'B',
        .argInfo = (POPT_ARG_INT | POPT_ARGFLAG_OPTIONAL),
        .arg = NULL,
        .val = 'B',
        .descrip = "Set the GPS baud rate. Default is 9600 bps",
        .argDescrip = "9600 or 19200 or 38400 or 115200"
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

static int gr_args_parse_uint(const char *str, uint32_t *valp)
{
    if (str && valp) {
        char *endp = NULL;
        errno = 0;
        *valp = (uint32_t)strtoul(str, &endp, 10);
        if (errno != 0 || (str == endp)) {
            errno = 0;
            GPSUTILS_ERROR("Cannot parse unsigned integer from '%s'\n", str);
            *valp = 0;
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
        GPSUTILS_ERROR("Invalid arguments\n");
        return -1;
    }
    poptContext ctx = poptGetContext(argv[0], argc, argv, gr_args_table, 0);
    if (!ctx) {
        GPSUTILS_ERROR("poptGetContext() error\n");
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
            GPSUTILS_LOGLEVEL_SET(DEBUG);
            break;
        case 'B':
            argbuf = poptGetOptArg(ctx);
            if (argbuf) {
                uint32_t speed = 0;
                if (gr_args_parse_uint(argbuf, &speed) < 0) {
                    GPSUTILS_WARN("Invalid value for GPS baud rate: %s\n", argbuf);
                } else {
                    //TODO: make sure only acceptable baud rates are allowed
                    args->gps_baud_rate = speed;
                }
            }
        default:
            GPSUTILS_ERROR("'%c' is an invalid option\n", (opt & 0xFF));
            show_usage = true;
            break;
        }
        GPSUTILS_FREE(argbuf);
    }
    if (opt < -1) {
        GPSUTILS_ERROR("%s %s: %s\n", argv[0], poptBadOption(ctx, POPT_BADOPTION_NOALIAS), poptStrerror(opt));
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
