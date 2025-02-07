/*
 * control with keyboard
 */
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include "webcamctrl.h"

void key_controller(void) {
    int ch;
    initscr(); // init screen
    raw(); // no buffering
    keypad(stdscr, TRUE); // accept special keys
    noecho();
    int dxy = 10;
    int dz = 3;
    while (1) {
        ch = getch();
        switch(ch) {
          case KEY_UP:
            printf("up ");
            tilt_relative_c(dxy);
            break;
          case KEY_DOWN:
            tilt_relative_c(-1 * dxy);
            printf("down ");
            break;
          case KEY_LEFT:
            pan_relative_c(dxy);
            printf("left ");
            break;
          case KEY_RIGHT:
            pan_relative_c(-1 * dxy);
            printf("right ");
            break;
          case 'z':
            zoom_relative_c(dz);
            printf("zoom ");
            break;
          case 'x':
            zoom_relative_c(-1 * dz);
            printf("unzoom ");
            break;
          case 'r':
            reset_divelog_c();
            printf("reset ");
            break;
          case 'q':
            endwin(); // finalize screen
            exit(0);
          default:
            printf("%c ", ch);
            break;
        }
        fflush(stdout);
        refresh();
        usleep(1);
    }
    endwin(); // finalize screen
    return;
}

