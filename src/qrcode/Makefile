TOP_DIR = ../..
Test_Dir = tests

.PHONY: all clean run_tests

all: run_tests

include $(TOP_DIR)/.yams/cpp_rules.min

Module_Includes = \
	-I. \
	-I$(TOP_DIR)/src/include \
	-I$(TOP_DIR)/src/lib/ \
	-I$(TOP_DIR)/build/config/standalone/

Module_Test_Includes = $(Module_Includes)

CPP_Files = \
	SetupCodeUtils.cpp \
	SetupPayloadGenerator.cpp \

libqr.a: $(CPP_Objects)
	ar rvs $@ $^
	@echo "LINK =>  $@"

clean: my_clean
	@rm -f $(LIB_NAME) *.gcda *.gcno *.gcov

