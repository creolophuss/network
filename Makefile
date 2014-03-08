main_server : myhdb.o cmd_handler.o main_server.o lg_client server 
	cc -o main_server main_server.o myhdb.o cmd_handler.o -ltokyocabinet
lg_client : login_client.c
	cc -o lg_client login_client.c
server : myhdb.o cmd_handler.o service.o
	cc -o  server service.o myhdb.o cmd_handler.o -ltokyocabinet
service.o : service.c
	cc -c -o service.o service.c
main_server.o : main_server.c 
	cc  -c -o main_server.o main_server.c 
cmd_handler.o : cmd_handler.c 
	cc  -c -o cmd_handler.o cmd_handler.c 
myhdb.o : myhdb.c 
	cc -c -o myhdb.o myhdb.c 
clean :
	rm  cmd_handler.o myhdb.o  server lg_client service.o main_server main_server.o
