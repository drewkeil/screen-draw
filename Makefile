# there isn't much to compile...

CFLAGS = -O2 -Wall -Werror -march=native
LINK = -lX11

screen-draw: screen-draw.c drawing.c
	gcc $(CFLAGS) $^ -o $@ $(LINK)

clean:
	rm screen-draw
.PHONY: clean
