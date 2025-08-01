#ifndef _HOME_TUI_H_
#define _HOME_TUI_H_

typedef void (*onButtonClick)(void);
typedef void (*onCheckBoxClick)(int isChecked);
typedef void (*onTextBoxTextEntered)(char *text);

struct ui* ui_create(void);
void ui_render(const struct ui* ctx);
void ui_process_input(struct ui *ctx, int key);

int ui_add_button(struct ui *ctx, int x, int y, int w, int h, char *text, onButtonClick);
int ui_add_checkbox(struct ui *ctx, int x, int y, int state, onCheckBoxClick);
int ui_add_textbox(struct ui *ctx, int x, int y, int w, int h, char *initial_text, onTextBoxTextEntered);

/* External render function */
void render_init();
void render_update();
void render_clear();
void render_dispose();
void render_text(int x, int y, const char *text);
void render_cell(int x, int y, int c);

#endif
