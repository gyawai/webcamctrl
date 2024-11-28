#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <netdb.h>
#include "utils.h"
#include "webcamctrl.h"


static int send_packet(CameraMovePkt *pktp);

SOCKET Soc;

void
pan_relative_c(int val) {
    CameraMovePkt pkt = {
        .pan = val,
        .tilt = 0,
        .zoom = 0,
        .type = 0,
    };
    send_packet(&pkt);
}

void
tilt_relative_c(int val) {
    CameraMovePkt pkt = {
        .pan = 0,
        .tilt = val,
        .zoom = 0,
        .type = 0,
    };
    send_packet(&pkt);
}

void
zoom_relative_c(int val) {
    CameraMovePkt pkt = {
        .pan = 0,
        .tilt = 0,
        .zoom = val,
        .type = 0,
    };
    send_packet(&pkt);
}

void
zoom_continuous_c(int val) {
    // nop
    fprintf(stderr, "zoom_continuous_c() not implemented.\n");
}


int
setup_client(char *hostname, in_port_t port)
{
    struct hostent *server_ent;
    struct sockaddr_in server;

    fprintf(stderr, "connecting to %s ... ", hostname);
    server_ent = gethostbyname(hostname);
    if (server_ent == NULL) {
	perror("gethostbyname() failed.");
	return -1;
    }

    memset((char *)&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    memcpy((char *)&server.sin_addr, server_ent->h_addr,
	   server_ent->h_length);

    Soc = socket(AF_INET, SOCK_STREAM, 0);
    if (Soc < 0) {
	perror("socket() failed.");
	return -1;
    }

    if (connect(Soc, (struct sockaddr *)&server, sizeof(server)) == -1) {
	perror("connect() failed.");
	return -1;
    }
    fprintf(stderr, "connected.\n");
}


static int
send_packet(CameraMovePkt *pktp) {
    char buf[MAX_DATA_SIZE];
    int pktsize = sizeof(CameraMovePkt);
    int bufsize = pktsize + sizeof(int);
    *(int *)buf = htonl(pktsize);
    memcpy(buf + sizeof(int), pktp, pktsize);
    int nsent = send(Soc, buf, bufsize, 0);
    if (nsent == -1) {
        perror("send() failed.");
        return -1;
    }
    if (nsent != bufsize) {
                
        fprintf(stderr, "Requested and sent data size unmatched. nsent:%d != bufsize:%d\n", nsent, bufsize);
        return -1;
    }
}
