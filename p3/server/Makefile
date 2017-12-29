all:
	gcc shm_client.c -o shm_client -Wall -Werror -lrt -lpthread -ggdb
	gcc shm_server.c -o shm_server -Wall -Werror -lrt -lpthread -ggdb
clean:
	rm -f shm_client
	rm -f shm_server
