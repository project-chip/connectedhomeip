NAME=utest-on-off

# ---------- GENERIC -------------
CC=gcc
CFLAGS="-I../api/" "-I../utest-gen/" "-I../plugin/on-off-server/" -DCHIP_TEST

$(NAME).out: $(NAME).o ../plugin/on-off-server/on-off-server.o ../utest-gen/call-command-handler.o ../utest-gen/gen-specs.o ../api/mock/mock.o
	$(CC) -o $(NAME).out $(NAME).o ../plugin/on-off-server/on-off-server.o ../utest-gen/call-command-handler.o ../utest-gen/gen-specs.o ../api/mock/mock.o

test: $(NAME).out
	./$(NAME).out

clean:
	rm -f *.o $(objects) *.out
	rm -f ../../api/mock/*.o