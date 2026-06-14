#include <locale.h>
#include <ncurses.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "home_tui.h"

#define RET_LOG(msg, ret_value) \
	fprintf(stderr, (msg)); \
	return (ret_value);

#define IN_RANGE(val, min, max) \
	((val) >= (min) && (val) <= (max))

void render_init(int timeout_ms)
{
	setlocale(LC_ALL, "");
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	timeout(timeout_ms);

	start_color();
	use_default_colors();
	if(COLORS < 256) {
		fprintf(stderr, "Terminal does not support 256 colors.");
	}
}

void render_update() { refresh(); }
void render_clear() { erase(); }

void render_dispose()
{
	endwin();
}

void set_input_timeout(int input_timeout_milliseconds)
{
	timeout(input_timeout_milliseconds);
}

void get_window_size(int *w, int *h)
{
	*w = COLS;
	*h = LINES;
}

void render_text(int x, int y, const char *text)
{
#ifdef USE_UTF8 //TODO: Handle some how both char and wchar_t
	mvprintw(y, x, "%s", text);
#else
	mvprintw(y, x, "%s", text);
#endif
}

void render_ftext(int x, int y, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	move(y, x);
	vw_printw(stdscr, format, args);
	va_end(args);
}

void render_cell(int x, int y, UI_CHAR c) {
 #if USE_UTF8
 	cchar_t complex_char = {0};
 	setcchar(&complex_char, &c, WA_NORMAL, 0, NULL);
 	mvadd_wch(y, x, &complex_char);
 #else
      mvaddch(y, x, c);
#endif
}

void set_color(short fg, short bg) {
	if(fg == -1 && bg == -1) {
		attrset(COLOR_PAIR(0));
		return;
	}

	static short color_pairs_table[257][257] = {0};
	static short next_id = 1;

	if(color_pairs_table[fg][bg] == 0) {
		if (next_id < COLOR_PAIRS) {
			init_pair(next_id, fg, bg);
			color_pairs_table[fg][bg] = next_id;
			++next_id;
		} else {
			attrset(A_NORMAL);
		}
	}

	attron(COLOR_PAIR(color_pairs_table[fg][bg]));
}

void reset_colors() {
	attrset(COLOR_PAIR(0));
}


int get_keyboard_input() { return getch(); }

/* Extensions */
struct TextureAtlas {
	const char *data;
	int width;
	int height;
	int number;
	UI_CHAR *textures;
};

struct TextureAtlas* allocate_texture_atlas(int n, int w, int h) {
	struct TextureAtlas *t = malloc(sizeof(struct TextureAtlas));
	//TODO: Add checks for range, add MAX LIMITS defines
	t->width = w;
	t->height = h;
	t->number = n;
	t->textures = malloc(sizeof(UI_CHAR)*w*h*n);
	return t;
}

bool read_figlet_texture_metadata(FILE *f, int *number, int *width, int *height);

bool read_textures(FILE *f, struct TextureAtlas *t);

const struct TextureAtlas* load_figlet_texture(const char *path) {
	FILE *f = fopen(path, "r");
	if(!f) {
		RET_LOG("Failed to open figlet texture file\n", NULL);
	}

	int width, height, number;
	if(!read_figlet_texture_metadata(f, &number, &width, &height)) {
		RET_LOG("Failed to load figlet texture\n", NULL);
	}

	struct TextureAtlas *t = allocate_texture_atlas(number, width, height);
	if(!t) {
		RET_LOG("Failed to alloc textures\n", NULL);
	}

	read_textures(f, t);

	//TODO: Reuse CLEANUP pattern as mentioned in read_figlet_texture_metadata,
	// We need to close file on errors
	fclose(f);

	return t;
}

void get_texture_dims(const struct TextureAtlas* t, int *number, int *width, int *height) {
	if (!t || !number || !width || !height)
		return;

	*number = t->number;
	*width = t->width;
	*height = t->height;
}

UI_CHAR* get_texture(const struct TextureAtlas* t, int n) {
	if(!t || !IN_RANGE(n, 0, t->number))
		return NULL;

	const size_t area = t->width * t->height;
	return t->textures + n*area;
}

/* Helper functions impl*/

bool read_figlet_texture_metadata(FILE *f, int *number, int *width, int *height) {
	char *metadata = NULL;
	size_t n = 0;
	ssize_t nread = getline(&metadata, &n, f);
	if(nread == -1) {
		RET_LOG("Failed to read metadata from figlet texture file\n", false);
	}

	if(sscanf(metadata, "%d %d %d", width, height, number) != 3) {
		RET_LOG("Failed to parse metadata from figlet texture file\n", false);
	}

	if(!IN_RANGE(*width, 1, 20) || !IN_RANGE(*height, 1, 20) || !IN_RANGE(*number, 1, 40)) {
		RET_LOG("Metadata in figlet texture file is too large\n", false);
	}

	free(metadata);

	//TODO: Reuse pattern from: https://softwareengineering.stackexchange.com/questions/374143/how-to-handle-repetitive-mallocs-frees-in-a-dry-way-in-c

	return true;
}

bool read_textures(FILE *f, struct TextureAtlas *t) {
	char *line = NULL;
	size_t n = 0;
	ssize_t nread ;
	for (int j = 0; j < t->height; ++j) {
		if ((nread = getline(&line, &n, f)) != -1) {
			mbstate_t state;
			memset(&state, 0, sizeof(state));
			const char *src = line;
			for (int i = 0; i < t->number; ++i) {
				UI_CHAR *dst = get_texture(t, i) + j*t->width;
				size_t wchars_read = mbsrtowcs(dst, &src, t->width, &state);
				if (wchars_read == (size_t)-1) {
					RET_LOG("Failed to read multi-byte string -1\n", false);
				}
				if (wchars_read == (size_t)-2) {
					RET_LOG("Failed to read multi-byte string -2", false);
					return 0;
				}
			}
		}
	}

	free(line);

	return true;
}

