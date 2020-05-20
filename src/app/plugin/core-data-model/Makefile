NAME=zcl-data-model
GEN=../../gen
REQUIRED_PLUGINS=$(GEN)/gen-callback-stubs.o ../binding-mock/mock.o

# ---------- GENERIC -------------
CC=gcc
CFLAGS="-I../../api/" "-I$(GEN)/" -DCHIP_TEST

$(NAME).out: $(NAME).o zcl-struct.o utest.o $(REQUIRED_PLUGINS)
	$(CC) -o $(NAME).out $(NAME).o zcl-struct.o utest.o $(REQUIRED_PLUGINS)

test: $(NAME).out
	./$(NAME).out
	
clean:
	rm -f *.o $(objects) *.out
