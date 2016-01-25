all: server client draw

server: server.c
	gcc -o server server.c `pkg-config --cflags --libs gtk+-2.0`

client: client.c
	gcc -o client client.c

gui: gtk_test.c
	gcc -o gui gtk_test.c `pkg-config --cflags --libs gtk+-2.0`

draw: draw.c
	gcc -o draw draw.c `pkg-config --cflags --libs gtk+-2.0`

clean:
	rm ./server
	rm ./client
	rm ./draw
