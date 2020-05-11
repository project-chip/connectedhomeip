NAME=dispatch
REQUIRED_PLUGINS=../zcl-data-model/zcl-data-model.o ../on-off-server/on-off-server.o
REQUIRED_GEN=../../utest-gen/cluster-command-handler.o ../../utest-gen/gen-specs.o ../../utest-gen/gen-callback-stubs.o

# ---------- GENERIC -------------
CC=gcc
CFLAGS="-I../../api/" "-I../../utest-gen/" "-I../zcl-data-model/" -DCHIP_TEST

$(NAME).out: $(NAME).o utest.o general-command-handler.o ../../api/mock/mock.o $(REQUIRED_GEN) $(REQUIRED_PLUGINS)
	$(CC) -o $(NAME).out $(NAME).o ../../api/mock/mock.o utest.o general-command-handler.o $(REQUIRED_GEN) $(REQUIRED_PLUGINS)

test: $(NAME).out
	./$(NAME).out
	
clean:
	rm -f *.o $(objects) *.out
	rm -f ../../api/mock/*.o
	rm -f ../../utest-gen/*.o
