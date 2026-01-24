#include <ncurses.h>
#include <stdarg.h>

#include "home_tui.h"

void render_init(int timeout_ms)
{
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	timeout(timeout_ms);

	start_color();
	use_default_colors();
	if(COLORS < 256) {
		fprintf(stderr, "Terminal does not support 256 colors.");
	}
}

void render_update() { refresh(); }
void render_clear() { erase(); }

void render_dispose()
{
	endwin();
}

void set_input_timeout(int input_timeout_milliseconds)
{
	timeout(input_timeout_milliseconds);
}

void get_window_size(int *w, int *h)
{
	*w = COLS;
	*h = LINES;
}

void render_text(int x, int y, const char *text) { mvprintw(y, x, "%s", text); }

void render_ftext(int x, int y, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	move(y, x);
	vw_printw(stdscr, format, args);
	va_end(args);
}

void render_cell(int x, int y, int c) { mvaddch(y, x, c); }

void set_color(short fg, short bg) {
	if(fg == -1 && bg == -1) {
		attrset(COLOR_PAIR(0));
		return;
	}

	static short color_pairs_table[257][257] = {0};
	static short next_id = 1;

	if(color_pairs_table[fg][bg] == 0) {
		if (next_id < COLOR_PAIRS) {
			init_pair(next_id, fg, bg);
			color_pairs_table[fg][bg] = next_id;
			++next_id;
		} else {
			attrset(A_NORMAL);
		}
	}

	attron(COLOR_PAIR(color_pairs_table[fg][bg]));
}

void reset_colors() {
	attrset(COLOR_PAIR(0));
}


int get_keyboard_input() { return getch(); }
