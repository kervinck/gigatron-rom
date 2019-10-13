; ROM
giga_text32         EQU     0x0700
giga_text82         EQU     0x0800
giga_notesTable     EQU     0x0900

; RAM
giga_vram           EQU     0x0800
giga_videoTable     EQU     0x0100
giga_soundChan1     EQU     0x01FA
giga_soundChan2     EQU     0x02FA
giga_soundChan3     EQU     0x03FA
giga_soundChan4     EQU     0x04FA

; defines
giga_xres           EQU     160
giga_yres           EQU     120

; page 0
giga_rand0          EQU     0x06
giga_rand1          EQU     0x07
giga_rand2          EQU     0x08
giga_videoY         EQU     0x09
giga_buttonState    EQU     0x11
giga_frameCount     EQU     0x0E
giga_sysFn          EQU     0x22
giga_sysArg0        EQU     0x24
giga_sysArg1        EQU     0x25
giga_sysArg2        EQU     0x26
giga_sysArg3        EQU     0x27
giga_sysArg4        EQU     0x28
giga_sysArg5        EQU     0x29
giga_sysArg6        EQU     0x2A
giga_sysArg7        EQU     0x2B
giga_soundTimer     EQU     0x2C

vPC             	EQU     0x16
vAC             	EQU     0x18
vLR             	EQU     0x1A
vSP             	EQU     0x1C

; SYS calls
SYS_Reset_36        EQU     0x009a
SYS_Exec_88         EQU     0x00ad
SYS_Out_22          EQU     0x00f4
SYS_In_24           EQU     0x00f9
SYS_NextByteIn      EQU     0x02e9
SYS_Random_34       EQU     0x04a7
SYS_LSRW7_30        EQU     0x04b9
SYS_LSRW8_24        EQU     0x04c6
SYS_LSLW8_24        EQU     0x04cd
SYS_Draw4_30        EQU     0x04d4
SYS_VDrawBits_134   EQU     0x04e1
SYS_LSRW1_48        EQU     0x0600
SYS_LSRW2_52        EQU     0x0619
SYS_LSRW3_52        EQU     0x0636
SYS_LSRW4_50        EQU     0x0652
SYS_LSRW5_50        EQU     0x066d
SYS_LSRW6_48        EQU     0x0687
SYS_LSLW4_46        EQU     0x06a0
SYS_Read3_40        EQU     0x06b9
SYS_Unpack_56       EQU     0x06c0
SYS_PayloadCopy_34  EQU     0x06e7
