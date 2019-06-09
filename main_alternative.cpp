#include <ncurses.h>
#include <clocale>
#include <memory.h>
#include "stages.h"

enum Direction {LEFT, RIGHT, UP, DOWN};
enum Tile {DEFAULT, WALL, BOX, GOAL, OUTSIDE, CURR};
enum Pair {P_DEFAULT = 1, P_WALL, P_BOX, P_GOAL, P_OUTSIDE, P_CURR};
// Enum starts with 1 because We can't assign 0 to COLOR_PAIR palette.
struct Pos { int y; int x; Direction heading; };

Pos chk_pos(Direction dir, Pos curr);
void refr_game(WINDOW* w, Pos curr);
void refr_info(WINDOW* w);
int** get_arr(int n);

int* stagep[STAGEN] =
        {(int*)stage1, (int*)stage2, (int*)stage3, (int*)stage4, (int*)stage5};
int heights[STAGEN] = {7, 9, 6, 7, 8};
int widths[STAGEN] = {6, 7, 8, 7, 10};

int step = 0;
int push = 0;
int stage = 0;

int** curr_arr;

int main() {
    curr_arr = get_arr(0);

    setlocale(LC_ALL, ""); // to use unicode

    Pos curr = {2, 2, LEFT}; // y, x, Dir
    WINDOW *game_win;
    WINDOW *info_win;

    initscr();
    keypad(stdscr, TRUE);
    curs_set(0);
    noecho();
    resize_term(40, 40);

    start_color();
    init_pair(P_DEFAULT, COLOR_WHITE, COLOR_WHITE);
    init_pair(P_WALL, COLOR_RED, COLOR_RED);
    init_pair(P_BOX, COLOR_MAGENTA, COLOR_MAGENTA);
    init_pair(P_GOAL, COLOR_YELLOW, COLOR_YELLOW);
    init_pair(P_OUTSIDE, COLOR_BLACK, COLOR_BLACK);
    init_pair(P_CURR, COLOR_WHITE, COLOR_GREEN);

    mvprintw(0, 0, "★PushBox Game☆"); // length = 14
    mvprintw(0, 26, "Press :"); // length = 7
    mvprintw(1, 26, "Q to Exit"); // length = 9
    refresh();

    game_win = newwin(heights[stage], widths[stage]*2, 3, 3);
    wbkgd(game_win, COLOR_PAIR(DEFAULT));
    refr_game(game_win, curr);

    info_win = newwin(20, 15, 6, 26);
    wbkgd(game_win, COLOR_PAIR(DEFAULT));
    mvwprintw(info_win, 0, 0, "STEP : ");
    mvwprintw(info_win, 1, 0, "PUSH : ");
    mvwprintw(info_win, 2, 0, "LEVEL");
    refr_info(info_win);

    refresh();

    int chr = 0;
    Pos chk = curr;
    bool cleared = false;
    while (chr != 'q' && chr != 'Q') {
        chr = getch();

        if (chr == KEY_LEFT) chk = chk_pos(LEFT, curr);
        else if (chr == KEY_RIGHT) chk = chk_pos(RIGHT, curr);
        else if (chr == KEY_UP) chk = chk_pos(UP, curr);
        else if (chr == KEY_DOWN) chk = chk_pos(DOWN, curr);

        int chk_num = curr_arr[chk.y][chk.x];

        if (chk_num == WALL) continue; // heading to wall
        else if (chk_num == DEFAULT || chk_num == GOAL) {
            curr.y = chk.y;
            curr.x = chk.x;
            refr_game(game_win, curr);
            step += 1;
            refr_info(info_win);
        }
        else if (chk_num == BOX) {
            Pos alt_chk = chk_pos(chk.heading, chk);
            int alt_chk_num = curr_arr[alt_chk.y][alt_chk.x];

            if (alt_chk_num == WALL || alt_chk_num == BOX) continue;
            else if (alt_chk_num == DEFAULT || alt_chk_num == GOAL) {
                curr_arr[alt_chk.y][alt_chk.x] = BOX;
                curr_arr[chk.y][chk.x] = DEFAULT;
                curr.y = chk.y;
                curr.x = chk.x;
                refr_game(game_win, curr);
                step += 1;
                push += 1;
                refr_info(info_win);
            }
        }
    }

    mvprintw(1, 0, cleared? "You win!" : "Quit game");
    mvprintw(2, 0, "Press any key to quit game");

    getch();
    delwin(game_win);
    endwin();

    return 0;
}

int** get_arr(int n) {
    int** arr = new int*[heights[n]];
    for(int i = 0; i < heights[n]; ++i)
        arr[i] = new int[widths[n]];

    memcpy(arr, stagep[n], sizeof(*stagep[n]));
    return arr;
}

Pos chk_pos(Direction dir, Pos curr) {
    Pos chk = {0, 0, LEFT};
    switch (dir) {
        case LEFT:
            chk.heading = LEFT;
            chk.y = curr.y;
            chk.x = curr.x-1;
            break;
        case RIGHT:
            chk.heading = RIGHT;
            chk.y = curr.y;
            chk.x = curr.x+1;
            break;
        case UP:
            chk.heading = UP;
            chk.y = curr.y-1;
            chk.x = curr.x;
            break;
        case DOWN:
            chk.heading = DOWN;
            chk.y = curr.y+1;
            chk.x = curr.x;
            break;
    }

    return chk;
}

void refr_game(WINDOW *w, Pos curr) {
    for(int y=0; y < heights[stage]; y++) {
        for(int x=0; x < widths[stage]*2; x++) {
            int n = curr_arr[y][x];
            wattron(w, COLOR_PAIR(n+1));
            char *c = new char;
            sprintf(c, "%d", n);
            mvwprintw(w, y, (x*2), c);
            mvwprintw(w, y, (x*2)+1, c);
            wattroff(w, COLOR_PAIR(n+1));
            delete c;
        }
    }

    wattron(w, COLOR_PAIR(CURR));
    mvwprintw(w, curr.y, (curr.x*2), "C");
    mvwprintw(w, curr.y, (curr.x*2)+1, "C");
    wattron(w, COLOR_PAIR(CURR));

    wrefresh(w);
    refresh();
}

void refr_info(WINDOW *w) {
    char *c = new char;
    sprintf(c, "%d", step);
    mvwprintw(w, 0, 7, c);
    sprintf(c, "%d", push);
    mvwprintw(w, 1, 7, c);
    sprintf(c, "%d", stage+1);
    mvwprintw(w, 2, 6, c);
    delete c;
    wrefresh(w);
}