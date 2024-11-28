#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <argp.h>
#include <stdbool.h>

// For the webcam
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>
#include "webcamctrl.h"

static int xioctl(int fd, int request, void *arg);
static int32_t swap_int32(int32_t in32);
static void test_basic(int fd);



/*
 * functions exported
 */

void
move_relative(int fd, int pan, int tilt) {
    struct v4l2_control gctrl;
    struct v4l2_control sctrl;

    // pan
    if (pan != 0) {
        gctrl.id = V4L2_CID_PAN_ABSOLUTE;
        if (xioctl(fd, VIDIOC_G_CTRL, &gctrl) == -1) {
            perror("get ioctl");
        }
        sctrl.id = V4L2_CID_PAN_ABSOLUTE;
        sctrl.value = 3500 * pan + gctrl.value;
        if (xioctl(fd, VIDIOC_S_CTRL, &sctrl) == -1) {
            perror("set ioctl");
        }
    }

    // tilt
    if (tilt != 0) {
        gctrl.id = V4L2_CID_TILT_ABSOLUTE;
        if (xioctl(fd, VIDIOC_G_CTRL, &gctrl) == -1) {
            perror("get ioctl");
        }
        sctrl.id = V4L2_CID_TILT_ABSOLUTE;
        sctrl.value = 3500 * tilt + gctrl.value;
        if (xioctl(fd, VIDIOC_S_CTRL, &sctrl) == -1) {
            perror("set ioctl");
        }
    }
}

void
zoom_relative(int fd, int val) {
    struct v4l2_control gctrl;
    struct v4l2_control sctrl;

    gctrl.id = V4L2_CID_ZOOM_ABSOLUTE;
    if (xioctl(fd, VIDIOC_G_CTRL, &gctrl) == -1) {
        perror("get ioctl");
    }
    printf("zoom: %d\n", gctrl.value);
    sctrl.id = V4L2_CID_ZOOM_ABSOLUTE;
    sctrl.value = val + gctrl.value;
    if (sctrl.value <= 0) sctrl.value = 0;
    if (xioctl(fd, VIDIOC_S_CTRL, &sctrl) == -1) {
        perror("set ioctl");
    }

}

void
zoom_continuous(int fd, int val) {
    struct v4l2_control sctrl;

    sctrl.id = V4L2_CID_ZOOM_CONTINUOUS;
    sctrl.value = val;
    if (xioctl(fd, VIDIOC_S_CTRL, &sctrl) == -1) {
        perror("set ioctl");
    }

}

/*
 * local functions
 */

const char *argp_program_version = "webcamctrl 0.0";
const char *argp_program_bug_address = "nobody@nowhere.org";
static char doc[] = "A webcam controller";
static char args_doc[] = ""; // mandatory args
static struct argp_option options[] = { 
    { "indev", 'i', "DEV_FILE_NAME", 0, "Device file name of the gamepad, or \"key\" for keyboard [/dev/input/event19]"},
    { "outdev", 'o', "DEV_FILE_NAME", 0, "Device file name of the output, i.e., the camera. [/dev/obsbot]"},
    { "client", 'c', 0, 0, "Run as a client that handles the input device. [false]"},
    { "server", 's', 0, 0, "Run as a server that handles the camera device. [false]"},
    { 0 },
};

struct arguments {
    char *outdev;
    char *indev;
    bool is_client;
    bool is_server;
    bool is_standalone;
    // char *args[2]; // not used for now
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *args = state->input;
    switch (key) {
      case 'i': args->indev = arg;
        break;
      case 'o': args->outdev = arg;
        break;
      case 'c':
        args->is_client = true;
        args->is_standalone = false;
        break;
      case 's':
        args->is_server = true;
        args->is_standalone = false;
        break;
      case ARGP_KEY_ARG: // variable numbers of mandatory args
#if 0
        if (state->arg_num >= 2) {
            argp_usage(state);
        }
        args->args[state->arg_num] = arg;
#endif
        return 0;
      default:
        return ARGP_ERR_UNKNOWN;
    }   
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };







static int
open_webcam(char *devname) {
    fprintf(stderr, "Open camera %s ... ", devname);
    int fd = open(devname, O_RDWR);
    if (fd == -1){
        perror("open_webcam() failed.");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "opened.\n");
    return fd;
}


