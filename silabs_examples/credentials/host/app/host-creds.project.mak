####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 10                                              #
####################################################################

BASE_SDK_PATH = /Volumes/Green/git/gsdk
UNAME:=$(shell uname -s | sed -e 's/^\(CYGWIN\).*/\1/' | sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= gecko_sdk_4.1.0

# This uses the explicit build rules below
PROJECT_SOURCE_FILES =

C_SOURCE_FILES   += $(filter %.c, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cpp, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cc, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.s, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.S, $(PROJECT_SOURCE_FILES))
LIB_FILES        += $(filter %.a, $(PROJECT_SOURCE_FILES))

C_DEFS += \

ASM_DEFS += \

INCLUDES += \
 -I..

GROUP_START =
GROUP_END =

PROJECT_LIBS = \
 -lc \
 -lm

LIBS += $(GROUP_START) $(PROJECT_LIBS) $(GROUP_END)

LIB_FILES += $(filter %.a, $(PROJECT_LIBS))

C_FLAGS += \
 -std=c99 \
 -Wall \
 -Wextra \
 -Os

CXX_FLAGS += \
 -std=c++11 \
 -Wall \
 -Wextra \
 -Os

ASM_FLAGS += \

LD_FLAGS += \


####################################################################
# SDK Build Rules                                                  #
####################################################################
$(OUTPUT_DIR)/project/_/host_creds.o: ../host_creds.c
	@echo 'Building ../host_creds.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../host_creds.c
CDEPS += $(OUTPUT_DIR)/project/_/host_creds.d
OBJS += $(OUTPUT_DIR)/project/_/host_creds.o

$(OUTPUT_DIR)/project/_/main.o: ../main.c
	@echo 'Building ../main.c'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../main.c
CDEPS += $(OUTPUT_DIR)/project/_/main.d
OBJS += $(OUTPUT_DIR)/project/_/main.o

# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=eJztWW1PGzkQ/ivRfrwmu6RHUVkB1QnoiRNREIFWpy5aGdskvuzaK9tLQhH//cbelySbFyCwhEqtqiUej2cejz3jsefeOTvv/nN8eBH2upfnh8c9x3f2vozjqHFLpWKC7wdO290KnAblWBDG+0C4vPja+hw4Xw4CvpdI8R/FugFDuPJjQWgEHAOtE9/zRqORq1iErpWLRewp5fV0SphwKRaSglAYnVCp73oY/sK4XFrggOhGY+9GRITKBkex6USpFn3K807TzSJadLpY4hALfsP65ifITiUDuuHxvW8iSmOqvL8lpdxTGqmBN4p5qIWIlBcjran0MqQhHaM4iYAZS0oo1wwBy0Ao7aEk8XIQ3pw+L4PsZZhz/FMAY8S4Ww+uQrQ3r9X0h2aAqkn3rILVCAZ1IxgsRHB+/NdR59iNSS3qp6TnurNdkUqkwX8KEEf0BqUR7OwGyLM+MqEkSOpDEScw4JpFTN/lZnMjxtNx9g13toHTQMMDWO4qO/QpMqxSwekK/4NeF6aI4ZuCy/nbLri1G7Y/7+x++rjzaXt74lbWiX1CFZYsMXM42PPmadlun5mriQde7sLw22k6vZPO2enJ4cnFv2Hv4vLopBt2ukeXpzbM/LgPnOuURZrx4zGOUhAeOP6Pq2bgSBqLWwoW9W/AzrRZMvZEKnHGV8aDeVc0IgBwGkGI8QOniFF+p2OJeazKqftBsCJenWVMecACzkaOxA7TMs2IjNh2it1Mr6uoThM3xcV4s5ytbJP2MbZjEhLPCDkIrEkZB1uYyKtgX5gtmKly/zBfzzCVNi4mdBA4ExPBlI3Eh+ZLzTu1sZvVQNKs+nVzEuDejfVBSCI4OGvPNn+5JXg3hkzxYeZdv024vgkvKBwgSNPfRlzbiCQ7McGdYURfbcCSVwvPtLOjTpk3B0E1cwbKwtw5mGTPwJKnCU91PSN1afLsWeGNevAs28ebQrQ6yq9EZTUAA/y3CeNESXGWAaXWdH2Z/tnTthYUc4n7E7AM6scyWIllKiepBchsMj9BYeMHoWdID+rQW9EWsWuJ5N3XmalXF2khU1xhMii6Nl+HlmmcQGR1JlTwZ3O9GFJpDSoscSmPC9k2hVSfdlBi9ZoxtyhKjWobcZ+v3bgviHpEf8Hl4hBMzAmSZEY53t19A93j8RLtHz602/XrHyHJ7bmHomgDpi/V07GWaJMAEkoQeA+ewWDzl7dcBElDKqWQahMwDEfMftpr94x+xX4+TX2MhtSEMHtimrNyaqNXQaziXeaSLaXJ/lP9cpWCJHkOnCRZ7qcZpCc766tZqNgzYdVxW98t5a0NVDOe9WwzH1Va33PaxuxTH6Zn2Wips7e66u2t83pokIxnxM8BYQrPZOlTYk+Z0qXoEkG/AsCbIFgDF1KKxtevAmwMjVJca8T0oGUtvz7aqbddRuAyxP78mBmTaDdGHPUpMZdX4poRbp+nbpnGXSNFq1N6gTjMJE4jCLY0oRzyWny3XnLwfmbEYeMTSLDXPdrXUA2OtTgRf4msjS5NGTQeWZwKd8bkGhdzhR5QGTGln+dslavSo/w3cIFTMIdWRHlfD/a33tgvk+R5xprm/wXMNVUTqusR6JGXwc29li2p/zz9WcpWkycapmrg1QWbfSmZK4pBV/nWmjs+kIobPVkcDcpq9twyLn4IPe+eHZ9fnNj63v1UidMPoBnA7QTdUtLTAg+/IckQHIbKkH3zMQzmn2PKod2EF02/+FHWQfN2s/gBBmQglgxPBc7SkurQPsVDESoyDG3dsyA/ZJ+inJUt0LtH+wC2z4SYRxiVl1LLlczfqpvljrKvVmZSSwumwCsk6zOOopyZp1EE1NwZgNBuGuRcU26Wc3f3U3u7vbXTfrhyHv4H2A83pw===END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA