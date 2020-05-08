NAME=identify-server
REQUIRED_PLUGINS=../attribute-db/attribute-db.o ../../utest-gen/gen-callback-stubs.o

# ---------- GENERIC -------------
CC=gcc
CFLAGS="-I../../api/" "-I../../utest-gen/" -DCHIP_TEST

$(NAME).out: $(NAME).o utest.o ../../api/mock/mock.o $(REQUIRED_PLUGINS)
	$(CC) -o $(NAME).out $(NAME).o ../../api/mock/mock.o utest.o  $(REQUIRED_PLUGINS)

test: $(NAME).out
	./$(NAME).out

clean:
	rm -f *.o $(objects) *.out
	rm -f ../../api/mock/*.o
