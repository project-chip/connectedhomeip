TOP_DIR = ../../..
Test_Dir = tests

.PHONY: all clean run_tests

all: libcore.a run_tests

include $(TOP_DIR)/.yams/cpp_rules.min

Module_Includes = 	\
	-I. \
	-I$(TOP_DIR)/src/include \
	-I$(TOP_DIR)/src/lib/ \
	-I$(TOP_DIR)/src/system \
	-I$(TOP_DIR)/build/config/standalone/

Module_Test_Includes = $(Module_Includes)

CPP_Files = CHIPError.cpp

libcore.a: $(CPP_Objects)
	ar rvs $@ $^
	@echo "LINK =>  $@"

clean: my_clean
	@rm -f libcore.a *.gcda *.gcno *.gcov
