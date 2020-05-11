NAME=utest-on-off
REQUIRED_PLUGINS=../plugin/core-data-model/zcl-data-model.o ../plugin/cluster-server-on-off/on-off-server.o ../utest-gen/gen-callback-stubs.o ../plugin/binding-mock/mock.o

# ---------- GENERIC -------------
CC=gcc
CFLAGS="-I../api/" "-I../utest-gen/" "-I../plugin/cluster-server-on-off/" -DCHIP_TEST

$(NAME).out: $(NAME).o ../utest-gen/cluster-command-handler.o ../utest-gen/gen-specs.o $(REQUIRED_PLUGINS)
	$(CC) -o $(NAME).out $(NAME).o ../utest-gen/cluster-command-handler.o ../utest-gen/gen-specs.o  $(REQUIRED_PLUGINS)

test: $(NAME).out
	./$(NAME).out

clean:
	rm -f *.o $(objects) *.out
	rm -f ../../api/mock/*.o