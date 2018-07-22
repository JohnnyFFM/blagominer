#include "stdafx.h"
#include "error.h"

void ShowMemErrorExit(void)
{
	Log("\n!!! Error allocating memory");
	bm_wattron(12);
	bm_wprintw("\nError allocating memory\n", 0);
	bm_wattroff(12);
	refreshMain();
	system("pause");
	exit(-1);
}