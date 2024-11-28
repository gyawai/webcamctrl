/*
 * control with keyboard
 */
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include "webcamctrl.h"

void key_controller(int fd) {
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
            move_relative(fd, 0, dxy);
            break;
          case KEY_DOWN:
            move_relative(fd, 0, -1 * dxy);
            printf("down ");
            break;
          case KEY_LEFT:
            move_relative(fd, dxy, 0);
            printf("left ");
            break;
          case KEY_RIGHT:
            move_relative(fd, -1 * dxy, 0);
            printf("right ");
            break;
          case 'z':
            zoom_relative(fd, dz);
            printf("zoom ");
            break;
          case 'x':
            zoom_relative(fd, -1 * dz);
            printf("unzoom ");
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

