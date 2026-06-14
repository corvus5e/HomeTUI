#include "home_tui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONTROLS_NUM 10

struct ui_box {
	int x, y, w, h;
	char *text;
};

struct ui_button {
	struct ui_box box;
	onButtonClick on_click;
	void * on_click_arg;
};

struct ui_checkbox {
	struct ui_box box;
	int is_checked;
	onCheckBoxClick on_click;
};

struct ui_textbox {
	struct ui_box box;
	onTextBoxTextEntered on_value_entered;
};

enum ui_type { BOX, BUTTON, TEXT_BOX, CHECK_BOX, TYPE_COUNT }; //NOTE: do not change order

struct ui_control {
	enum ui_type type;
	struct ui_box *box;
};

enum ui_mode { NAVIGATE, EDIT };

struct ui {
	//TODO: replace with vector/list
	struct ui_control ui_controls[CONTROLS_NUM];
	int ui_controls_size;

	int selected;

	enum ui_mode mode;
};

struct ui_style {
	const UI_CHAR vertical_border;
	const UI_CHAR horizontal_border;
	const UI_CHAR left_upper_corner;
	const UI_CHAR left_bottom_corner;
	const UI_CHAR right_upper_corner;
	const UI_CHAR right_bottom_corner;

	short fg_color_id;
	short bg_color_id;
	short text_fg_color_id;
	short text_bg_color_id;
};

struct ui_style idle_style = {CHAR_L('|', L'│'),CHAR_L('-', L'─'), CHAR_L('*', L'╭'), CHAR_L('*', L'╰'), CHAR_L('*', L'╮'),CHAR_L('*', L'╯'), -1, -1, -1, -1};
struct ui_style hovered_style = {CHAR_L('|', L'┃'), CHAR_L('=', L'━'), CHAR_L('*', L'┏'), CHAR_L('*', L'┗'), CHAR_L('*', L'┓'), CHAR_L('*', L'┛'), 3, -1, 30, -1};

#define TEXT_BOX_INVITE_SYMBOL L'>'
#define TEXT_BOX_CLOSING_INPUT L'|'
#define CHECK_BOX_ON "ON"
#define CHECK_BOX_OFF "OFF"

int ui_add_control(struct ui *ctx, struct ui_box *box, enum ui_type type)
{
	ctx->ui_controls[ctx->ui_controls_size].type = type;
	ctx->ui_controls[ctx->ui_controls_size].box = box;

	return ctx->ui_controls_size++;
}

void ui_render_box(const struct ui *ctx, const struct ui_box *, const struct ui_style *);
void ui_render_button(const struct ui *ctx, const struct ui_box *, const struct ui_style*);
void ui_render_textbox(const struct ui *ctx, const struct ui_box *, const struct ui_style*);
void ui_render_checkbox(const struct ui *ctx, const struct ui_box *, const struct ui_style*);

void ui_click_box(struct ui *ctx, const struct ui_box *);
void ui_click_button(struct ui *ctx, const struct ui_box *);
void ui_click_checkbox(struct ui *ctx, const struct ui_box *);
void ui_click_textbox(struct ui *ctx, const struct ui_box *);

int ui_click_control(struct ui *ctx, const struct ui_control control)
{
	static void (*ui_click_[TYPE_COUNT])(struct ui *, const struct ui_box *) = {
	    ui_click_box, ui_click_button, ui_click_textbox, ui_click_checkbox};
	ui_click_[control.type](ctx, control.box);
	return 1;
}

void ui_render_control(const struct ui *ctx, const struct ui_control control,
		       const struct ui_style* style)
{
	static void (*ui_render_[TYPE_COUNT])(
	    const struct ui *, const struct ui_box *, const struct ui_style*) = {
	    ui_render_box, ui_render_button, ui_render_textbox, ui_render_checkbox};
	ui_render_[control.type](ctx, control.box, style);
}

struct ui *ui_create(void)
{
	struct ui *ctx = (struct ui *)malloc(sizeof(struct ui));
	if (!ctx)
		return NULL;

