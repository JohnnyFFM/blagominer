#pragma once
extern short win_size_x;
extern short win_size_y;

void bm_init();
// main window functions
int bm_wattron(int color);
int bm_wattroff(int color);
int bm_wprintw(const char * output,...);
void refreshMain();
int bm_wgetchMain(); //get input vom main window

// progress window function
int bm_wattronP(int color);
int bm_wattroffP(int color);
int bm_wprintwP(const char * output,...);
void refreshProgress();
void clearProgress();
void bm_wmoveP();
void boxProgress();
