default: all

all: clean server client test

server:
	gcc server.c crc.c hamming.c tag.c encDec.c client_list.c -o server
client:
	gcc client.c crc.c hamming.c tag.c encDec.c -o client
test:
	gcc test.c crc.c hamming.c encDec.c -o test

clean:
	rm -f server client test
