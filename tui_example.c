#include "home_tui.h"

#include <string.h>
#include <stdio.h>

int calcelled = 0;
struct ui_label *label;

void onCancelClicked() { calcelled = 1; }

void onTextEntered(struct ui_textbox *textbox){
	strcpy(label->text, textbox->text);
}

int main()
{
	render_init(-1);

	struct ui *ctx = ui_create();
	if (!ctx) {
		fprintf(stderr, "Error: ui_create failed\n");
		return 1;
	}

	ui_add_button(ctx, 1, 1, 5, 2, "OK", NULL);
	ui_add_button(ctx, 1, 6, 7, 2, "Cancel", onCancelClicked);
	ui_add_checkbox(ctx, 1, 10, 1, NULL);
	ui_add_textbox(ctx, 1, 15, 15, 2, "Enter name", onTextEntered);
	ui_add_textbox(ctx, 18, 15, 15, 2, "Enter sirname", NULL);
	label = ui_add_label(ctx, 1, 20, 15, 2, "Label");

	ui_render(ctx); // First render
	while (!calcelled) {
		ui_process_input(ctx, get_keyboard_input());
		render_clear();
		ui_render(ctx);
		render_update();
	}

	render_dispose();
}
