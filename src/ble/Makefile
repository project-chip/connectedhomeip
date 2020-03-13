TOP_DIR = ../..
Test_Dir = tests

.PHONY: all clean check

all: libble.a check

include $(TOP_DIR)/.yams/cpp_rules.min

Module_Includes = 	\
	-I. \
	-I$(TOP_DIR)/src \
	-I$(TOP_DIR)/src/include \
	-I$(TOP_DIR)/src/lib/ \
	-I$(TOP_DIR)/src/lib/core/ \
	-I$(TOP_DIR)/src/system/ \
	-I$(TOP_DIR)/build/config/standalone/ \
	-I$(TOP_DIR)/third_party/nlassert/repo/include \
	-I$(TOP_DIR)/third_party/nlio/repo/include

Module_Test_Includes = $(Module_Includes)

CPP_Files = \
	BLEEndPoint.cpp \
	BleError.cpp \
	BleLayer.cpp \
	BleUUID.cpp \
	CHIPoBle.cpp

libble.a: $(CPP_Objects)
	ar rvs $@ $^
	@echo "LINK =>  $@"

clean: my_clean
	@rm -f libble.a *.gcda *.gcno *.gcov
