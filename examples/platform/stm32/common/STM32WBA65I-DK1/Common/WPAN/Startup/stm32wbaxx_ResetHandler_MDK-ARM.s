;********************************************************************************
;* File Name          : stm32wbaxx_ResetHandler.s
;* Author             : MCD Application Team
;* Description        : STM32WBA5xx Ultra Low Power Devices specific
;                       Reset handler for connectivity applications.
;                       MDK-ARM toolchain.
;********************************************************************************
;* @attention
;*
;* Copyright (c) 2022 STMicroelectronics.
;* All rights reserved.
;*
;* This software is licensed under terms that can be found in the LICENSE file
;* in the root directory of this software component.
;* If no LICENSE file comes with this software, it is provided AS-IS.
;*
;*******************************************************************************
;
; Cortex-M version
;
                 PRESERVE8
                 THUMB
                 AREA    |.text|, CODE, READONLY

; Reset_Handler
Reset_Handler    PROC
                 EXPORT  Reset_Handler

                 IMPORT  SystemInit
                 IMPORT  __main
                 IMPORT  is_boot_from_standby
                 IMPORT  backup_MSP
                 IMPORT  register_backup_table
                 IMPORT  register_backup_table_size

     EXPORT  CPUcontextSave
                 EXPORT  backup_system_register
                 EXPORT  restore_system_register
; If we exit from standby mode, restore CPU context and jump to asleep point.
        BL      is_boot_from_standby
        CMP     R0, #1
        BEQ     CPUcontextRestore
; buffer for local variables (up to 10)from is_boot_from_standby.
        SUB     SP, SP, #0x28
; end of specific code section for standby
        LDR     R0, =SystemInit
        BLX     R0
        LDR     R0, =__main
        BX      R0
        ENDP

; These 2 functions are designed to save and then restore CPU context.
CPUcontextSave
        PUSH   { R4 - R12, lr }        ; store R4 to R12 and LR (10 words) onto C stack
        LDR    R4, =backup_MSP         ; load address of backup_MSP into R4
        MOV    R3, SP                  ; load the stack pointer into R3
        STR    R3, [R4]                ; store the MSP into backup_MSP
        DSB
        WFI                            ; all saved, trigger deep sleep

CPUcontextRestore
  ; Even if we fall through the WFI instruction, we will immediately
  ; execute a context restore and end up where we left off with no
  ; ill effects.  Normally at this point the core will either be
  ; powered off or reset (depending on the deep sleep level).
        LDR    R4, =backup_MSP       ; load address of backup_MSP into R4.
        LDR    R4, [R4]              ; load the SP from backup_MSP.
        MOV    SP, R4                ; restore the SP from R4.
        POP   { R4 - R12, PC }       ; load R4-R12 and PC (10 words) from C stack
; end of specific code section for standby.

backup_system_register
; R0 -> register_backup_table array current item address
; R1 -> loop counter (from register_backup_table_size to 0)
; R2 -> register_backup_table array current item value

backup_loop_init
        LDR    R0, =register_backup_table        ; R0 points to the first array item
        LDR    R1, =register_backup_table_size
        LDR    R1, [R1]                          ; R1 contains the number of registers in the array

backup_loop_iter
        ; Offset processing
        LDR    R2, [R0], #4                      ; R2 contains the register_backup_table current item (register address)
                                                 ; R0 points to the next register_backup_table item (uint32_t items -> 4 bytes added to previous address)

        ; Register value backup
        LDR    R2, [R2]                          ; R2 contains now the register value to backup
        PUSH   {R2}                              ; Push register value into the stack

        ; Loop iteration control
        SUBS   R1, #1                            ; Decrement loop counter by 1 and update APSR (Application Processor Status Register)
        BNE    backup_loop_iter                  ; Loop continues until Z flag is set (still array item to handle)

backup_loop_end
        BX LR                                    ; Return to caller

restore_system_register
; R0 -> register_backup_table array current item address
; R1 -> loop counter (from register_backup_table_size to 0)
; R2 -> register_backup_table array current item value

restore_loop_init
        LDR    R0, =register_backup_table         ; R0 points to the first array item

        ; Reverse loop: counter initial value processing
        LDR    R1, =register_backup_table_size
        LDR    R1, [R1]                          ; R1 contains the number of registers in the array
        SUB    R1, #1                            ; R1 now contains last array item index (register_backup_table_size - 1)

        ; Reverse loop: apply offset to current array index (point to last array element)
        ADD    R0, R1, LSL #2                    ; R0 now points to last array item (register_backup_table + (register_backup_table_size - 1) * 4)

        ADD    R1, #1                            ; Re-add 1 to R1 (array length)

; Reverse loop
restore_loop_iter
        ; Offset processing
        LDR    R2, [R0], #-4                     ; R2 contains the register_backup_table current item (register address)
                                                 ; R0 now points to the previous register_backup_table item (uint32_t items -> 4 bytes subtracted to previous address)

        ; Register value restoration
        POP    {R3}                              ; Head of stack popped into R3. R3 contains register value to restore
        STR    R3, [R2]                          ; Write backuped value into the register

        ; Loop iteration control
        SUBS   R1, #1                            ; Decrement loop counter by 1 and update APSR (Application Processor Status Register)
        BNE    restore_loop_iter                 ; Loop continues until Z flag is set (still array item to handle)

restore_loop_end
        BX LR                                    ; Return to caller

        END

