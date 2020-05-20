NAME=dispatch
GEN=../../gen
REQUIRED_PLUGINS=../core-data-model/zcl-data-model.o ../cluster-server-on-off/on-off-server.o ../binding-mock/mock.o $(GEN)/gen-command-handler.o $(GEN)/gen-specs.o $(GEN)/gen-callback-stubs.o

# ---------- GENERIC -------------
CC=gcc
CFLAGS="-I../../api/" "-I$(GEN)" "-I../core-data-model/" -DCHIP_TEST

$(NAME).out: $(NAME).o utest.o general-command-handler.o $(REQUIRED_PLUGINS)
	$(CC) -o $(NAME).out $(NAME).o utest.o general-command-handler.o $(REQUIRED_PLUGINS)

test: $(NAME).out
	./$(NAME).out
	
clean:
	rm -f *.o $(objects) *.out
