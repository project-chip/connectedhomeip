TOP_DIR = ../..
Test_Dir = tests

.PHONY: all clean run_tests

all: libSystemLayer.a run_tests

include $(TOP_DIR)/.yams/cpp_rules.min

Module_Includes = \
				-I. \
				-I$(TOP_DIR)/src \
				-I$(TOP_DIR)/src/include \
				-I$(TOP_DIR)/src/lib \
				-I$(TOP_DIR)/build/config/standalone \
				-I$(TOP_DIR)/third_party/nlassert/repo/include \
				-I$(TOP_DIR)/src/lib/core \


Module_Test_Includes = $(Module_Includes)

CPP_Files = \
	SystemMutex.cpp \
	SystemError.cpp \
	SystemClock.cpp \
	SystemObject.cpp \
	SystemStats.cpp \
	SystemTimer.cpp \
	SystemLayer.cpp \
	SystemFaultInjection.cpp \
	SystemPacketBuffer.cpp

libSystemLayer.a: $(CPP_Objects)
	ar rvs $@ $^
	@echo "LINK =>  $@"

tests_FLAGS = -lpthread

clean: my_clean
	@rm -f libSystemLayer.a *.gcda *.gcno *.gcov
