#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <ncurses.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        printf("\nrainfall v1\n");
        printf("written in C by servalx4\n");
        printf("\nInstructions:\n");
        printf("--help | show this menu\n");
        printf("k/K    | toggle paused/unpaused\n");
        printf("q/Q    | quit\n");
        printf("up/dwn | speed up, speed down\n");
        printf("right  | change mode\n");
        printf("c/C    | clear screen\n");
        return 0;
    }

    typedef struct {
        int type;
        int state;
    } cell;

    const char CHARLIST[] = " o*#";

    int paused = 0;
    int mode = 1;
    int delay = 50000;

    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }

    const int gameWidth = w.ws_col - 1;
    const int gameHeight = w.ws_row;

    cell xyMatrix[gameWidth][gameHeight];
    for (int i = 0; i < gameWidth; i++) {
        for (int j = 0; j < gameHeight; j++) {
            xyMatrix[i][j].state = 0;
            xyMatrix[i][j].type = 0;
        }
    }

    int KeyInput() {
        int key = getch();
        switch (key) {
            case 'k':
            case 'K':
                paused = !paused;
                break;
            case KEY_UP:
                delay += 10000;
                break;
            case KEY_DOWN:
                delay -= 10000;
                break;
            case KEY_RIGHT:
                mode = ((mode + 1) % 4);
                break;
            case 'c':
            case 'C':
                for (int y = 0; y < gameHeight; y++) {
                    for (int x = 0; x < gameWidth; x++) {
                        xyMatrix[x][y].type = 0;
                        xyMatrix[x][y].state = 0;
                    }
                }
                break;
            case 'q':
            case 'Q':
                return 1;
        }
        if (mode == 0) { mode = 1; }
        return 0;
    }

    void UpdateScreen(int active) {
        for (int y = 0; y < gameHeight; y++) {
            for (int x = 0; x < gameWidth; x++) {
                if (xyMatrix[x][y].type != 0 && active == 0) {
                    if (xyMatrix[x][y].state == 0) {

                        if (xyMatrix[x][y].type == 3) { 
                            if (y + 1 < gameHeight) {
                                if (!(xyMatrix[x][y + 1].type == 0 || xyMatrix[x][y + 1].type == 1)) {
                                    if (x - 1 > 0) { xyMatrix[x - 1][y].type = 2; xyMatrix[x - 1][y].state = 3; }
                                    xyMatrix[x][y].type = 2;
                                    xyMatrix[x][y].state = 3;
                                    if (x + 1 < gameWidth) { xyMatrix[x + 1][y].type = 2; xyMatrix[x + 1][y].state = 3; }
                                } else {
                                    xyMatrix[x][y + 1].type = 3;
                                    xyMatrix[x][y + 1].state = 1;
                                    xyMatrix[x][y].type = 0;
                            }
                            }
                            if (y + 1 >= gameHeight) {
                                if (x - 1 > 0) { xyMatrix[x - 1][y].type = 2; xyMatrix[x - 1][y].state = 3; }
                                xyMatrix[x][y].type = 2;
                                xyMatrix[x][y].state = 3;
                                if (x + 1 < gameWidth) { xyMatrix[x + 1][y].type = 2; xyMatrix[x + 1][y].state = 3; }
                            }
                        }

                        if (xyMatrix[x][y].type == 2 && xyMatrix[x][y].state == 0) {
                            if (y + 1 < gameHeight && xyMatrix[x][y + 1].type != 0) {
                                if (rand() % 2 == 0) {
                                    if (x - 1 >= 0 && xyMatrix[x - 1][y + 1].type == 0) {
                                        xyMatrix[x - 1][y + 1].type = 2;
                                        xyMatrix[x - 1][y + 1].state = 3;
                                        xyMatrix[x][y].type = 0;
                                    }
                                } else {
                                    if (x + 1 < gameWidth && xyMatrix[x + 1][y + 1].type == 0) {
                                        xyMatrix[x + 1][y + 1].type = 2;
                                        xyMatrix[x + 1][y + 1].state = 3;
                                        xyMatrix[x][y].type = 0;
                                    }
                                }
                                if (y + 1 < gameHeight && xyMatrix[x][y + 1].type == 0) {
                                    xyMatrix[x][y + 1].type = 2;
                                    xyMatrix[x][y + 1].state = 3;
                                    xyMatrix[x][y].type = 0;
                                }
                            } else {
                                if (y + 1 < gameHeight) {
                                    xyMatrix[x][y + 1].type = 2;
                                    xyMatrix[x][y + 1].state = 3;
                                    xyMatrix[x][y].type = 0;
                                }
                            }
                        } else {
                            if (y + 1 < gameHeight && xyMatrix[x][y + 1].type == 0) {
                                xyMatrix[x][y + 1].type = xyMatrix[x][y].type;
                                switch (xyMatrix[x][y].type) {
                                    case 1:
                                    case 3:
                                        xyMatrix[x][y + 1].state = 1;
                                        break;
                                    case 2:
                                        xyMatrix[x][y + 1].state = 3;
                                        break;
                                }
                                xyMatrix[x][y].type = 0;
                            }

                            if (y + 1 >= gameHeight && xyMatrix[x][y].type == 1) { xyMatrix[x][y].type = 0; }
                            if (xyMatrix[x][y + 1].type != 0 && xyMatrix[x][y].type == 1) { xyMatrix[x][y].type = 0; }
                        }
                    } else { 
                        xyMatrix[x][y].state -= 1; 
                    }
                }
                attron(COLOR_PAIR(xyMatrix[x][y].type));
                printw("%c", CHARLIST[xyMatrix[x][y].type]);
                attroff(COLOR_PAIR(xyMatrix[x][y].type));
            }
            printw("\n");
        }
    }

    srand((int)time(NULL));

    initscr();
    start_color();
    timeout(0);
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);

    init_pair(0, COLOR_BLACK, COLOR_BLACK);
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_WHITE);

    while (1) {
        if (KeyInput()) {
            break;
        }

        erase();
        UpdateScreen(paused);
        if (paused == 0) {
            xyMatrix[rand() % gameWidth][0].type = mode;
            xyMatrix[rand() % gameWidth][0].state = 1;
        }

        refresh();
        usleep(delay);
    }

    endwin();
    return 0;
}
