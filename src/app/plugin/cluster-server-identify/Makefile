NAME=identify-server
REQUIRED_PLUGINS=../core-data-model/zcl-data-model.o ../../utest-gen/gen-callback-stubs.o ../binding-mock/mock.o

# ---------- GENERIC -------------
CC=gcc
CFLAGS="-I../../api/" "-I../../utest-gen/" "-I../zcl-data-model" -DCHIP_TEST

$(NAME).out: $(NAME).o utest.o $(REQUIRED_PLUGINS)
	$(CC) -o $(NAME).out $(NAME).o utest.o $(REQUIRED_PLUGINS)

test: $(NAME).out
	./$(NAME).out

clean:
	rm -f *.o $(objects) *.out
	rm -f ../../api/mock/*.o
