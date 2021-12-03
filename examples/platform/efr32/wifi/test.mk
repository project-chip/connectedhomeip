SILABS_BASE=$(HOME)/silabs
MATTER_BASE=$(SILABS_BASE)/chip/connectedhomeip
EFR32_SDK=$(MATTER_BASE)/third_party/efr32_sdk/repo
RSI_BASE=$(SILABS_BASE)/wiseconnect-wifi-bt-sdk
TMP_SRC=$(SILABS_BASE)/chip/silabs/rs911x/examples/platform/efr32/rs911x-wfx/
WHICH_APP=lock-app
ifeq ($(EFR32_BOARD), BRD4161A)
WHICH_EFR=EFR32MG12
efr_subdir=efr32mg12
else
# Board is 4180A
WHICH_EFR=EFR32MG21
efr_subdir=efr32mg21
EFR_PART=EFR32MG21A010F1024IM32
EFR_BOARD=EFR32MG21_BRD4180A
endif
INC= -I$(EFR32_SDK)/platform/emlib/inc \
	-I$(EFR32_SDK)/util/third_party/freertos/kernel/include \
	-I$(EFR32_SDK)/platform/Device/SiliconLabs/$(WHICH_EFR)/Include \
	-I$(EFR32_SDK)/util/third_party/freertos/kernel/portable/GCC/ARM_CM3/ \
	-I$(EFR32_SDK)/platform/CMSIS/Include \
	-I$(EFR32_SDK)/platform/common/inc \
	-I$(MATTER_BASE)/examples/$(WHICH_APP)/efr32/include \
	-I$(MATTER_BASE)/examples/efr32/$(efr_subdir)/$(EFR32_BOARD) \
	-I$(MATTER_BASE)/examples/efr32 \
	-I$(RSI_BASE)/sapi/include \
	-I$(TMP_SRC)/ \
	-I$(TMP_SRC)/hal
VPATH= \
	$(TMP_SRC) $(TMP_SRC)/hal
DEFS=-D$(EFR_PART) -D$(EFR_BOARD)
CROSS=arm-none-eabi-
CC=$(CROSS)gcc
O= -O2
MFLAGS= -mcpu=cortex-m3
CFLAGS= $(INC) $O $(DEFS) $(MFLAGS)
OBJS= \
	wfx_host.o\
	rsi_if.o \
	rsi_timer.o \
	rsi_hal_mcu_spi.o \
	rsi_hal_mcu_interrupt.o \
	rsi_hal_mcu_ioports.o \
	wfx_notify.o

all: $(OBJS)
