#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <syslog.h>
#include <X11/Xlib.h>

#define STATUS_BUF_SIZE         64

#define INFO_STRING             "dwmstatus-"VERSION", this manifestation written by Ed Willson"

#define ENERGY_NOW_PATH         "/sys/class/power_supply/BAT0/energy_now"
#define ENERGY_FULL_PATH        "/sys/class/power_supply/BAT0/energy_full"
#define ENERGY_FULL_DESIGN_PATH "/sys/class/power_supply/BAT0/energy_full_design"
#define BATT_STATUS_PATH        "/sys/class/power_supply/BAT0/status"
#define WIRELESS_INFO_PATH      "/proc/net/wireless"

static Display *disp;

int read_int_from_file(FILE *);
size_t read_str_from_file(FILE *, char *, size_t);
size_t read_word_from_file(FILE *, char *, size_t);
bool get_batt_info(char *);
bool get_network_info(char *);
bool get_date_time(char *);

bool get_batt_info(char *buf)
{
    char batt_state[STATUS_BUF_SIZE];
    float energy_now, energy_full, energy_full_design, percent;

    FILE *f_energy_now = fopen(ENERGY_NOW_PATH, "r");
    FILE *f_energy_full = fopen(ENERGY_FULL_PATH, "r");
    FILE *f_energy_full_design = fopen(ENERGY_FULL_DESIGN_PATH, "r");

    if (!f_energy_now || !f_energy_full || !f_energy_full_design)
    {
        syslog(LOG_ERR, "%s: Failed to open battery value file(s)", __func__);
        return false;
    }

    energy_now = (float)read_int_from_file(f_energy_now);
    energy_full	= (float)read_int_from_file(f_energy_full);
    energy_full_design = (float)read_int_from_file(f_energy_full_design);

    if (abs(energy_full-energy_full_design) / energy_full_design > 0.1)
    {
        syslog(LOG_WARNING, "%s: Large energy_full - energy_full_design deviation. "
                "Falling back to design value", __func__);
        percent = energy_now / energy_full_design;
    }
    else
    {
        percent = energy_now / energy_full;
    }

    if (percent < 0.0)
    {
        syslog(LOG_WARNING, "%s: Battery percent negative (%f)", __func__, percent);
        return false;
    }

    fclose(f_energy_now);
    fclose(f_energy_full);
    fclose(f_energy_full_design);

    FILE *f_batt_status = fopen(BATT_STATUS_PATH, "r");

    if (!f_batt_status)
    {
        syslog(LOG_ERR, "%s: Failed to open battery status file", __func__);
        return false;
    }

    read_str_from_file(f_batt_status, batt_state, STATUS_BUF_SIZE);

    fclose(f_batt_status);

    size_t n_ch = snprintf(buf, STATUS_BUF_SIZE, "Battery %s (%.0f%%)", batt_state, 100*percent);
    return n_ch > STATUS_BUF_SIZE ? false : true;
}

bool get_network_info(char *buf)
{
    char iface_name[STATUS_BUF_SIZE];
    char iface_status[STATUS_BUF_SIZE];

    FILE *wireless_info = fopen(WIRELESS_INFO_PATH, "r");

    if (!(wireless_info))
    {
        syslog(LOG_ERR, "%s: Failed to open wireless info file", __func__);
        return false;
    }

    // we need to read 2 complete lines, then we read our words...
    char ch;
    do { ch = fgetc(wireless_info); } while (ch != EOF && ch != '\n');
    do { ch = fgetc(wireless_info); } while (ch != EOF && ch != '\n');

    read_word_from_file(wireless_info, iface_name, STATUS_BUF_SIZE);
    read_word_from_file(wireless_info, iface_status, STATUS_BUF_SIZE);

    fclose(wireless_info);

    size_t n_ch = snprintf(buf, STATUS_BUF_SIZE, "%s up (status %s)", iface_name, iface_status);
    return n_ch > STATUS_BUF_SIZE ? false : true;
}

