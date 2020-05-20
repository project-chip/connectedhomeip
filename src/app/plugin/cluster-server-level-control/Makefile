NAME=level-control-server
GEN=../../gen
REQUIRED_PLUGINS=../core-data-model/zcl-data-model.o $(GEN)/gen-callback-stubs.o ../binding-mock/mock.o

# ---------- GENERIC -------------
CC=gcc
CFLAGS="-I../../api/" "-I$(GEN)/" -DCHIP_TEST

$(NAME).out: $(NAME).o utest.o $(REQUIRED_PLUGINS)
	$(CC) -o $(NAME).out $(NAME).o utest.o $(REQUIRED_PLUGINS)

test: $(NAME).out
	./$(NAME).out
	
clean:
	rm -f *.o $(objects) *.out
