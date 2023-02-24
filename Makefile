PORT = 1234
all:
	gcc server.c threadpool.c -o server
	gcc client.c -o client

run_server:
	./server $(PORT)
run_client:
	./client $(PORT)

clean:
	rm -f server client
