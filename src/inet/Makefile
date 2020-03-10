TOP_DIR = ../..
Test_Dir = tests

.PHONY: all clean run_tests

all: libnet.a run_tests

include $(TOP_DIR)/.yams/cpp_rules.min

Module_Includes = 	\
	-I. \
	-I$(TOP_DIR)/src \
	-I$(TOP_DIR)/src/system \
	-I$(TOP_DIR)/src/include \
	-I$(TOP_DIR)/src/lib/ \
	-I$(TOP_DIR)/src/lib/core \
	-I$(TOP_DIR)/build/config/standalone/ \
	-I$(TOP_DIR)/third_party/nlio/repo/include \
	-I$(TOP_DIR)/third_party/nlassert/repo/include

Module_Test_Includes = $(Module_Includes)

CPP_Files = \
	AsyncDNSResolverSockets.cpp \
	IPEndPointBasis.cpp \
	InetLayer.cpp \
	TCPEndPoint.cpp \
	DNSResolver.cpp \
	IPPrefix.cpp \
	InetLayerBasis.cpp \
	EndPointBasis.cpp \
	InetError.cpp \
	InetTimer.cpp \
	UDPEndPoint.cpp \
	IPAddress-StringFuncts.cpp \
	InetFaultInjection.cpp \
	InetUtils.cpp \
	IPAddress.cpp \
	InetInterface.cpp \
	RawEndPoint.cpp

libnet.a: $(CPP_Objects)
	ar rvs $@ $^
	@echo "LINK =>  $@"

clean: my_clean
	@rm -f libnet.a *.gcda *.gcno *.gcov
