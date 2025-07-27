#ifndef _HOME_TUI_H_
#define _HOME_TUI_H_

enum ui_type { LABEL, BUTTON, TEXT_BOX, CHECK_BOX };

typedef void (*onClickFn)(void);

struct box {
	int x, y, w, h;
	enum ui_type type;
	void *data;
	onClickFn onClick;
};

struct text_data {
	char *text;
};

struct ui {
	struct box *ui_controls[10];
	int ui_controls_size;
	int selected;
};

void ui_init(struct ui* context);
void ui_render(const struct ui* context);
void ui_process_input(struct ui *context, int key);

int ui_add_button(struct ui *context, int x, int y, int w, int h, char *text, onClickFn);

#endif
