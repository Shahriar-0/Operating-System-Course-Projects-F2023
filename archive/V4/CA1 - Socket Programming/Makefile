CC=gcc
QST=question.txt
LOG=log.txt

serverI: server client file


server: server.c
	$(CC) server.c -o server.out

client: client.c
	$(CC) client.c -o client.out

file:
	touch $(QST)
	touch $(LOG)
	chmod u+r $(QST)
	chmod u+r $(QST)

clean:
	rm -rf *.o *.out *.txt
	
