#ifndef _HOME_TUI_H_
#define _HOME_TUI_H_

#include <stddef.h>

#ifdef USE_UTF8
#include <wchar.h>
#define UI_CHAR wchar_t
#define CHAR_L(ascii_char, utf8_str) utf8_str
#else
#define UI_CHAR char
#define CHAR_L(ascii_char, utf8_str) ascii_char
#endif

struct ui_box;
#define UI_BOX(obj) ((struct ui_box*)(obj))

struct ui_checkbox;
struct ui_textbox;
struct ui_button;

typedef void (*onButtonClick)(struct ui_button *, void * arg);
typedef void (*onCheckBoxClick)(struct ui_checkbox *);
typedef void (*onTextBoxTextEntered)(struct ui_textbox *);

struct ui* ui_create(void);

struct ui_box      *ui_add_box(struct ui *ctx, int x, int y, int w, int h, const char *text);
struct ui_button   *ui_add_button(struct ui *ctx, int x, int y, int w, int h, char *text, onButtonClick, void *);
struct ui_checkbox *ui_add_checkbox(struct ui *ctx, int x, int y, int state, onCheckBoxClick);
struct ui_textbox  *ui_add_textbox(struct ui *ctx, int x, int y, int w, int h, char *initial_text, onTextBoxTextEntered);

void ui_set_text(struct ui_box*, const char*);
const char* ui_get_text(struct ui_box*);

void ui_render(const struct ui* ctx);
/* Returns 1 if the input was handled, 0 - if was ignored or had no handler*/
int ui_process_input(struct ui *ctx, int key);

/*
 * External render and user input functions
*/
void render_init(int input_timeout_milliseconds);
void render_update();
void render_clear();
void render_dispose();

void set_input_timeout(int input_timeout_milliseconds);
void get_window_size(int *w, int *h);

void render_text(int x, int y, const char *text);
void render_ftext(int x, int y, const char *format, ...);

void render_cell(int x, int y, UI_CHAR);

void set_color(short foreground_color_id, short backgroung_color_id);
void reset_colors();

int get_keyboard_input();

/* Some render extensions */
void render_block(const UI_CHAR *data, int x, int y, int w, int h);

/* Figlet-like textures */
struct TextureAtlas;
const struct TextureAtlas* load_figlet_texture(const char *path);
void get_texture_dims(const struct TextureAtlas*, int *number, int *width, int *height);
UI_CHAR* get_texture(const struct TextureAtlas*, int n);
void dispose_textures(struct TextureAtlas *t);

/* Some non-printable key codes */
#define ESC 27
#define RESIZE 0632

/* Color constants */
#define COLOR_DARK_GREEN   22
#define COLOR_MEDIUM_GREEN 34
#define COLOR_BRIGHT_GREEN 46
#define COLOR_NEON_GREEN   118

#endif
