NAME=utest-on-off
REQUIRED_PLUGINS=../plugin/zcl-data-model/zcl-data-model.o ../utest-gen/gen-callback-stubs.o

# ---------- GENERIC -------------
CC=gcc
CFLAGS="-I../api/" "-I../utest-gen/" "-I../plugin/on-off-server/" -DCHIP_TEST

$(NAME).out: $(NAME).o ../plugin/on-off-server/on-off-server.o ../utest-gen/cluster-command-handler.o ../utest-gen/gen-specs.o ../api/mock/mock.o $(REQUIRED_PLUGINS)
	$(CC) -o $(NAME).out $(NAME).o ../plugin/on-off-server/on-off-server.o ../utest-gen/cluster-command-handler.o ../utest-gen/gen-specs.o ../api/mock/mock.o  $(REQUIRED_PLUGINS)

test: $(NAME).out
	./$(NAME).out

clean:
	rm -f *.o $(objects) *.out
	rm -f ../../api/mock/*.o