#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include "utils.h"
#include "webcamctrl.h"

static int open_camera(const char *devname);
static int recv_packet(SOCKET soc, CameraMovePkt *pktp);

int
setup_server(in_port_t port)
{
    struct sockaddr_in me;
    SOCKET soc_waiting;
    SOCKET soc_client;

    memset((char *)&me, 0, sizeof(me));
    me.sin_family = AF_INET;
    me.sin_addr.s_addr = htonl(INADDR_ANY);
    me.sin_port = htons(port);

    soc_waiting = socket(AF_INET, SOCK_STREAM, 0);
    if (soc_waiting < 0) {
	perror("socket");
	return -1;
    }

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
camera_controller(const char *devname, in_port_t tcp_port) {
    int fd_camera = open_camera(devname);
    while (true) {
        fprintf(stderr, "Listening port %d ... ", tcp_port);
        SOCKET soc_client = setup_server(tcp_port);
        while (true) {
            CameraMovePkt pkt;
            int nrecv = recv_packet(soc_client, &pkt);
            if (nrecv == 0) {
                break;
            }
            fprintf(stderr, "pan:%d  tile:%d  zoom:%d  type:%d\n",
                    pkt.pan, pkt.tilt, pkt.zoom, pkt.type);
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
 * Return value;
 *      -1: error
 *       0: connection closed by peer.
 *  others: bytes recieved.
 *
 */
static int
recv_packet(SOCKET soc, CameraMovePkt *pktp) {
    char buf[MAX_DATA_SIZE];
    int nrecvd = recv(soc, buf, sizeof(int), 0);
    if (nrecvd == -1) {
        perror("recv_packet() failed.");
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
