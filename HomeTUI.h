#ifndef _HOME_TUI_H_
#define _HOME_TUI_H_

struct ui_checkbox;
struct ui_textbox;
struct ui_button;

typedef void (*onButtonClick)(struct ui_button *);
typedef void (*onCheckBoxClick)(struct ui_checkbox *);
typedef void (*onTextBoxTextEntered)(struct ui_textbox *);

struct ui_box {
	int x, y, w, h;
};

struct ui_label {
	struct ui_box box;
	char *text;
};

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

struct ui* ui_create(void);

struct ui_label    *ui_add_label(struct ui *ctx, int x, int y, int w, int h, char *text);
struct ui_button   *ui_add_button(struct ui *ctx, int x, int y, int w, int h, char *text, onButtonClick);
struct ui_checkbox *ui_add_checkbox(struct ui *ctx, int x, int y, int state, onCheckBoxClick);
struct ui_textbox  *ui_add_textbox(struct ui *ctx, int x, int y, int w, int h, char *initial_text, onTextBoxTextEntered);

void ui_render(const struct ui* ctx);
void ui_process_input(struct ui *ctx, int key);

/* External render function */
void render_init();
void render_update();
void render_clear();
void render_dispose();
void render_text(int x, int y, const char *text);
void render_cell(int x, int y, int c);

#endif
