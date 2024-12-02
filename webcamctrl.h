#ifndef __WEBCAMCTRL_H__
#define __WEBCAMCTRL_H__

#include <netinet/in.h>

void pad_controller(const char *devname);
void key_controller();
void pan_relative_c(int val);
void tilt_relative_c(int val);
void zoom_relative_c(int val);
void zoom_continuous_c(int val);
void camera_controller(const char *devname, in_port_t tcp_port);
int setup_server(in_port_t port);
int setup_client(char *hostname, in_port_t port);

#define INDEV_DEFAULT "key" // or device file name for gamepad e.g. "/dev/input/event19"
#define OUTDEV_DEFAULT "/dev/obsbot"
#define TCP_PORT_DEFAULT ((in_port_t)1234)
#define TCP_PORT_STR_DEFAULT "1234"
#define MAX_CLIENT (1)
#define MAX_DATA_SIZE (128)

typedef struct {
    int pan;
    int tilt;
    int zoom;
    int type; // 0:relative 1:absolute
} CameraMovePkt;

#endif // __WEBCAMCTRL_H__
