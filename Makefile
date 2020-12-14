mole: lib/read_params/read_params.o lib/get_index/get_index.o src/main.o  lib/index_pthread/index_pthread.o lib/data_saving/data_saving.o lib/read_commands/read_commands.o lib/bulk_operations/bulk_operations.o
	gcc -Wall -std=gnu99 -o mole src/main.o lib/read_params/read_params.o lib/get_index/get_index.o lib/index_pthread/index_pthread.o lib/data_saving/data_saving.o lib/read_commands/read_commands.o lib/bulk_operations/bulk_operations.o -lpthread -lm 
	make clean

lib/read_commands/read_commands.o: lib/read_commands/read_commands.c
	gcc -Wall -o lib/read_commands/read_commands.o -c lib/read_commands/read_commands.c

lib/data_saving/data_saving.o: lib/data_saving/data_saving.c
	gcc -Wall -o lib/data_saving/data_saving.o -c lib/data_saving/data_saving.c

lib/index_pthread/index_pthread.o: lib/index_pthread/index_pthread.c
	gcc -Wall -o lib/index_pthread/index_pthread.o -c lib/index_pthread/index_pthread.c

lib/read_params/read_params.o: lib/read_params/read_params.c
	gcc -Wall -o lib/read_params/read_params.o -c lib/read_params/read_params.c

lib/get_index/get_index.o: 
	gcc -Wall -o lib/get_index/get_index.o -c lib/get_index/get_index.c

lib/bulk_operations/bulk_operations.o: lib/bulk_operations/bulk_operations.c
	gcc -Wall -o lib/bulk_operations/bulk_operations.o -c lib/bulk_operations/bulk_operations.c

src/main.o: src/main.c
	gcc -Wall -o src/main.o -c src/main.c

.PHONY: clear clear_all

clean:
	rm src/*.o
	rm lib/read_params/*.o
	rm lib/get_index/*.o
	rm lib/index_pthread/*.o
	rm lib/data_saving/*.o
	rm lib/read_commands/*.o

clean_all: 
	rm mole 
	rm src/*.o 
	rm lib/read_params/*.o
	rm lib/get_index/*.o
	rm lib/index_pthread/*.o
	rm lib/data_saving/*.o
	rm lib/read_commands/*.o