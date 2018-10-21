#include "gigatron.h"


namespace Gigatron
{
    GigaEquate _gigaEquateMap = 
    {
        {"giga_text32",         0x0700},
        {"giga_text82",         0x0800},
        {"giga_notesTable",     0x0900},
        {"giga_vram",           0x0800},
        {"giga_videoTable",     0x0101},
        {"giga_soundChan1",     0x01FA},
        {"giga_soundChan2",     0x02FA},
        {"giga_soundChan3",     0x03FA},
        {"giga_soundChan4",     0x04FA},
        {"giga_xres",              160},
        {"giga_yres",              120},
        {"giga_rand0",            0x06},
        {"giga_rand1",            0x07},
        {"giga_rand2",            0x08},
        {"giga_videoY",           0x09},
        {"giga_buttonState",      0x11},
        {"giga_frameCount",       0x0E},
        {"giga_sysFn",            0x22},
        {"giga_sysArg0",          0x24},
        {"giga_sysArg1",          0x25},
        {"giga_sysArg2",          0x26},
        {"giga_sysArg3",          0x27},
        {"giga_sysArg4",          0x28},
        {"giga_sysArg5",          0x29},
        {"giga_sysArg6",          0x2A},
        {"giga_sysArg7",          0x2B},
        {"giga_soundTimer",       0x2C},

        {"SYS_Reset_36",        0x009a},
        {"SYS_Exec_88",         0x00ad},
        {"SYS_Out_22",          0x00f4},
        {"SYS_In_24",           0x00f9},
        {"SYS_NextByteIn",      0x02e9},
        {"SYS_Random_34",       0x04a7},
        {"SYS_LSRW7_30",        0x04b9},
        {"SYS_LSRW8_24",        0x04c6},
        {"SYS_LSLW8_24",        0x04cd},
        {"SYS_Draw4_30",        0x04d4},
        {"SYS_VDrawBits_134",   0x04e1},
        {"SYS_LSRW1_48",        0x0600},
        {"SYS_LSRW2_52",        0x0619},
        {"SYS_LSRW3_52",        0x0636},
        {"SYS_LSRW4_50",        0x0652},
        {"SYS_LSRW5_50",        0x066d},
        {"SYS_LSRW6_48",        0x0687},
        {"SYS_LSLW4_46",        0x06a0},
        {"SYS_Read3_40",        0x06b9},
        {"SYS_Unpack_56",       0x06c0},
        {"SYS_PayloadCopy_34",  0x06e7},
    };


    
}
