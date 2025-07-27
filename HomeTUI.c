#include "HomeTUI.h"

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

enum ui_mode { HOVER, EDIT, IDLE };

struct ui_style {
	char vertical_border;
	char horizontal_border;
} idle_style, hovered_style, edit_style;

void ui_render_button(const struct box *, struct ui_style);

void ui_init(struct ui *context)
{
	context->ui_controls_size = 0;
	context->selected = -1;

	idle_style.vertical_border = '|';
	idle_style.horizontal_border = '-';

	hovered_style.vertical_border = '*';
	hovered_style.horizontal_border = '*';

}

void ui_render(const struct ui *context)
{
	erase();

	for (int i = 0; i < context->ui_controls_size; ++i) {
		enum ui_mode mode = i == context->selected ? HOVER : IDLE;
		struct ui_style style = mode == IDLE ? idle_style : hovered_style;
		switch (context->ui_controls[i]->type) {
		case LABEL:
		case BUTTON:
			ui_render_button(context->ui_controls[i], style);
		case TEXT_BOX:
		case CHECK_BOX:
			break;
		}
	}

	refresh();
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
	case KEY_ENTER:
	case 10:
		click = 1;
		break;
	}

	if (context->selected + inc >= 0 &&
	    context->selected + inc < context->ui_controls_size) {
		context->selected += inc;
		if (click)
			context->ui_controls[context->selected]->onClick();
	}
}

int ui_add_button(struct ui *context, int x, int y, int w, int h, char *text,
		  onClickFn f)
{
	struct box *box = (struct box *)malloc(sizeof(struct box));
	box->x = x;
	box->y = y;
	box->w = w;
	box->h = h;
	box->onClick = f;
	struct text_data *button_data =
	    (struct text_data *)malloc(sizeof(struct text_data));
	if (!button_data) {
		fprintf(stderr, "Malloc failed");
		exit(1);
	}
	button_data->text = text; // NOTE: pointer to literall data for now
	box->data = button_data;
	context->ui_controls[context->ui_controls_size] = box;
	return context->ui_controls_size++;
}

/* RENDER FUNCTIONS*/

void ui_render_button(const struct box *button, struct ui_style style)
{
	int x_start = button->x;
	int x_end = x_start + button->w;
	int y_start = button->y;
	int y_end = button->y + button->h;

	for (int x = x_start; x <= x_end; ++x) {
		mvaddch(y_start, x, style.horizontal_border);
		mvaddch(y_end, x, style.horizontal_border);
	}

	for (int y = y_start; y <= y_end; ++y) {
		mvaddch(y, x_start, style.vertical_border);
		mvaddch(y, x_end, style.vertical_border);
	}

	char *text = ((struct text_data *)(button->data))->text;
	mvprintw(y_start + 1, x_start + 1, "%s", text);
}
