NAME=dispatch
REQUIRED_PLUGINS=../core-data-model/zcl-data-model.o ../cluster-server-on-off/on-off-server.o ../binding-mock/mock.o
REQUIRED_GEN=../../utest-gen/cluster-command-handler.o ../../utest-gen/gen-specs.o ../../utest-gen/gen-callback-stubs.o

# ---------- GENERIC -------------
CC=gcc
CFLAGS="-I../../api/" "-I../../utest-gen/" "-I../core-data-model/" -DCHIP_TEST

$(NAME).out: $(NAME).o utest.o general-command-handler.o $(REQUIRED_GEN) $(REQUIRED_PLUGINS)
	$(CC) -o $(NAME).out $(NAME).o utest.o general-command-handler.o $(REQUIRED_GEN) $(REQUIRED_PLUGINS)

test: $(NAME).out
	./$(NAME).out
	
clean:
	rm -f *.o $(objects) *.out
	rm -f ../../api/mock/*.o
	rm -f ../../utest-gen/*.o
