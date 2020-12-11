mole: lib/read_params/read_params.o src/main.o 
	gcc -Wall -o mole src/main.o lib/read_params/read_params.o
	make clean

lib/read_params/read_params.o: lib/read_params/read_params.c
	gcc -Wall -o lib/read_params/read_params.o -c lib/read_params/read_params.c

src/main.o: src/main.c
	gcc -Wall -o src/main.o -c src/main.c

.PHONY: clear clear_all

clean:
	rm src/*.o
	rm lib/read_params/*.o

clean_all: 
	rm mole 
	rm src/*.o 
	rm lib/read_params/*.o