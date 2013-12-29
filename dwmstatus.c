#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <syslog.h>
#include <X11/Xlib.h>

#define STATUS_BUF_SIZE 64

#define INFO_STRING "dwmstatus-"VERSION", this manifestation written by Ed Willson"

static Display *disp;

int read_int_from_file(const char *path);
void read_str_from_file(const char *path, char *buf, size_t buf_size);
bool get_batt_info(char *buf);
bool get_date_time(char *buf);

bool get_batt_info(char *buf)
{
    char batt_state[STATUS_BUF_SIZE];
    float energy_now, energy_full, energy_full_design, percent;

    energy_now = (float)read_int_from_file("/sys/class/power_supply/BAT0/energy_now");
    energy_full	= (float)read_int_from_file("/sys/class/power_supply/BAT0/energy_full");
    energy_full_design =
        (float)read_int_from_file("/sys/class/power_supply/BAT0/energy_full_design");


    if(abs(energy_full-energy_full_design) / energy_full_design > 0.1)
    {
        syslog(LOG_WARNING, "Large energy_full - energy_full_design deviation. "
               "Falling back to design value");
        percent = energy_now / energy_full_design;
    } else {
        percent = energy_now / energy_full;
    }

    if(percent < 0.0)
    {
        syslog(LOG_WARNING, "Battery percent < 0");
        return False;
    }

    read_str_from_file("/sys/class/power_supply/BAT0/status", batt_state, STATUS_BUF_SIZE);

    sprintf(buf, "Battery %s (%.0f%%)", batt_state, 100*percent);
    return True;
}

bool get_date_time(char *buf)
{
    time_t result;
    struct tm *resulttm;

    result = time(NULL);
    resulttm = localtime(&result);
    if(resulttm == NULL) {
        syslog(LOG_WARNING, "Error getting localtime");
        return False;
    }
    if(!strftime(buf, sizeof(char)*STATUS_BUF_SIZE - 1, "%A %d %B %R", resulttm)) {
        syslog(LOG_WARNING, "strftime is 0");
        return False;
    }

    return True;
}

int read_int_from_file(const char *path)
{
    int rv = -1;
    FILE *fd = fopen(path, "r");

    if(fd == NULL)
    {
        syslog(LOG_WARNING, "Error opening file %s", path);
    } else {
        fscanf(fd, "%d", &rv);
        fclose(fd);
    }

    return rv;
}

void read_str_from_file(const char *path, char *buf, size_t buf_size)
{
    FILE *fd;
    char ch = 0;
    int idx = 0;

    if (!(fd = fopen(path, "r"))) return;

    while ((ch = fgetc(fd)) != EOF && ch != '\0' && ch != '\n' && idx < buf_size) {
        buf[idx++] = ch;
    }

    buf[idx] = '\0';
    fclose(fd);
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

    if (!(disp = XOpenDisplay(NULL))) {
        syslog(LOG_ERR, "Cannot open X display");
        exit(EXIT_FAILURE);
    }

    char status_text[2*STATUS_BUF_SIZE];
    char batt_buf[STATUS_BUF_SIZE];
    char time_buf[STATUS_BUF_SIZE];

    for(;;sleep(10)) 
    {
        if(!get_batt_info(batt_buf) || !get_date_time(time_buf))
        {
            syslog(LOG_WARNING, "Error fetching status info");
        }

        // Generate output string and write to X root window name / stdout.
        sprintf(status_text, "%s  |  %s", batt_buf, time_buf);

#ifndef DEBUG
        XStoreName(disp, DefaultRootWindow(disp), status_text);
        XSync(disp, False);
#else
        printf("%s\n", status_text);
#endif // DEBUG
    }

    XCloseDisplay(disp);

    closelog();
    
    exit(EXIT_SUCCESS);
}
