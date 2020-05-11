NAME=zcl-data-model

# ---------- GENERIC -------------
CC=gcc
CFLAGS="-I../../api/" "-I../../utest-gen/" -DCHIP_TEST

$(NAME).out: $(NAME).o utest.o ../../api/mock/mock.o ../../utest-gen/gen-callback-stubs.o 
	$(CC) -o $(NAME).out $(NAME).o ../../api/mock/mock.o ../../utest-gen/gen-callback-stubs.o utest.o

test: $(NAME).out
	./$(NAME).out
	
clean:
	rm -f *.o $(objects) *.out
	rm -f ../../api/mock/*.o