bool get_date_time(char *buf)
{
    time_t result;
    struct tm *resulttm;

    result = time(NULL);
    resulttm = localtime(&result);
    if (resulttm == NULL)
    {
        syslog(LOG_WARNING, "%s: Failed to get localtime", __func__);
        return false;
    }

    if (!strftime(buf, STATUS_BUF_SIZE, "%A %d %B %R", resulttm))
    {
        syslog(LOG_WARNING, "%s: strftime failed - buffer too small?", __func__);
        return false;
    }

    return true;
}

int read_int_from_file(FILE *f)
{
    int rv = -1;

    if (f == NULL)
    {
        syslog(LOG_CRIT, "%s: File not open", __func__);
    }
    else
    {
        fscanf(f, "%d", &rv);
    }

    return rv;
}

size_t read_str_from_file(FILE *f, char *buf, size_t buf_size)
{
    char ch = 0;
    size_t idx = 0;

    if (f == NULL)
    {
        syslog(LOG_CRIT, "%s: File not open", __func__);
    }
    else
    {
        while ((ch = fgetc(f)) != EOF &&
                ch != '\0' &&
                ch != '\n' &&
                idx < buf_size)
        {
            buf[idx++] = ch;
        }

        buf[idx] = '\0';
    }

    return idx;
}

size_t read_word_from_file(FILE *f, char *buf, size_t buf_size)
{
    char ch = 0;
    size_t idx = 0;

    if (f == NULL)
    {
        syslog(LOG_CRIT, "%s: File not open", __func__);
    }
    else
    {
        bool in_word = false;
        while ((ch = fgetc(f)) != EOF && idx < buf_size)
        {
            if (ch == '\n' || ch == '\r' || ch == ' ' || ch == '\t')
            {
                // whitespace
                if (in_word)
                {
                    // word ended
                    break;
                }
            }
            else
            {
                // word
                if (!in_word)
                {
                    in_word = true;
                }

                buf[idx++] = ch;
            }
        }

        buf[idx] = '\0';
    }

    return idx;
}

int main(int argc, char *argv[])
{
    if(argc == 2 && !(strcmp(argv[1], "-v") && strcmp(argv[1], "--version")))
    {
        printf(INFO_STRING"\n");
        exit(EXIT_SUCCESS);
    }
    else if(argc > 1)
    {
        printf(INFO_STRING"\nUsage dwmstatus [-v | --version]\n");
        exit(EXIT_FAILURE);
    }

    openlog("dwmstatus", LOG_PID|LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "%s: Opened log file", __func__);

    if (!(disp = XOpenDisplay(NULL))) {
        syslog(LOG_ERR, "%s: Cannot open X display", __func__);
        exit(EXIT_FAILURE);
    }

    char status_text[3*STATUS_BUF_SIZE];
    char batt_buf[STATUS_BUF_SIZE];
    char net_buf[STATUS_BUF_SIZE];
    char time_buf[STATUS_BUF_SIZE];

    for(;;sleep(10)) 
    {
        if (!get_batt_info(batt_buf))
        {
            syslog(LOG_WARNING, "%s: Error fetching battery info", __func__);
        }

        if (!get_network_info(net_buf))
        {
            syslog(LOG_WARNING, "%s: Error fetching network info", __func__);
        }

        if (!get_date_time(time_buf))
        {
            syslog(LOG_WARNING, "%s: Error fetching date and time info", __func__);
        }

        // Generate output string and write to X root window name / stdout.
        snprintf(status_text, 3*STATUS_BUF_SIZE, "%s  | %s | %s", batt_buf, net_buf, time_buf);

        XStoreName(disp, DefaultRootWindow(disp), status_text);
        XSync(disp, false);
#ifdef DEBUG
        syslog(LOG_DEBUG, "%s", status_text);
#endif
    }

    XCloseDisplay(disp);

    closelog();

    exit(EXIT_SUCCESS);
}
