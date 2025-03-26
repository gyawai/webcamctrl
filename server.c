#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>
#include "utils.h"
#include "webcamctrl.h"

static int open_camera(const char *devname);
static int recv_packet(SOCKET soc, Pkt *pktp);
static int xioctl(int fd, int request, void *arg);
static void move_camera(int fd, int type, int pan, int tilt, int zoom);
static void reset_divelog(void);
static void reboot_myself(void);
static void shutdown_myself(void);

int
setup_server(in_port_t port)
{
    struct sockaddr_in me;
    SOCKET soc_waiting;
    SOCKET soc_client;
    int yes = 1;

    memset((char *)&me, 0, sizeof(me));
    me.sin_family = AF_INET;
    me.sin_addr.s_addr = htonl(INADDR_ANY);
    me.sin_port = htons(port);

    soc_waiting = socket(AF_INET, SOCK_STREAM, 0);
    if (soc_waiting < 0) {
	perror("socket");
	return -1;
    }
    setsockopt(soc_waiting, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));
   
    if (bind(soc_waiting, (struct sockaddr *)&me, sizeof(me)) == -1) {
	perror("bind");
	return -1;
    }

    listen(soc_waiting, 1);
    fprintf(stderr, "Successfully bound.\n"
            "Waiting for connection request ...\n");

    soc_client = accept(soc_waiting, NULL, NULL);
    close(soc_waiting);
    fprintf(stderr, "Connection established.\n");
    
    return soc_client;
}

void
server_mainloop(const char *devname, in_port_t tcp_port) {
    int fd_camera = open_camera(devname);
    while (true) {
        fprintf(stderr, "Listening port %d ... ", tcp_port);
        SOCKET soc_client = setup_server(tcp_port);
        while (true) {
	    Pkt pkt;
            int nrecv = recv_packet(soc_client, &pkt);
            if (nrecv == -1) {
                fprintf(stderr, "Disconnected.\n");
                break;
            }
            if (nrecv == 0) {
                break;
            }
	    switch (pkt.type) {
	    case 0:
	        fprintf(stderr, "pan:%d  tile:%d  zoom:%d  type:%d\n",
			pkt.camera.pan, pkt.camera.tilt, pkt.camera.zoom, pkt.camera.type);
		move_camera(fd_camera, pkt.camera.type, pkt.camera.pan, pkt.camera.tilt, pkt.camera.zoom);
		break;
	    case 1:
  	        fprintf(stderr, "reset?: %d\n", pkt.sensor.reset);
		if (pkt.sensor.reset == 1) {
		    reset_divelog();
		}
		if (pkt.sensor.reboot == 1) {
		  // reboot_myself();
		  shutdown_myself();
		}
		break;

	    default:
	        fprintf(stderr, "ignore an undefined packet type: %d\n", pkt.type);
		break;
	    }
        }
        close(soc_client);
    }
    close(fd_camera);
    return;
}

static int
open_camera(const char *devname) {
    fprintf(stderr, "Open camera %s ... ", devname);
    int fd = open(devname, O_RDWR);
    if (fd == -1){
        perror("open_webcam() failed.");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "Opened.\n");
    return fd;
}

/*
 * Move camera by the amount given for each direction (i.e. ptz).
 * The 'type' can take 0:relative 1:absolute but for now this value is not used.
 */
static void
move_camera(int fd, int type, int pan, int tilt, int zoom) {
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

    // zoom
    if (zoom != 0) {
        gctrl.id = V4L2_CID_ZOOM_ABSOLUTE;
        if (xioctl(fd, VIDIOC_G_CTRL, &gctrl) == -1) {
            perror("get ioctl");
        }
        sctrl.id = V4L2_CID_ZOOM_ABSOLUTE;
        sctrl.value = zoom + gctrl.value;
        if (sctrl.value <= 0) sctrl.value = 0;
        if (xioctl(fd, VIDIOC_S_CTRL, &sctrl) == -1) {
            perror("set ioctl");
        }
    }
}

static void
reset_divelog(void) {
    fprintf(stderr, "#### reset dive log ####\n");
    char *fname = "../streamer/var/depthlog.csv";
#if 0
    FILE* fp = fopen(fname, "w");
    if (!fp) {
      perror(fname);
    }
    else {
      fclose(fp);
      fprintf(stderr, "the file content cleared.");
    }
#else
    int err = remove(fname);
    if (err) {
        perror(fname);
    }
    else {
        fprintf(stderr, "the file removed.");
    }
#endif
}

static void reboot_myself(void) {
    fprintf(stderr, "#### reboot the system! ####\n");
    system("/usr/bin/sudo /usr/sbin/reboot");
    fprintf(stderr, "#### reboot execed. ####\n");
}

static void shutdown_myself(void) {
    fprintf(stderr, "#### shutdown the system! ####\n");
    system("/usr/bin/sudo /usr/sbin/poweroff");
    fprintf(stderr, "#### poweroff execed. ####\n");
}

static int
xioctl(int fd, int request, void *arg) {
    int r;
    do {
        r = ioctl(fd,request,arg);
    } while (-1 == r && EINTR == errno);
    return r;
}

/*
 * Return value;
 *      -1: error
 *       0: connection closed by peer.
 *  others: bytes recieved.
 *
 */
static int
recv_packet(SOCKET soc, Pkt *pktp) {
    char buf[MAX_DATA_SIZE];
    int nrecvd = recv(soc, buf, sizeof(int), 0);
    if (nrecvd == -1) {
        perror("recv_packet() failed.");
	sleep(10);
        return -1;
    }
    if (nrecvd == 0) {
        fprintf(stderr, "Connection closed by peer.\n");
        return 0;
    }
    int pktsize = ntohl(*(int *)buf);
    nrecvd = recv(soc, buf, pktsize, 0);
    if (nrecvd == -1) {
        perror("recv_packet() failed.");
        return -1;
    }
    if (nrecvd != pktsize) {
        fprintf(stderr,
                "Requested and received data size unmatched. nrecvd:%d != pktsize:%d\n",
                nrecvd, pktsize);
        return -1;
    }
    memcpy((char *)pktp, buf, pktsize);
    return pktsize;
}
