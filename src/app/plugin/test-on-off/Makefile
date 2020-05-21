NAME=ChipZclOnOffTestDriver
GEN=../../gen
REQUIRED_PLUGINS=\
  ../core-data-model/zcl-data-model.o \
  ../core-api/core-api.o \
  ../cluster-server-on-off/on-off-server.o \
  ../binding-mock/mock.o \
  cluster-cmd-on-off-test.o \
  $(GEN)/gen-callback-stubs.o \
  $(GEN)/gen-specs.o \
  $(GEN)/gen-command-handler.o

# ---------- GENERIC -------------
CC=gcc
CFLAGS="-I../../chip-zcl/" "-I$(GEN)/" "-I../cluster-server-on-off/" -DCHIP_TEST

$(NAME).out: $(NAME).o $(REQUIRED_PLUGINS)
	$(CC) -o $(NAME).out $(NAME).o  $(REQUIRED_PLUGINS)

test: $(NAME).out
	./$(NAME).out

clean:
	rm -f *.o $(objects) *.out
