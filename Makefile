src=tui_example.c home_tui.c ncurses_io.c
target=tui_example

main:
	gcc -std=c11 -g -Wall $(src) -o $(target) $(shell ncursesw6-config --cflags --libs)

run:
	./$(target) 2>std_err.log
