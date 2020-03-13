TOP_DIR = ../../..
Test_Dir = tests

.PHONY: all clean check

all: libcore.a check

include $(TOP_DIR)/.yams/cpp_rules.min

Module_Includes = \
	-I. \
	-I$(TOP_DIR)/src \
	-I$(TOP_DIR)/src/include \
	-I$(TOP_DIR)/src/lib/ \
	-I$(TOP_DIR)/src/system \
	-I$(TOP_DIR)/build/config/standalone/ \
	-I$(TOP_DIR)/third_party/nlio/repo/include \
	-I$(TOP_DIR)/third_party/nlassert/repo/include

Module_Test_Includes = $(Module_Includes)

CPP_Files = \
	CHIPError.cpp \
	CHIPTLVWriter.cpp \
	CHIPTLVReader.cpp \
	CHIPTLVUtilities.cpp \
	CHIPTLVUpdater.cpp \
	CHIPTLVDebug.cpp

libcore.a: $(CPP_Objects)
	ar rvs $@ $^
	@echo "LINK =>  $@"

clean: my_clean
	@rm -f libcore.a *.gcda *.gcno *.gcov
