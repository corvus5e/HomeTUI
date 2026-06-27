#include "home_tui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONTROLS_NUM 100

struct ui_style;

typedef int (*ClickBox)(struct ui *ctx, struct ui_box *this);
typedef void (*RenderBox)(const struct ui *ctx, const struct ui_box *, const struct ui_style *);

struct ui_box {
	int	  x, y, w, h;
	char	 *text;
	ClickBox  click;
	RenderBox render;
};

struct ui_button {
	struct ui_box box;
	onButtonClick on_click;
	void	     *on_click_arg;
};

struct ui_checkbox {
	struct ui_box	box;
	bool		is_checked;
	onCheckBoxClick on_click;
};

struct ui_textbox {
	struct ui_box	     box;
	onTextBoxTextEntered on_value_entered;
};

enum ui_mode { NAVIGATE, EDIT };

struct ui {
	// TODO: replace with vector/list
	struct ui_box *ui_controls[CONTROLS_NUM];
	int	       ui_controls_size;
	int	       selected;
	enum ui_mode   mode;
};

bool ui_focus_move(struct ui* ctx, int step) {
        int i = ctx->selected + step;

	while (i >= 0 && i < ctx->ui_controls_size && !ctx->ui_controls[i]->click) {
		i += step;
	}

	if(i >= 0 && i < ctx->ui_controls_size) {
                ctx->selected = i;
                return true;
        }

        return false;
}

struct ui_box* ui_get_selected(const struct ui* ctx){
        if(ctx->selected >= 0 && ctx->selected < ctx->ui_controls_size) {
                return ctx->ui_controls[ctx->selected];
        }
        return nullptr;
}

struct ui_style {
	const UI_CHAR vertical_border;
	const UI_CHAR horizontal_border;
	const UI_CHAR left_upper_corner;
	const UI_CHAR left_bottom_corner;
	const UI_CHAR right_upper_corner;
	const UI_CHAR right_bottom_corner;

	short	      fg_color_id;
	short	      bg_color_id;
	short	      text_fg_color_id;
	short	      text_bg_color_id;
};

struct ui_style idle_style = {CHAR_L('|', L'│'),
			      CHAR_L('-', L'─'),
			      CHAR_L('*', L'╭'),
			      CHAR_L('*', L'╰'),
			      CHAR_L('*', L'╮'),
			      CHAR_L('*', L'╯'),
			      -1,
			      -1,
			      -1,
			      -1};

struct ui_style hovered_style = {CHAR_L('|', L'┃'),
				 CHAR_L('=', L'━'),
				 CHAR_L('*', L'┏'),
				 CHAR_L('*', L'┗'),
				 CHAR_L('*', L'┓'),
				 CHAR_L('*', L'┛'),
				 3,
				 -1,
				 30,
				 -1};

#define TEXT_BOX_INVITE_SYMBOL L'>'
#define TEXT_BOX_CLOSING_INPUT L'|'
#define CHECK_BOX_ON	       "ON"
#define CHECK_BOX_OFF	       "OFF"

int ui_add_control(struct ui *ctx, struct ui_box *box)
{
	ctx->ui_controls[ctx->ui_controls_size] = box;
	return ctx->ui_controls_size++;
}

void	      ui_render_box(const struct ui *ctx, const struct ui_box *, const struct ui_style *);
void	      ui_render_button(const struct ui *ctx, const struct ui_box *, const struct ui_style *);
void	      ui_render_textbox(const struct ui *ctx, const struct ui_box *, const struct ui_style *);
void	      ui_render_checkbox(const struct ui *ctx, const struct ui_box *, const struct ui_style *);

struct ui_box create_ui_box(int x, int y, int w, int h, const char *text, ClickBox, RenderBox);

int	      ui_click_box(struct ui *ctx, struct ui_box *);
int	      ui_click_button(struct ui *ctx, struct ui_box *);
int	      ui_click_checkbox(struct ui *ctx, struct ui_box *);
int	      ui_click_textbox(struct ui *ctx, struct ui_box *);

struct ui    *ui_create(void)
{
	struct ui *ctx = (struct ui *)malloc(sizeof(struct ui));
	if (!ctx)
		return nullptr;

	ctx->ui_controls_size = 0;
	ctx->selected	      = -1;
	ctx->mode	      = NAVIGATE;

	return ctx;
}

void ui_render(const struct ui *ctx)
{
	const struct ui_style *styles[2] = {
	    &idle_style,
	    &hovered_style,
	};

	for (int i = 0; i < ctx->ui_controls_size; ++i) {
		struct ui_box *box = ctx->ui_controls[i];
		box->render(ctx, box, styles[i == ctx->selected]);
	}
}

int ui_process_input_navigate(struct ui *ctx, int key)
{
	switch (key) {
	case 'j':
		ui_focus_move(ctx, 1);
		return PROCESSED;
	case 'k':
		ui_focus_move(ctx, -1);
		return PROCESSED;
	}

	return IGNORED;
}

int ui_process_input_edit(struct ui *ctx, int key)
{
	// NOTE: Currently only textbox is editable
	struct ui_textbox *tb = (struct ui_textbox *)ui_get_selected(ctx);
	if (key == DEL) {
		size_t n = strlen(ui_get_text(&tb->box));
		if (n > 0)
			tb->box.text[n - 1] = '\0';
		return PROCESSED;
	} else if (key >= SPACE && key < DEL) { /* Visual chars*/
		// TODO: Replace this mess with dedicated function
		// TODO: Make static edit string !
		const char *old = ui_get_text(&tb->box);
		const int   n	= strlen(old);
		char	   *new = malloc(n + 2);
		strcpy(new, old);
		new[n]	   = key;
		new[n + 1] = '\0';
		ui_set_text(&tb->box, new);
		return PROCESSED;
	}

	return IGNORED;
}

