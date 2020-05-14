NAME=codec-simple
GEN=../../gen
REQUIRED_PLUGINS=../binding-mock/mock.o

# ---------- GENERIC -------------
CC=gcc
CFLAGS="-I../../api/" "-I$(GEN)/" -DCHIP_TEST -g

$(NAME).out: $(NAME).o utest.o $(REQUIRED_PLUGINS)
	$(CC) -o $(NAME).out $(NAME).o utest.o $(REQUIRED_PLUGINS)

test: $(NAME).out
	./$(NAME).out
	
clean:
	rm -f *.o $(objects) *.out
