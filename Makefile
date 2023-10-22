default: all

all: clean server client

server:
	gcc server.c crc.c hamming.c tag.c encDec.c client_list.c -o server
client:
	gcc client.c crc.c hamming.c tag.c encDec.c -o client

clean:
	rm -f server client
