mole: lib/read_params/read_params.o lib/get_index/get_index.o src/main.o  lib/index_pthread/index_pthread.o lib/data_saving/data_saving.o
	gcc -Wall -std=gnu99 -o mole src/main.o lib/read_params/read_params.o lib/get_index/get_index.o lib/index_pthread/index_pthread.o lib/data_saving/data_saving.o -lpthread -lm 
	make clean

lib/data_saving/data_saving.o: lib/data_saving/data_saving.c
	gcc -Wall -o lib/data_saving/data_saving.o -c lib/data_saving/data_saving.c

lib/index_pthread/index_pthread.o: lib/index_pthread/index_pthread.c
	gcc -Wall -o lib/index_pthread/index_pthread.o -c lib/index_pthread/index_pthread.c

lib/read_params/read_params.o: lib/read_params/read_params.c
	gcc -Wall -o lib/read_params/read_params.o -c lib/read_params/read_params.c

lib/get_index/get_index.o:
	gcc -Wall -o lib/get_index/get_index.o -c lib/get_index/get_index.c


src/main.o: src/main.c
	gcc -Wall -o src/main.o -c src/main.c

.PHONY: clear clear_all

clean:
	rm src/*.o
	rm lib/read_params/*.o
	rm lib/get_index/*.o

clean_all: 
	rm mole 
	rm src/*.o 
	rm lib/read_params/*.o