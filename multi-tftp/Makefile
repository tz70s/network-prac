mserv:mserv.c
	gcc $^ -lpthread -o $@
	gcc -o client client.c
	mkdir recvs

clean:
	rm -rf mserv client recvs

