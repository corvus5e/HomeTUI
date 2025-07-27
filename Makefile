src=main.c HomeTUI.c
target=window

main: 
	gcc -std=c11 -g -Wall $(src) -lncurses -o $(target)

run:
	./$(target) 2>std_err.log
