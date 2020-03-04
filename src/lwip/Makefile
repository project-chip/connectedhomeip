TOP_DIR = ../..
Test_Dir = tests

.PHONY: all

all: liblwip.a run_tests

include $(TOP_DIR)/.yams/cpp_rules.min

Module_Includes = \
	-I. \
	-I$(TOP_DIR) \
	-I$(TOP_DIR)/third_party/lwip/repo/lwip/src/include \
	-I$(TOP_DIR)/third_party/lwip/repo/lwip/src/include/ipv4 \
	-I$(TOP_DIR)/third_party/lwip/repo/lwip/src/include/ipv6 \
	-I$(TOP_DIR)/src/lwip \
	-I$(TOP_DIR)/src/lwip/standalone

Module_Test_Includes = $(Module_Includes)

C_Files = \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/api/api_lib.c                  \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/api/api_msg.c                  \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/api/err.c                      \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/api/netbuf.c                   \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/api/netdb.c                    \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/api/netifapi.c                 \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/api/sockets.c                  \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/api/tcpip.c                    \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/def.c                     \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/dns.c                     \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/inet_chksum.c             \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/init.c                    \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/ipv4/autoip.c             \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/ipv4/icmp.c               \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/ipv4/igmp.c               \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/ipv4/ip4.c                \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/ipv4/ip4_addr.c           \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/ipv4/etharp.c             \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/ipv4/ip4_frag.c           \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/ipv4/dhcp.c               \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/ipv6/ip6_route_table.c    \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/ipv6/dhcp6.c              \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/ipv6/ethip6.c             \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/ipv6/icmp6.c              \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/ipv6/inet6.c              \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/ipv6/ip6.c                \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/ipv6/ip6_addr.c           \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/ipv6/ip6_frag.c           \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/ipv6/mld6.c               \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/ipv6/nd6.c                \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/mem.c                     \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/memp.c                    \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/netif.c                   \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/pbuf.c                    \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/raw.c                     \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/stats.c                   \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/sys.c                     \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/tcp.c                     \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/tcp_in.c                  \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/tcp_out.c                 \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/udp.c                     \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/ip.c                      \
    $(TOP_DIR)/third_party/lwip/repo/lwip/src/core/timeouts.c                \
    standalone/sys_arch.c                                                    \
    standalone/TapInterface.c

liblwip.a: $(C_Objects)
	@ar cru $@ $^
	@echo "LINK =>  $@"

.PHONY: clean

clean: my_clean
	@rm -f liblwip.a *.gcda *.gcno *.gcov
