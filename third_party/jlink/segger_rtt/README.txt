README.txt for the SEGGER RTT Implementation Pack.

MDK-ARM specifics:
https://wiki.segger.com/Keil_MDK-ARM#RTT_in_uVision

Included files:
===============
Root Directory
  - Examples
    - Main_RTT_InputEchoApp.c    - Sample application which echoes input on Channel 0.
    - Main_RTT_MenuApp.c         - Sample application to demonstrate RTT bi-directional functionality.
    - Main_RTT_PrintfTest.c      - Sample application to test RTT small printf implementation.
    - Main_RTT_SpeedTestApp.c    - Sample application for measuring RTT performance. embOS needed.
  - RTT
    - SEGGER_RTT.c                - The RTT implementation.
    - SEGGER_RTT.h                - Header for RTT implementation.
    - SEGGER_RTT_Conf.h           - Pre-processor configuration for the RTT implementation.
    - SEGGER_RTT_Printf.c         - Simple implementation of printf to write formatted strings via RTT.
  - Syscalls
    - RTT_Syscalls_GCC.c          - Low-level syscalls to retarget printf() to RTT with GCC / Newlib.
    - RTT_Syscalls_IAR.c          - Low-level syscalls to retarget printf() to RTT with IAR compiler.
    - RTT_Syscalls_KEIL.c         - Low-level syscalls to retarget printf() to RTT with KEIL/uVision compiler.
    - RTT_Syscalls_SES.c          - Low-level syscalls to retarget printf() to RTT with SEGGER Embedded Studio.
