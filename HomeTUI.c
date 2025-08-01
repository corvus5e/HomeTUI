#include "HomeTUI.h"

#include <stdlib.h>
#include <string.h>

enum ui_type { LABEL, BUTTON, TEXT_BOX, CHECK_BOX, TYPE_COUNT }; //NOTE: do not change order

struct ui_box {
	int x, y, w, h;
	enum ui_type type;
};

enum ui_mode { NAVIGATE, EDIT };

struct ui {
	//TODO: replace with vector/list
	struct ui_box *ui_controls[10];
	int ui_controls_size;
	int selected;
	enum ui_mode mode;
};

struct ui_style {
	char vertical_border;
	char horizontal_border;
	char corner;
} idle_style = {'|', '-', '+'}, hovered_style = {'|', '=', '*'};

struct ui_button {
	struct ui_box box;
	char *text;
	onButtonClick on_click;
};

struct ui_checkbox {
	struct ui_box box;
	int is_checked;
	onCheckBoxClick on_click;
};

struct ui_textbox {
	struct ui_box box;
	char *text;
	onTextBoxTextEntered on_value_entered;
};

void ui_render_button(const struct ui *ctx, const struct ui_box *, struct ui_style);
void ui_render_checkbox(const struct ui *ctx, const struct ui_box *, struct ui_style);
void ui_render_textbox(const struct ui *ctx, const struct ui_box *, struct ui_style);

void ui_click_button(struct ui *ctx, const struct ui_box *);
void ui_click_checkbox(struct ui *ctx, const struct ui_box *);
void ui_click_textbox(struct ui *ctx, const struct ui_box *);

void ui_click_element(struct ui *ctx, const struct ui_box *box)
{
	static void (*ui_click_[TYPE_COUNT])(struct ui *,
					     const struct ui_box *) = {
	    NULL, ui_click_button, ui_click_textbox, ui_click_checkbox};
	ui_click_[box->type](ctx, box);
}

void ui_render_element(const struct ui *ctx, const struct ui_box *box,
		       struct ui_style style)
{
	static void (*ui_render_[TYPE_COUNT])(
	    const struct ui *, const struct ui_box *, struct ui_style) = {
	    NULL, ui_render_button, ui_render_textbox, ui_render_checkbox};
	ui_render_[box->type](ctx, box, style);
}

struct ui *ui_create(void)
{
	struct ui *context = (struct ui *)malloc(sizeof(struct ui));
	if (!context)
		return NULL;

	context->ui_controls_size = 0;
	context->selected = -1;

	return context;
}

void ui_render(const struct ui *ctx)
{
	render_clear();

	for (int i = 0; i < ctx->ui_controls_size; ++i) {
		ui_render_element(ctx, ctx->ui_controls[i], i == ctx->selected
							   ? hovered_style
							   : idle_style);
	}

	render_update();
}

void ui_process_input_navigate(struct ui *ctx, int key) 
{
	switch (key) {
	case 'j':
		if(ctx->selected + 1 < ctx->ui_controls_size)
			ctx->selected += 1;
		break;
	case 'k':
		if(ctx->selected > 0)
			ctx->selected -= 1;
		break;
	}
}

void ui_process_input_edit(struct ui *ctx, int key)
{
	//NOTE: Currently only textbox is editable
	struct ui_textbox *tb = (struct ui_textbox*)ctx->ui_controls[ctx->selected];
	if(key == 127/*Del*/){
		size_t n = strlen(tb->text);
		if(n > 0)
			tb->text[n-1] = '\0';
	}
	else if(key > 31 && key < 127) { /* Visual chars*/
		size_t n = strlen(tb->text);
		tb->text[n] = key; //TODO: Use vector here, assume ehough memory for now
	}

}


void ui_process_input(struct ui *ctx, int key)
{
	if(key == 10 || key == 13) {
		ui_click_element(ctx, ctx->ui_controls[ctx->selected]);
		return;
	}

	switch (ctx->mode) {
	case NAVIGATE:
			ui_process_input_navigate(ctx, key);
		break;
	case EDIT:
			ui_process_input_edit(ctx, key);
		break;
	}

}


int ui_add_button(struct ui *context, int x, int y, int w, int h, char *text,
		  onButtonClick on_click)
{
	struct ui_button *button = (struct ui_button *)malloc(sizeof(struct ui_button));

	struct ui_box box = {x, y, w, h};
	button->box = box;
	button->box.type = BUTTON;

	button->text = text; //TODO: copy, for now usilg litterals;
	button->on_click = on_click;

	//TODO: Move to a separate function, like add_element
	context->ui_controls[context->ui_controls_size] = (struct ui_box*)button;

	return context->ui_controls_size++;
}

