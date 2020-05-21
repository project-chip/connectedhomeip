NAME=ChipZclUnitTests
GEN=../../gen
REQUIRED_PLUGINS=\
  ../binding-mock/mock.o \
  ../codec-simple/codec-simple.o \
  ../../gen/gen-callback-stubs.o \
  ../../gen/gen-command-handler.o \
  ../cluster-server-basic/basic-server.o \
  ../cluster-server-level-control/level-control-server.o \
  ../cluster-server-identify/identify-server.o \
  ../cluster-server-on-off/on-off-server.o \
  ../core-message-dispatch/dispatch.o \
  ../core-message-dispatch/general-command-handler.o \
  ../core-data-model/zcl-data-model.o


TEST_FILES=\
  cluster-server-basic-test.o \
  cluster-server-identify-test.o \
  cluster-server-level-control-test.o \
  cluster-server-on-off-test.o \
  codec-simple-test.o \
  core-data-model-test.o \
  core-message-dispatch-test.o

# ---------- GENERIC -------------
CC=gcc
CFLAGS="-I../../chip-zcl/" "-I$(GEN)/" -DCHIP_TEST -g

$(NAME).out: $(NAME).o $(REQUIRED_PLUGINS) $(TEST_FILES)
	$(CC) -o $(NAME).out $(NAME).o $(REQUIRED_PLUGINS) $(TEST_FILES)

test: $(NAME).out
	./$(NAME).out

clean:
	rm -f *.o $(objects) *.out
