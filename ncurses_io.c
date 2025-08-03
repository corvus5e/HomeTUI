#include <ncurses.h>

#include "home_tui.h"

void render_init(int timeout_ms)
{
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	timeout(timeout_ms);
}

void render_update() { refresh(); }
void render_clear() { erase(); }

void render_dispose()
{
	timeout(-1);
	mvaddstr(LINES - 1, 0, "Press any key to exit ...");
	refresh();
	getch();
	endwin();
}

void render_text(int x, int y, const char *text) { mvprintw(y, x, "%s", text); }

void render_cell(int x, int y, int c) { mvaddch(y, x, c); }

int get_keyboard_input() { return getch(); }
