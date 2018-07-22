#include "stdafx.h"
#include "inout.h"
#undef  MOUSE_MOVED
#include "curses.h" //include pdcurses

short win_size_x = 80;
short win_size_y = 60;
WINDOW * win_main;
WINDOW * win_progress;

//Turn on color attribute
int bm_wattron(int color) {
	return wattron(win_main, COLOR_PAIR(color));
}

//Turn on color attribute
int bm_wattroff(int color) {
	return wattroff(win_main, COLOR_PAIR(color));
}

//print
int bm_wprintw(const char * output, ...) {
	va_list args;
	va_start(args, output);
	return vw_printw(win_main, output, args);
	va_end(args);
}

//Turn on color attribute
int bm_wattronP(int color) {
	return wattron(win_progress, COLOR_PAIR(color));
}

//Turn on color attribute
int bm_wattroffP(int color) {
	return wattroff(win_progress, COLOR_PAIR(color));
}

//print
int bm_wprintwP(const char * output,...) {
	va_list args;
	va_start(args, output);
	return vw_printw(win_progress, output, args);
	va_end(args);
}

// init screen
void bm_init() {
	initscr();
	raw();
	cbreak();		// не использовать буфер для getch()
	noecho();		// не отображать нажатия клавиш
	curs_set(0);	// убрать курсор
	start_color();	// будет всё цветное 			

	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(4, COLOR_RED, COLOR_BLACK);
	init_pair(6, COLOR_CYAN, COLOR_BLACK);
	init_pair(7, COLOR_WHITE, COLOR_BLACK);
	init_pair(9, 9, COLOR_BLACK);
	init_pair(10, 10, COLOR_BLACK);
	init_pair(11, 11, COLOR_BLACK);
	init_pair(12, 12, COLOR_BLACK);
	init_pair(14, 14, COLOR_BLACK);
	init_pair(15, 15, COLOR_BLACK);
	init_pair(25, 15, COLOR_BLUE);

	win_main = newwin(LINES - 2, COLS, 0, 0);
	scrollok(win_main, true);
	keypad(win_main, true);
	nodelay(win_main, true);
	win_progress = newwin(3, COLS, LINES - 3, 0);
	leaveok(win_progress, true);
}

void refreshMain(){
wrefresh(win_main);
}
void refreshProgress(){
wrefresh(win_progress);
}
void clearProgress(){
wclear(win_progress);
}

int bm_wgetchMain() {
	return wgetch(win_main);
}
void boxProgress(){
box(win_progress, 0, 0);
}
void bm_wmoveP() {
	wmove(win_progress, 1, 1);
};
