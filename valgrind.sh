#!/bin/bash

valgrind --leak-check=full \
	--show-leak-kinds=all \
	--log-file=valgrind.log \
	--track-origins=yes \
	./tui_example
