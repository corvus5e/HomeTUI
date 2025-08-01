#include <ncurses.h>

#include "HomeTUI.h"

int calcelled = 0;

void onCancelClicked() { calcelled = 1; }

int main()
{
	render_init();

	struct ui *context = ui_create();
	if (!context) {
		fprintf(stderr, "Error: ui_create failed\n");
		return 1;
	}

	ui_add_button(context, 1, 1, 5, 2, "OK", NULL);
	ui_add_button(context, 1, 6, 7, 2, "Cancel", onCancelClicked);
	ui_add_checkbox(context, 1, 10, 1, NULL);
	ui_add_textbox(context, 1, 15, 15, 2, "Enter name", NULL);

	ui_render(context); // First render
	while (!calcelled) {
		ui_process_input(context, getch());
		ui_render(context);
	}

	render_dispose();
}
