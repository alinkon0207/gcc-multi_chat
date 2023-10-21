(Compile source)
> gcc test.c -o test
(Test)
> ./test


> gcc server.c -o server
> gcc client.c -o client

./server

./client * n
Client Menu:
1. REQUEST
2. LOGIN
3. MSG
1
2
3
Input Msg: How are u!
<MSG><TO>C1</TO><BODY>How are u!(Hamming code)</BODY></MSG>(CRC32)

To compile and run the program:

Save the server code in a file called server.c and the client code in a file called client.c.
Open a terminal and compile the server code using the command: gcc -o server server.c -lpthread
Run the server: ./server
Open another terminal and compile the client code using the command: gcc -o client client.c
Run the client: ./client
Repeat step 5 to run additional clients and start chatting.