TOP_DIR = ../../..
Test_Dir = tests
Test_Bin = tests_q

.PHONY: all clean run_tests

all: run_tests

include $(TOP_DIR)/.yams/cpp_rules.min

Module_Includes = 	\
				-I. \
				-I$(TOP_DIR)/src/include 				\
				-I$(TOP_DIR)/src/lib/ 					\
				-I$(TOP_DIR)/build/config/standalone/

Module_Test_Includes = $(Module_Includes)

CPP_Files = \
	QRCodeSetupPayload.cpp \
	SetupCodeUtils.cpp \
	QRCodeSetupPayloadGenerator.cpp \
	$(Test_Dir)/tests.cpp
	
HEADER_Files = \
	QRCodeSetupPayload.h \
	SetupCodeUtils.h \
	QRCodeSetupPayloadGenerator.h


run_tests: $(Test_Bin)
	./$(Test_Dir)/$(Test_Bin)


$(Test_Bin): $(CPP_Files) $(HEADER_Files)
	-mkdir -p $(Test_Dir)
	$(CXX) $(CPP_Files) -o $(Test_Dir)/$@ 

clean: my_clean
	@rm -f $(LIB_NAME) *.gcda *.gcno *.gcov
	
