#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <argp.h>
#include <stdbool.h>
#include "webcamctrl.h"

/*
 * functions exported
 */

// nop

/*
 * local functions
 */

const char *argp_program_version = "webcamctrl 0.0";
const char *argp_program_bug_address = "nobody@nowhere.org";
static char doc[] = "A webcam controller";
static char args_doc[] = ""; // mandatory args
static struct argp_option options[] = { 
    { "indev", 'i', "DEV_FILE_NAME", 0, "Device file name of the gamepad, or \"key\" for keyboard [" INDEV_DEFAULT "]"},
    { "outdev", 'o', "DEV_FILE_NAME", 0, "Device file name of the output, i.e., the camera. [" OUTDEV_DEFAULT "]"},
    { "tcpport", 'p', "TCP_PORT_NUM", 0, "TCP port number to communicate [" TCP_PORT_STR_DEFAULT "]"},
    { "client", 'c', "SERVER_NAME", 0, "Run as a client that handles the input device. [\"\"]"},
    { "server", 's', 0, 0, "Run as a server that handles the camera device. [false]"},
    { 0 },
};

struct arguments {
    char *outdev;
    char *indev;
    bool is_client;
    bool is_server;
    in_port_t tcp_port;
    char *servername;
    // char *args[2]; // not used for now
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *args = state->input;
    switch (key) {
      case 'i': args->indev = arg;
        break;
      case 'o': args->outdev = arg;
        break;
      case 'p': args->tcp_port = (in_port_t)atoi(arg);
        break;
      case 'c':
        args->servername = arg;
        args->is_client = true;
        break;
      case 's':
        args->is_server = true;
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

int
main(int argc, char **argv) {
    struct arguments args;

    // args default values
    args.outdev = OUTDEV_DEFAULT;
    args.indev = INDEV_DEFAULT;
    args.is_client = false;
    args.is_server = false;
    args.tcp_port = TCP_PORT_DEFAULT;
    argp_parse(&argp, argc, argv, 0, 0, &args);

    if (args.is_client) {
        setup_client(args.servername, args.tcp_port);
        if (strcmp(args.indev, "key")) {
            printf("indev: %s (gamepad)\n", args.indev);
            pad_controller(args.indev);
        }
        else {
            printf("indev: arrow keys ('q' to quit).\n");
            key_controller();
        }
    }
    else if (args.is_server) {
      server_mainloop(args.outdev, args.tcp_port);
    }
    else {
        fprintf(stderr, "Either '--server' or '--client' must be specified.\n");
        exit(1);
    }
    exit(EXIT_SUCCESS);
}
