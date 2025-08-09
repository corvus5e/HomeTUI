#include "home_tui.h"

#include <string.h>
#include <stdio.h>

int calcelled = 0;

struct OnSaveArgs {
	struct ui_label *label;
	struct ui_textbox *name;
};

void onCancelClicked(struct ui_button *button, void *arg) { calcelled = 1; }

void onSaveClicked(struct ui_button *button, void *arg)
{
	struct OnSaveArgs *args = (struct OnSaveArgs*)(arg);
	strcpy(args->label->text, args->name->text);
	calcelled = 1;
}


int main()
{
	render_init(-1);

	struct ui *ctx = ui_create();
	if (!ctx) {
		fprintf(stderr, "Error: ui_create failed\n");
		return 1;
	}

	struct OnSaveArgs args;

	args.label = ui_add_label(ctx, 1, 20, 35, 2, "Label");
	ui_add_button(ctx, 1, 1, 5, 2, "Save", onSaveClicked, &args);
	ui_add_button(ctx, 1, 6, 7, 2, "Cancel", onCancelClicked, NULL);
	ui_add_checkbox(ctx, 1, 10, 1, NULL);
	args.name = ui_add_textbox(ctx, 1, 15, 15, 2, "Enter name", NULL);
	ui_add_textbox(ctx, 18, 15, 15, 2, "Enter sirname", NULL);

	ui_render(ctx); // First render
	while (!calcelled) {
		ui_process_input(ctx, get_keyboard_input());
		render_clear();
		ui_render(ctx);
		render_update();
	}

	render_dispose();
}
