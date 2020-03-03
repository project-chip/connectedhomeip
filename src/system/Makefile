TOP_DIR = ../..

.PHONY: all clean

all: libsystem.a

include $(TOP_DIR)/.yams/cpp_rules.min

Module_Includes = 	\
				-I. \
				-I$(TOP_DIR)/src/include 				\
				-I$(TOP_DIR)/src/lib/ 					\
				-I$(TOP_DIR)/build/config/standalone/

CPP_Files = \
	SystemMutex.cpp \
	SystemError.cpp \

libsystem.a: $(CPP_Objects)
	ar rvs $@ $^
	@echo "LINK =>  $@"

clean: my_clean
	@rm -f libsystem.a *.gcda *.gcno *.gcov
