# HomeTUI Build Configuration

# OS Detection
UNAME_S := $(shell uname -s)

# Compiler defaults
CC ?= cc
COMPILE_FLAGS += -std=c11 -Wall

# Ncurses Discovery
ifeq ($(UNAME_S),Darwin)
    # MacOS Logic (Homebrew)
    ifeq ($(shell uname -m),arm64)
        BREW_PREFIX := /opt/homebrew
    else
        BREW_PREFIX := /usr/local
    endif

    NCURSES_CONFIG := $(BREW_PREFIX)/opt/ncurses/bin/ncursesw6-config

    # Use the config tool if found, otherwise use a safe hardcoded fallback
    ifneq ($(wildcard $(NCURSES_CONFIG)),)
        NCURSES_CFLAGS := $(shell $(NCURSES_CONFIG) --cflags)
        NCURSES_LIBS   := $(shell $(NCURSES_CONFIG) --libs)
    else
        NCURSES_CFLAGS := -I$(BREW_PREFIX)/opt/ncurses/include
        NCURSES_LIBS   := -L$(BREW_PREFIX)/opt/ncurses/lib -lncursesw
    endif
else
    # Linux and others
    NCURSES_CFLAGS := $(shell ncursesw6-config --cflags 2>/dev/null || echo "-D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600")
    NCURSES_LIBS   := $(shell ncursesw6-config --libs 2>/dev/null || echo "-lncursesw")
endif
