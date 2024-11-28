/*
 * control with Nintendo Pro Controller gamepad
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include "webcamctrl.h"

/*
  Perform nonblocking read() from /dev/input/event19
  and you'd obtain an event in the form:

  struct input_event {
      struct timeval time;
      __u16 type;
      __u16 code;
      __s32 value;
  }
 */
void
pad_controller(const char *devname) {
    fprintf(stderr, "Open gamepad %s ... ", devname);
    int procon = open(devname, O_RDONLY);
    if (procon == -1){
        perror("pad_controller() failed.");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "opened.\n");

    char buf[128];
    struct input_event ev;
    int evsize = sizeof(ev);
    int dir; // direction -1 or +1
    int dxy = 10;
    int dz = 10;
    while (1) {
        int nread = read(procon, buf, evsize);
        if (nread == -1) {
            perror("procon read");
            exit(1);
        }
        // printf("nread: %d  evsize: %d\n", nread, evsize);
        memcpy((void *)&ev, buf, evsize);
        switch (ev.type) {
          case EV_SYN:
            // nop
            break;

          case EV_KEY:
            if (ev.code == BTN_TL2) zoom_relative_c(-dz);
            if (ev.code == BTN_TR2) zoom_relative_c(dz);
            // printf("type: %d  code: %d  value: %d\n", ev.type, ev.code, ev.value);
            break;

          case EV_ABS:
            // cross key
            dir = ev.value;
            if (ev.code == ABS_HAT0X) pan_relative_c(dir * dxy);
            if (ev.code == ABS_HAT0Y) tilt_relative_c(dir * dxy);

            // left stick
            int val = ev.value / 1000;
            if (abs(val) > 3) { // ignore too small values
                if (ev.code == ABS_X) pan_relative_c(-1 * val);
                if (ev.code == ABS_Y) tilt_relative_c(val);
                printf("%s val:%d\n", ev.code == ABS_X ? " pan" : "tilt", val);
            }
            break;

          default:
            // nop
            break;
        }
    }
    close(procon);
    exit(0);
    return;
}