int ui_add_checkbox(struct ui *context, int x, int y, int state, onCheckBoxClick on_click)
{
	struct ui_checkbox *check_box = (struct ui_checkbox *)malloc(sizeof(struct ui_button));

	struct ui_box box = {x, y, 4, 2};
	check_box->box = box;
	check_box->box.type = CHECK_BOX;

	check_box->on_click = on_click;

	context->ui_controls[context->ui_controls_size] = (struct ui_box*)check_box;

	return context->ui_controls_size++;
}

int ui_add_textbox(struct ui *ctx, int x, int y, int w, int h, char *text, onTextBoxTextEntered on_value_entered)
{
	struct ui_textbox *text_box = (struct ui_textbox *)malloc(sizeof(struct ui_textbox));

	struct ui_box box = {x, y, w, h};
	text_box->box = box;
	text_box->box.type = TEXT_BOX;

	text_box->text = (char*)malloc(sizeof(char)*100);//text;
	strcpy(text_box->text, text);
	text_box->on_value_entered = on_value_entered;

	ctx->ui_controls[ctx->ui_controls_size] = (struct ui_box*)text_box;

	return ctx->ui_controls_size++;
}

/**********************
 * CLICK UI FUNCTIONS *
 **********************/
void ui_click_button(struct ui *ctx, const struct ui_box *box)
{
	struct ui_button *bd = (struct ui_button *)box;
	if (bd->on_click)
		bd->on_click();
}

void ui_click_checkbox(struct ui *ctx, const struct ui_box *box)
{
	struct ui_checkbox *cd = (struct ui_checkbox *)box;
	cd->is_checked = !cd->is_checked;
	if (cd->on_click)
		cd->on_click(cd->is_checked);
}

//TODO: Rename it, it is not clicked actually
void ui_click_textbox(struct ui *ctx, const struct ui_box *box)
{
	struct ui_textbox *tb = (struct ui_textbox *)box;

	if(ctx->mode == NAVIGATE) {
		ctx->mode = EDIT;
	}
	else { // EDIT mode
		ctx->mode = NAVIGATE;
		if (tb->on_value_entered)
			tb->on_value_entered(tb->text);

	}

}


/*********************
 * DRAW UI FUNCTIONS *
 *********************/
void ui_render_box(const struct ui *ctx, const struct ui_box *button, struct ui_style style){
	int x_start = button->x;
	int x_end = x_start + button->w;
	int y_start = button->y;
	int y_end = button->y + button->h;

	for (int x = x_start; x <= x_end; ++x) {
		render_cell(x, y_start, style.horizontal_border);
		render_cell(x, y_end, style.horizontal_border);
	}

	render_cell(x_start, y_start, style.corner);
	render_cell(x_end, y_start, style.corner);
	render_cell(x_start, y_end, style.corner);
	render_cell(x_end, y_end, style.corner);

	for (int y = y_start + 1; y < y_end; ++y) {
		render_cell(x_start, y, style.vertical_border);
		render_cell(x_end, y, style.vertical_border);
	}
}

void ui_render_button(const struct ui *ctx, const struct ui_box *button, struct ui_style style)
{
	ui_render_box(ctx, button, style);
	const struct ui_button* bd = (const struct ui_button*)button;
	render_text(button->x + 1, button->y + 1, bd->text);
}

void ui_render_checkbox(const struct ui *ctx, const struct ui_box *check_box, struct ui_style style)
{
	ui_render_box(ctx, check_box, style);
	const struct ui_checkbox *cb = (const struct ui_checkbox *)check_box;
	render_text(check_box->x + 1, check_box->y + 1,
		    cb->is_checked ? "ON" : "OFF");
}

void ui_render_textbox(const struct ui *ctx, const struct ui_box *text_box, struct ui_style style)
{
	ui_render_box(ctx, text_box, style);
	const struct ui_textbox *tb = (const struct ui_textbox *)text_box;
	//TODO: render last text_box->box.w chars of text
	render_text(text_box->x + 1, text_box->y + 1, tb->text);

	if(ctx->mode == EDIT) {
		int n = strlen(tb->text);
		render_cell(text_box->x + 1 + n, text_box->y + 1, '|');
	}
}
