
/*
 *  Bindings for ROM v1
 *  XXX: check all numbers as they are hand typed
 * 
 *  Serious attempts should be made to keep all of these stable accross
 *  ROM versions. The location of embedded programs, application data and
 *  application-specific SYS extensions are not part of the defined
 *  bindings and are therefore free to change.
 *
 */

enum ROMv1bindings {
 /*
  *  ROM version/type identifier. There is no real scheme yet.
  *  Suggest to use 0 for home-grown ROMs.
  */
 romTypeROMv1   = 0x001c,

 /* Zero page */
 memSize        = 0x0001,
 bootCount      = 0x0004,
 bootCheck      = 0x0005,
 entropy        = 0x0006,
 videoY         = 0x0009,
 frameCount     = 0x000e,
 serialRaw      = 0x000f,
 buttonState    = 0x0011,
 xout           = 0x0013,
 xoutMask       = 0x0014,
 vPC            = 0x0016,
 vAC            = 0x0018,
 vLR            = 0x001a,
 vSP            = 0x001c,
 romType        = 0x0021,
 sysFn          = 0x0022,
 sysArgs        = 0x0024,
 soundTimer     = 0x002c,
 ledTimer       = 0x002d,
 ledState       = 0x002e,
 ledTempo       = 0x002f,
 vars           = 0x0030,

 /* Main memory */
 videoTable     = 0x0100,
 vReset         = 0x01f0,
 vCpuStart      = 0x0200,
 soundTable     = 0x0700,
 screenPages    = 0x0800,

 /* Sound channels */
 channel1       = 0x01fa,
 channel2       = 0x02fa,
 channel3       = 0x03fa,
 channel4       = 0x04fa,

 /* Sound channel structure */
 wavA   = 250,
 wavX   = 251,
 keyL   = 252,
 keyH   = 253,
 oscL   = 254,
 oscH   = 255,

 /* vCPU byte codes */
 LDWI   = 0x11,
 LD     = 0x1a,
 LDW    = 0x21,
 STW    = 0x2b,
 BCC    = 0x35,
 EQ     = 0x3f,
 GT     = 0x4d,
 LT     = 0x50,
 GE     = 0x53,
 LE     = 0x56,
 LDI    = 0x59,
 ST     = 0x5e,
 NE     = 0x72,
 PUSH   = 0x75,
 LUP    = 0x7f,
 ANDI   = 0x82,
 ORI    = 0x88,
 XORI   = 0x8c,
 BRA    = 0x90,
 INC    = 0x93,
 ADDW   = 0x99,
 PEEK   = 0xad,
 SYS    = 0xb4,
 SUBW   = 0xb8,
 DEF    = 0xcd,
 CALL   = 0xcf,
 ALLOC  = 0xdf,
 ADDI   = 0xe3,
 SUBI   = 0xe6,
 LSLW   = 0xe9,
 STLW   = 0xec,
 LDLW   = 0xee,
 POKE   = 0xf0,
 DOKE   = 0xf3,
 DEEK   = 0xf6,
 ANDW   = 0xf8,
 ORW    = 0xfa,
 XORW   = 0xfc,
 RET    = 0xff,

 /* Core SYS extensions */
 SYS_Reset_36           = 0x009a,
 SYS_Exec_88            = 0x00ad,
 SYS_Out_22             = 0x00f4,
 SYS_In_24              = 0x00f9,
 SYS_NextByteIn         = 0x02e9,
 SYS_Random_34          = 0x04a7,
 SYS_LSRW7_30           = 0x04b9,
 SYS_LSRW8_24           = 0x04c6,
 SYS_LSLW8_24           = 0x04cd,
 SYS_Draw4_30           = 0x04d4,
 SYS_VDrawBits_134      = 0x04e1,
 SYS_LSRW1_48           = 0x0600,
 SYS_LSRW2_52           = 0x0619,
 SYS_LSRW3_52           = 0x0636,
 SYS_LSRW4_50           = 0x0652,
 SYS_LSRW5_50           = 0x066d,
 SYS_LSRW6_48           = 0x0687,
 SYS_LSLW4_46           = 0x06a0,
 SYS_Read3_40           = 0x06b9,
 SYS_Unpack_56          = 0x06c0,
 SYS_PayloadCopy_34     = 0x06e7,

 /* Built-in font */
 font32up               = 0x0700,
 font82up               = 0x0800,
};