int ui_process_input(struct ui *ctx, int key)
{
	if (key == LINE_FEED || key == CARRIAGE_RETURN) { // Enter pressed
		struct ui_box *box = ui_get_selected(ctx);
		return box->click(ctx, box);
	}

	switch (ctx->mode) {
	case NAVIGATE:
		return ui_process_input_navigate(ctx, key);
	case EDIT:
		return ui_process_input_edit(ctx, key);
	}

	return IGNORED;
}

struct ui_box *ui_add_box(struct ui *ctx, int x, int y, int w, int h, const char *text)
{
	struct ui_box *box = (struct ui_box *)malloc(sizeof(struct ui_box));
	*box		   = create_ui_box(x, y, w, h, text, nullptr, ui_render_box);
	ui_add_control(ctx, box);
	return box;
}

struct ui_button *
ui_add_button(struct ui *ctx, int x, int y, int w, int h, char *text, onButtonClick on_click, void *arg)
{
	struct ui_button *button = (struct ui_button *)malloc(sizeof(struct ui_button));
	button->box		 = create_ui_box(x, y, w, h, text, ui_click_button, ui_render_button);

	button->on_click     = on_click;
	button->on_click_arg = arg;

	ui_add_control(ctx, (struct ui_box *)button);

	return button;
}

struct ui_checkbox *ui_add_checkbox(struct ui *ctx, int x, int y, int state, onCheckBoxClick on_click)
{
	struct ui_checkbox *check_box = (struct ui_checkbox *)malloc(sizeof(struct ui_checkbox));

	check_box->box =
	    create_ui_box(x, y, 4, 2, state ? CHECK_BOX_ON : CHECK_BOX_OFF, ui_click_checkbox, ui_render_checkbox);
	check_box->is_checked = state;
	check_box->on_click   = on_click;

	ui_add_control(ctx, (struct ui_box *)check_box);

	return check_box;
}

struct ui_textbox *
ui_add_textbox(struct ui *ctx, int x, int y, int w, int h, char *text, onTextBoxTextEntered on_value_entered)
{
	struct ui_textbox *text_box = (struct ui_textbox *)malloc(sizeof(struct ui_textbox));
	text_box->box		    = create_ui_box(x, y, 4, 2, text, ui_click_textbox, ui_render_textbox);
	text_box->on_value_entered  = on_value_entered;
	ui_add_control(ctx, UI_BOX(text_box));

	return text_box;
}

void ui_set_text(struct ui_box *box, const char *new_text)
{
	free(box->text);
	box->text = nullptr;
	if (new_text)
		box->text = strdup(new_text);
}

const char *ui_get_text(const struct ui_box *box)
{
	if (box->text)
		return box->text;
	return "";
}

int	      ui_is_checked(struct ui_checkbox *cb) { return cb->is_checked; }

struct ui_box create_ui_box(int x, int y, int w, int h, const char *text, ClickBox click, RenderBox render)
{
	struct ui_box box = {.x = x, .y = y, .w = w, .h = h, .text = nullptr, .click = click, .render = render};
	ui_set_text(&box, text);
	return box;
}

/**********************
 * CLICK UI FUNCTIONS *
 **********************/
int ui_click_button(struct ui *ctx, struct ui_box *box)
{
	struct ui_button *button = (struct ui_button *)box;
	if (button->on_click)
		button->on_click(button, button->on_click_arg);
	return PROCESSED;
}

int ui_click_checkbox(struct ui *ctx, struct ui_box *box)
{
	struct ui_checkbox *check_box = (struct ui_checkbox *)box;
	check_box->is_checked	      = !check_box->is_checked;
	ui_set_text(UI_BOX(check_box), check_box->is_checked ? CHECK_BOX_ON : CHECK_BOX_OFF);
	if (check_box->on_click)
		check_box->on_click(check_box);

	return PROCESSED;
}

int ui_click_textbox(struct ui *ctx, struct ui_box *box)
{
	struct ui_textbox *text_box = (struct ui_textbox *)box;

	if (ctx->mode == NAVIGATE) {
		ctx->mode = EDIT;
		return PROCESSED_AND_FOCUSED;
	} else { // EDIT mode
		ctx->mode = NAVIGATE;
		if (text_box->on_value_entered)
			text_box->on_value_entered(text_box);
		return PROCESSED_AND_UNFOCUSED;
	}

	return IGNORED;
}

/*********************
 * DRAW UI FUNCTIONS *
 *********************/
void ui_render_box(const struct ui *ctx, const struct ui_box *box, const struct ui_style *style)
{
	int x_start = box->x;
	int x_end   = x_start + box->w;
	int y_start = box->y;
	int y_end   = box->y + box->h;

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

	int str_len = strlen(ui_get_text(&tb->box));
	int x_end   = tb->box.x + str_len + 1;

	for (int x = tb->box.x; x <= x_end; ++x) {
		render_cell(x, tb->box.y + 2, style->horizontal_border);
	}

	render_cell(text_box->x, text_box->y + 1, TEXT_BOX_INVITE_SYMBOL);

	reset_colors();

	render_text(text_box->x + 1, text_box->y + 1, ui_get_text(&tb->box));

	if (ui_get_selected(ctx) == text_box && ctx->mode == EDIT) {
		set_color(style->fg_color_id, style->bg_color_id);
		render_cell(text_box->x + 1 + str_len, text_box->y + 1, TEXT_BOX_CLOSING_INPUT);
		reset_colors();
	}
}

/* Extensions */
void render_block(const UI_CHAR *data, int x, int y, int w, int h)
{
	for (int j = 0; j < h; ++j) {
		for (int i = 0; i < w; ++i) {
			render_cell(x + i, y + j, data[j * w + i]);
		}
	}
}
