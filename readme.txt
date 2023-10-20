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