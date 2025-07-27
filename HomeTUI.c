#include "HomeTUI.h"

#include <stdio.h>
#include <stdlib.h>

enum ui_type { LABEL, BUTTON, TEXT_BOX, CHECK_BOX };

struct ui_box {
	int x, y, w, h;
	enum ui_type type;
	void *data;
};

struct ui {
	//TODO: replace with vector/list
	struct ui_box *ui_controls[10];
	int ui_controls_size;
	int selected;
};

enum ui_mode { HOVER, EDIT, IDLE };

struct ui_style {
	char vertical_border;
	char horizontal_border;
} idle_style, hovered_style, edit_style;

struct ui_button_data {
	char *text;
	onButtonClick onClick;
};

struct ui_checkbox_data {
	int isChecked;
	onCheckBoxClick onClick;
};

void ui_click_element(void *data, enum ui_type);

void ui_render_button(const struct ui_box *, struct ui_style);
void ui_render_checkbox(const struct ui_box *, struct ui_style);

struct ui* ui_create(void)
{
	struct ui *context = (struct ui*)malloc(sizeof(struct ui));
	if(!context)
		return NULL;

	context->ui_controls_size = 0;
	context->selected = -1;

	idle_style.vertical_border = '|';
	idle_style.horizontal_border = '-';

	hovered_style.vertical_border = '*';
	hovered_style.horizontal_border = '*';

	return context;
}

void ui_render(const struct ui *context)
{
	render_clear();

	for (int i = 0; i < context->ui_controls_size; ++i) {
		enum ui_mode mode = i == context->selected ? HOVER : IDLE;
		struct ui_style style =
		    mode == IDLE ? idle_style : hovered_style;
		struct ui_box *current = context->ui_controls[i];

		switch (current->type) {
		case LABEL:
		case BUTTON:
			ui_render_button(current, style);
		case TEXT_BOX:
			break;
		case CHECK_BOX:
			ui_render_checkbox(current, style);
			break;
		}
	}

	render_update();
}

void ui_process_input(struct ui *context, int key)
{
	int inc = 0;
	int click = 0;
	switch (key) {
	case 'j':
		inc = 1;
		break;
	case 'k':
		inc = -1;
		break;
	case 13:
	case 10:
		click = 1;
		break;
	}

	if (context->selected + inc >= 0 &&
	    context->selected + inc < context->ui_controls_size) {
		context->selected += inc;
		if (click) {
			struct ui_box *curr_element = context->ui_controls[context->selected];
			ui_click_element(curr_element->data, curr_element->type);
		}
	}
}

void ui_click_element(void *data, enum ui_type type) {
	switch (type) {
	case LABEL:
		break;
	case BUTTON:;
		struct ui_button_data *bd = (struct ui_button_data *)data;
		if (bd->onClick)
			bd->onClick();
		break;
	case TEXT_BOX:
		break;
	case CHECK_BOX:;
		struct ui_checkbox_data *cd = (struct ui_checkbox_data *)data;
		cd->isChecked = !cd->isChecked;
		if(cd->onClick)
			cd->onClick(cd->isChecked);
		break;
	}
}

int ui_add_button(struct ui *context, int x, int y, int w, int h, char *text,
		  onButtonClick f)
{
	struct ui_box *box = (struct ui_box *)malloc(sizeof(struct ui_box));

	box->x = x;
	box->y = y;
	box->w = w;
	box->h = h;
	box->type = BUTTON;

	struct ui_button_data *bd = (struct ui_button_data *)malloc(sizeof(struct ui_button_data));
	bd->text = text; //TODO: copy, for now usilg litterals;
	bd->onClick = f;

	box->data = bd;

	context->ui_controls[context->ui_controls_size] = box;

	return context->ui_controls_size++;
}

int ui_add_checkbox(struct ui *context, int x, int y, int state, onCheckBoxClick f)
{
	struct ui_box *box = (struct ui_box *)malloc(sizeof(struct ui_box));

	box->x = x;
	box->y = y;
	box->w = 4;
	box->h = 2;
	box->type = CHECK_BOX;

	struct ui_checkbox_data *cd = (struct ui_checkbox_data *)malloc(sizeof(struct ui_button_data));
	cd->onClick = f;
	box->data = cd;

	context->ui_controls[context->ui_controls_size] = box;

	return context->ui_controls_size++;
}

/*********************
 * DRAW UI FUNCTIONS *
 *********************/

void ui_render_box(const struct ui_box *button, struct ui_style style){
	int x_start = button->x;
	int x_end = x_start + button->w;
	int y_start = button->y;
	int y_end = button->y + button->h;

	for (int x = x_start; x <= x_end; ++x) {
		render_cell(x, y_start, style.horizontal_border);
		render_cell(x, y_end, style.horizontal_border);
	}

	for (int y = y_start; y <= y_end; ++y) {
		render_cell(x_start, y, style.vertical_border);
		render_cell(x_end, y, style.vertical_border);
	}
}

void ui_render_button(const struct ui_box *button, struct ui_style style)
{
	ui_render_box(button, style);
	struct ui_button_data* bd = button->data;
	render_text(button->x + 1, button->y + 1, bd->text);
}

void ui_render_checkbox(const struct ui_box *check_box, struct ui_style style) {
	ui_render_box(check_box, style);
	struct ui_checkbox_data* cd = check_box->data;
	const char *t = cd->isChecked ? "ON" : "OFF";
	render_text(check_box->x + 1, check_box->y + 1, t);
}
