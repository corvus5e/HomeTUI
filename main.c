#include <ncurses.h>

#include "HomeTUI.h"

void render_init()
{
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	timeout(-1);
}

void render_dispose()
{
	timeout(-1);
	mvaddstr(LINES - 1, 0, "Press any key to exit ...");
	refresh();
	getch();
	endwin();
}

int calcelled = 0;

void onCancelClicked() {
	calcelled = 1;
}

int main() {

	render_init();

	struct ui context;
	ui_init(&context);
	
	ui_add_button(&context, 1, 1, 5, 2, "OK", NULL);
	ui_add_button(&context, 1, 6, 7, 2, "Cancel", onCancelClicked);


	int c;
	ui_render(&context); // First render
	while(!calcelled) {
		if((c = getch())< 0)
			continue;

		ui_process_input(&context, c);
		ui_render(&context);
	}

	render_dispose();
}
