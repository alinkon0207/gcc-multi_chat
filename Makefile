default: build

all: server client

server:
	gcc server.c tag.c client_list.c -o server
client:
	gcc client.c tag.c -o client

clean:
	rm -f server client
