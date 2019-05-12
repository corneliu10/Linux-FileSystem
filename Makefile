build: tema3.c 
	gcc tema3.c -o tema3 -Wall -g
run:	build
	./tema3
clean:
	rm -f tema3
