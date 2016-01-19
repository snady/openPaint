all: server client gui

server: server.c
	gcc -o server server.c

client: client.c
	gcc -o client client.c

gui: gtk_test.c
	gcc $(`pkg-config --cflags --libs gtk+-2.0`) -o gui gtk_test.c 

clean:
	rm ./server
	rm ./client
	rm ./gui
