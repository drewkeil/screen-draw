# there isn't much to compile...

CFLAGS = -lX11 -O2 -Wall -Werror -march=native

screen-draw: screen-draw.c drawing.c
	gcc $(CFLAGS) $^ -o $@

clean:
	rm screen-draw
.PHONY: clean
