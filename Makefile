src=main.c home_tui.c ncurses_io.c
target=tui_example

main:
	gcc -std=c11 -g -Wall $(src) -lncurses -o $(target)

run:
	./$(target) 2>std_err.log
