all: select draw

select:
	gcc -o select select.c `pkg-config --cflags --libs gtk+-2.0`

draw: draw.c
	gcc -o draw draw.c `pkg-config --cflags --libs gtk+-2.0`

clean:
	rm ./select
	rm ./draw