static int
xioctl(int fd, int request, void *arg) {
    int r;
    do {
        r = ioctl(fd,request,arg);
    } while (-1 == r && EINTR == errno);
    return r;
}

static int32_t
swap_int32(int32_t in32) {
#if 0
    int32_t out32;
    char *in8 = (char *)&in32;
    char *out8 = (char *)&out32;
    out8[0] = in8[3];
    out8[1] = in8[2];
    out8[2] = in8[1];
    out8[3] = in8[0];
    return out32;
#else
    return in32;
#endif
}

static void test_basic(int fd) {
    struct v4l2_control gctrl;
    struct v4l2_control sctrl;
    int32_t val, dir = 1;
    sctrl.value = 10;

    while (1) {
        for (int i = 0; i < 5; i += 1) {

#if 1
            // "G"et control
            gctrl.id = V4L2_CID_PAN_ABSOLUTE;
            if (xioctl(fd, VIDIOC_G_CTRL, &gctrl) != -1) {
                val = swap_int32(gctrl.value);
                fprintf(stderr, "PAN: %d\n", (int)val);
            }
            else {
                perror("get ioctl");
            }
#endif


#if 0
            // "S"et control
            sctrl.id = V4L2_CID_PAN_ABSOLUTE;
            val = 3600 * 10 * i * dir;
            fprintf(stderr, "val: %d\n", (int)val);
            sctrl.value = swap_int32((int32_t)val);
            if (xioctl(fd, VIDIOC_S_CTRL, &sctrl) != -1) {
                sleep(1);
            }
            else {
                perror("set ioctl");
            }
#endif

#if 1
            // "S"et control
            sctrl.id = V4L2_CID_PAN_SPEED;
            val = 360 * 10 * dir;
            fprintf(stderr, "val: %d\n", (int)val);
            sctrl.value = swap_int32((int32_t)val);
            if (xioctl(fd, VIDIOC_S_CTRL, &sctrl) != -1) {
                sleep(1);
            }
            else {
                perror("set ioctl");
            }
#endif


#if 0
            // "G"et control
            gctrl.id = V4L2_CID_PAN_ABSOLUTE;
            if (xioctl(fd, VIDIOC_G_CTRL, &gctrl) != -1) {
                val = swap_int32(gctrl.value);
                fprintf(stderr, "PAN: %d\n", val);
            }
            else {
                perror("get ioctl");
            }
#endif


            
#if 0
            // "S"et control
            sctrl.id = V4L2_CID_PAN_ABSOLUTE;
            sctrl.value = swap_int32((int32_t)val);
            if (xioctl(fd, VIDIOC_S_CTRL, &sctrl) != -1) {
                fprintf(stderr, ".");
                sleep(10);
            }
            else {
                perror("set ioctl");
            }
#endif

#if 0
            sctrl.id = V4L2_CID_PAN_SPEED;
            sctrl.value = swap_int32((int32_t)(1000 * dir));
            if (xioctl(fd, VIDIOC_S_CTRL, &sctrl) != -1) {
                fprintf(stderr, ".");
            }
            else {
                perror("set ioctl");
            }
#endif
            sleep(1);
        }
        dir *= -1;
    }
    exit(EXIT_SUCCESS);
}

int
main(int argc, char **argv) {
    struct arguments args;
    bool use_key = false;

    // args default values
    args.outdev = "/dev/obsbot";
    args.indev = "/dev/input/event19";
    args.is_client = false;
    args.is_server = false;
    args.is_standalone = true;
   
    argp_parse(&argp, argc, argv, 0, 0, &args);

    printf("outdev: %s\n", args.outdev);
    if (strcmp(args.indev, "key")) {
        printf("indev: %s (gamepad)\n", args.indev);
    }
    else {
        use_key = true;
        printf("indev: arrow keys ('q' to quit).\n");
    }

    int fd = open_webcam(args.outdev);

    // test_basic(fd);

    if (use_key) {
        key_controller(fd);
    }
    else {
        pad_controller(fd, args.indev);
    }    
    close(fd);
    exit(EXIT_SUCCESS);
}
