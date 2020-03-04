TOP_DIR = ../..
Test_Dir = tests

.PHONY: all clean run_tests

all: libsystem.a run_tests

include $(TOP_DIR)/.yams/cpp_rules.min

Module_Includes = 	\
				-I. \
				-I$(TOP_DIR)/src/include 				\
				-I$(TOP_DIR)/src/lib/ 					\
				-I$(TOP_DIR)/build/config/standalone/

Module_Test_Includes = $(Module_Includes)

CPP_Files = \
	SystemMutex.cpp \
	SystemError.cpp \

libsystem.a: $(CPP_Objects)
	ar rvs $@ $^
	@echo "LINK =>  $@"

tests_FLAGS = -lpthread

clean: my_clean
	@rm -f libsystem.a *.gcda *.gcno *.gcov
