NAME=utest-on-off
GEN=../../gen
REQUIRED_PLUGINS=../core-data-model/zcl-data-model.o ../cluster-server-on-off/on-off-server.o $(GEN)/gen-callback-stubs.o ../binding-mock/mock.o

# ---------- GENERIC -------------
CC=gcc
CFLAGS="-I../../api/" "-I$(GEN)/" "-I../cluster-server-on-off/" -DCHIP_TEST

$(NAME).out: $(NAME).o $(GEN)/cluster-command-handler.o $(GEN)/gen-specs.o $(REQUIRED_PLUGINS)
	$(CC) -o $(NAME).out $(NAME).o $(GEN)/cluster-command-handler.o $(GEN)/gen-specs.o  $(REQUIRED_PLUGINS)

test: $(NAME).out
	./$(NAME).out

clean:
	rm -f *.o $(objects) *.out
