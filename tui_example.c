#include "home_tui.h"

#include <stdio.h>
#include <string.h>

int calcelled = 0;

struct MyTUI {
	struct ui_box *label;
	struct ui_textbox *name_text_box;
        char name_buf[100];
        char label_buf[100];
};

void onCancelClicked(void *arg) { calcelled = 1; }

void onSaveClicked(void *arg)
{
	struct MyTUI *args = (struct MyTUI *)(arg);
        strcpy(args->label_buf, args->name_buf);
}

void onNameEntered(void *arg) {}

int main()
{
	render_init(-1);
	const struct TextureAtlas *textures = load_figlet_texture("assets/home-tui-logo.txt");
	int w, h, n;
	get_texture_dims(textures, &n, &w, &h);

	struct ui *ctx = ui_create();
	if (!ctx) {
		fprintf(stderr, "Error: ui_create failed\n");
		return 1;
	}

	struct MyTUI args;

	render_block(get_texture(textures, 0), 4, 1, w, h);
	ui_add_box(ctx, 4, 4, 6, CONST_STR_ARG("Name:"));
	args.name_text_box = ui_add_textbox(ctx, 12, 4, 15, BUF_STR_ARG(args.name_buf, sizeof(args.name_buf), ""), onNameEntered, nullptr);

        ui_add_box(ctx, 4, 7, 15, CONST_STR_ARG("Add sparkles"));
	ui_add_checkbox(ctx, 22, 7, 1, nullptr, nullptr);

	args.label = ui_add_box(ctx, 4, 10, 6, BUF_STR_ARG(args.label_buf, sizeof(args.label_buf), ""));
//✨, TODO: Add add_... macros wrappers
//          Add UTF-8 compatible strlen (for render)
	ui_add_button(ctx, 4, 22, 7, CONST_STR_ARG("Save"), onSaveClicked, &args);
	ui_add_button(ctx, 12, 22, 7, CONST_STR_ARG("Exit"), onCancelClicked, nullptr);

	ui_render(ctx); // First render
	while (!calcelled) {
		ui_process_input(ctx, get_keyboard_input());
		render_clear();
		ui_render(ctx);
	        render_block(get_texture(textures, 0), 4, 1, w, h);
		render_update();
	}

	render_dispose();
}