	ctx->ui_controls_size = 0;
	ctx->selected = -1;
	ctx->mode = NAVIGATE;

	return ctx;
}

void ui_render(const struct ui *ctx)
{
	const struct ui_style *styles[2] = {&idle_style, &hovered_style, };
	for (int i = 0; i < ctx->ui_controls_size; ++i)
		ui_render_control(ctx, ctx->ui_controls[i], styles[i == ctx->selected]);
}

int ui_process_input_navigate(struct ui *ctx, int key) 
{
	switch (key) {
	case 'j':
		if(ctx->selected + 1 < ctx->ui_controls_size)
			ctx->selected += 1;
		return 1;
	case 'k':
		if(ctx->selected > 0)
			ctx->selected -= 1;
		return 1;
	}

	return 0;
}

int ui_process_input_edit(struct ui *ctx, int key)
{
	//NOTE: Currently only textbox is editable
	struct ui_textbox *tb = (struct ui_textbox*)ctx->ui_controls[ctx->selected].box;
	if(key == 127/*Del*/){
		size_t n = strlen(tb->box.text);
		if(n > 0)
			tb->box.text[n-1] = '\0';
		return 1;
	}
	else if(key > 31 && key < 127) { /* Visual chars*/
		size_t n = strlen(tb->box.text);
		tb->box.text[n] = key; //TODO: Use vector here, assume ehough memory for now
		return 1;
	}

	return 0;
}


int ui_process_input(struct ui *ctx, int key)
{
	if(key == 10 || key == 13)
		return ui_click_control(ctx, ctx->ui_controls[ctx->selected]);

	switch (ctx->mode) {
	case NAVIGATE:
			return ui_process_input_navigate(ctx, key);
	case EDIT:
			return ui_process_input_edit(ctx, key);
	}

	return 0;
}

struct ui_box *ui_add_box(struct ui *ctx, int x, int y, int w, int h, const char *text)
{
	struct ui_box *box = (struct ui_box*)malloc(sizeof(struct ui_box));

	box->x = x; box->y = y; box->w = w; box->h = h;
	box->text = NULL;
	ui_set_text(box, text);

	ui_add_control(ctx, box, BOX);

	return box;
}

struct ui_button *ui_add_button(struct ui *ctx, int x, int y, int w, int h, char *text,
		  onButtonClick on_click, void * arg)
{
	struct ui_button *button = (struct ui_button *)malloc(sizeof(struct ui_button));

	struct ui_box box = {x, y, w, h, NULL};
	button->box = box;

	ui_set_text(UI_BOX(button), text);

	button->on_click = on_click;
	button->on_click_arg = arg;

	ui_add_control(ctx, (struct ui_box*)button, BUTTON);

	return button;
}

struct ui_checkbox *ui_add_checkbox(struct ui *ctx, int x, int y, int state, onCheckBoxClick on_click)
{
	struct ui_checkbox *check_box = (struct ui_checkbox *)malloc(sizeof(struct ui_checkbox));

	struct ui_box box = {x, y, 4, 2};
	check_box->box = box;
	check_box->is_checked = state;
	check_box->on_click = on_click;
	ui_set_text(UI_BOX(check_box), state ? CHECK_BOX_ON : CHECK_BOX_OFF);

	ui_add_control(ctx, (struct ui_box*)check_box, CHECK_BOX);

	return check_box;
}

struct ui_textbox *ui_add_textbox(struct ui *ctx, int x, int y, int w, int h, char *text, onTextBoxTextEntered on_value_entered)
{
	struct ui_textbox *text_box = (struct ui_textbox *)malloc(sizeof(struct ui_textbox));

	struct ui_box box = {x, y, w, h};
	text_box->box = box;

	ui_set_text(UI_BOX(text_box), text);
	text_box->on_value_entered = on_value_entered;

	ui_add_control(ctx, UI_BOX(text_box), TEXT_BOX);

	return text_box;
}

void ui_set_text(struct ui_box *box, const char* new_text)
{
	free(box->text);
	box->text = strdup(new_text);
}

const char* ui_get_text(struct ui_box *box)
{
	return box->text;
}

