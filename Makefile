CC           := gcc
COMPILE_FLAGS:= -std=c11 -Wall
BUILD_DIR    := ./bin

LIB_SRCS         := home_tui.c ncurses_io.c
LIB_OBJS         := $(LIB_SRCS:%.c=$(BUILD_DIR)/%.o)
LIB_NAME         := home_tui
LIB_TARGET       := lib$(LIB_NAME).a

# OS Detection
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
    # MacOS specific
    NCURSES_FLAGS := -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600
    NCURSES_LIBS  := -lncurses
else
    # Linux and others
    NCURSES_FLAGS := $(shell ncursesw6-config --cflags)
    NCURSES_LIBS  := $(shell ncursesw6-config --libs)
endif

EXAMPLE_SRCS       := tui_example.c
EXAMPLE_TARGET     := tui_example
EXAMPLE_LINK_FLAGS := -L./$(BUILD_DIR) -l$(LIB_NAME) $(NCURSES_LIBS)

# .PHONY tells Make these aren't actual files on disk
.PHONY: build_ASCII build_UTF-8 clean

# Target 1: Standard ASCII build
UTF-8_example: UTF-8_lib
UTF-8_example: example

ASCII_example: ASCII_lib
ASCII_example: example

example:
	$(CC) $(COMPILE_FLAGS) $(NCURSES_FLAGS) $(EXAMPLE_SRCS) -o $(EXAMPLE_TARGET) $(EXAMPLE_LINK_FLAGS)

ASCII_lib: $(LIB_OBJS)
	ar rcs $(BUILD_DIR)/$(LIB_TARGET) $(LIB_OBJS)

# Corrected: Include $(BUILD_DIR)/ in the target pattern to correctly extract the stem
$(LIB_OBJS): $(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(COMPILE_FLAGS) $(NCURSES_FLAGS) -c $< -o $@

# Target 2: Target-specific variable assignment + dependency
UTF-8_lib: COMPILE_FLAGS += -DUSE_UTF8
UTF-8_lib: ASCII_lib

# Optional but helpful helper
clean:
	rm -f $(EXAMPLE_TARGET)
	rm -rdf $(BUILD_DIR)