int ui_is_checked(struct ui_checkbox *cb) {
	return cb->is_checked;
}

/**********************
 * CLICK UI FUNCTIONS *
 **********************/
void ui_click_box(struct ui *ctx, const struct ui_box *box)
{
	// Nothing to go for now
}

void ui_click_button(struct ui *ctx, const struct ui_box *box)
{
	struct ui_button *button = (struct ui_button *)box;
	if (button->on_click)
		button->on_click(button, button->on_click_arg);
}

void ui_click_checkbox(struct ui *ctx, const struct ui_box *box)
{
	struct ui_checkbox *check_box = (struct ui_checkbox *)box;
	check_box->is_checked = !check_box->is_checked;
	ui_set_text(UI_BOX(check_box), check_box->is_checked ? CHECK_BOX_ON : CHECK_BOX_OFF);
	if (check_box->on_click)
		check_box->on_click(check_box);
}

//TODO: Rename it, it is not clicked actually
void ui_click_textbox(struct ui *ctx, const struct ui_box *box)
{
	struct ui_textbox *text_box = (struct ui_textbox *)box;

	if(ctx->mode == NAVIGATE) {
		ctx->mode = EDIT;
	}
	else { // EDIT mode
		ctx->mode = NAVIGATE;
		if (text_box->on_value_entered)
			text_box->on_value_entered(text_box);

	}

}

/*********************
 * DRAW UI FUNCTIONS *
 *********************/
void ui_render_box(const struct ui *ctx, const struct ui_box *box, const struct ui_style *style){
	int x_start = box->x;
	int x_end = x_start + box->w;
	int y_start = box->y;
	int y_end = box->y + box->h;

	set_color(style->fg_color_id, style->bg_color_id);

	for (int x = x_start; x <= x_end; ++x) {
		render_cell(x, y_start, style->horizontal_border);
		render_cell(x, y_end, style->horizontal_border);
	}

	render_cell(x_start, y_start, style->left_upper_corner);
	render_cell(x_end, y_start, style->right_upper_corner);
	render_cell(x_start, y_end, style->left_bottom_corner);
	render_cell(x_end, y_end, style->right_bottom_corner);

	for (int y = y_start + 1; y < y_end; ++y) {
		render_cell(x_start, y, style->vertical_border);
		render_cell(x_end, y, style->vertical_border);
	}

	reset_colors();

	render_text(box->x + 1, box->y + 1, box->text);
}

void ui_render_button(const struct ui *ctx, const struct ui_box *box, const struct ui_style *style)
{
	ui_render_box(ctx, box, style);
}

void ui_render_checkbox(const struct ui *ctx, const struct ui_box *check_box, const struct ui_style *style)
{
	ui_render_box(ctx, check_box, style);
}

void ui_render_textbox(const struct ui *ctx, const struct ui_box *text_box, const struct ui_style *style)
{
	const struct ui_textbox *tb = (const struct ui_textbox *)text_box;

	set_color(style->fg_color_id, style->bg_color_id);

	int x_end = tb->box.x + tb->box.w;
	for(int x = tb->box.x; x <= x_end; ++x)
		render_cell(x, tb->box.y + 2, style->horizontal_border);

	render_cell(text_box->x, text_box->y + 1, TEXT_BOX_INVITE_SYMBOL);

	reset_colors();

	//TODO: render last text_box->box.w chars of text
	render_text(text_box->x + 1, text_box->y + 1, tb->box.text);

	if (ctx->selected > 0 && ctx->ui_controls[ctx->selected].box == text_box && //aka isSelected
	    ctx->mode == EDIT) {
		int n = strlen(tb->box.text);
		set_color(style->fg_color_id, style->bg_color_id);
		render_cell(text_box->x + 1 + n, text_box->y + 1, TEXT_BOX_CLOSING_INPUT);
		reset_colors();
	}
}

/* Extensions */
void render_block(const UI_CHAR *data, int x, int y, int w, int h) {
	for(int j =  0; j < h; ++j) {
		for(int i = 0; i < w; ++i) {
			render_cell(x + i, y + j, data[j*w + i]);
		}
	}
}
