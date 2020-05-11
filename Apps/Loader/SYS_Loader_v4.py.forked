
#-----------------------------------------------------------------------
#
#       Loader-specific SYS extensions
#
#-----------------------------------------------------------------------

from asm import *

# Peek into the ROM's symbol table
videoY = symbol('videoY')
sysArgs = symbol('sysArgs0')
vPC = symbol('vPC')
vLR = symbol('vLR')
vTmp = symbol('vTmp')
channelMask = symbol('channelMask_v4')

#-----------------------------------------------------------------------
# Extension SYS_LoaderNextByteIn_32
#-----------------------------------------------------------------------

# sysArgs[0:1] Current address
# sysArgs[2]   Checksum
# sysArgs[3]   Wait value (videoY)

label('SYS_LoaderNextByteIn_32')
ld([videoY])                    #15
xora([sysArgs+3])               #16
bne('.sysNbi#19')               #17
ld([sysArgs+0],X)               #18
ld([sysArgs+1],Y)               #19
ld(IN)                          #20
st([Y,X])                       #21
adda([sysArgs+2])               #22
st([sysArgs+2])                 #23
ld([sysArgs+0])                 #24
adda(1)                         #25
st([sysArgs+0])                 #26
ld(hi('REENTER'),Y)             #27
jmp(Y,'REENTER')                #28
ld(-32/2)                       #29
# Restart the instruction in the next timeslice
label('.sysNbi#19')
ld([vPC])                       #19
suba(2)                         #20
st([vPC])                       #21
ld(hi('NEXTY'),Y)               #22
jmp(Y,'NEXTY')                  #23
ld(-26/2)                       #24

#-----------------------------------------------------------------------
# Extension SYS_LoaderProcessInput_64
#-----------------------------------------------------------------------

# sysArgs[0:1] Source address
# sysArgs[2]   Checksum
# sysArgs[4]   Copy count
# sysArgs[5:6] Destination address

label('SYS_LoaderProcessInput_64')
ld([sysArgs+1],Y)               #15
ld([sysArgs+2])                 #16
bne('.sysPi#19')                #17
ld([sysArgs+0])                 #18
suba(65,X)                      #19 Point at first byte of buffer
ld([Y,X])                       #20 Command byte
st([Y,Xpp]);                    C('Just X++')#21
xora(ord('L'))                  #22 This loader lumps everything under 'L'
bne('.sysPi#25')                #23
ld([Y,X]);                      C('Valid command')#24 Length byte
st([Y,Xpp]);                    C('Just X++')#25
anda(63)                        #26 Bit 6:7 are garbage
st([sysArgs+4])                 #27 Copy count 0..60
adda([Y,X])                     #28 One location past (+1) the last byte of fragment
adda(1)                         #29 254+1 = $ff becomes 0, 255+1 = $00 becomes 1
anda(0xfe)                      #30 Will be zero iff writing in top 2 bytes of page
st([vTmp])                      #31 Remember as first condition
ld([Y,X])                       #32 Low copy address
st([Y,Xpp]);                    C('Just X++')#33
st([sysArgs+5])                 #34
ld([Y,X])                       #35 High copy address
st([Y,Xpp]);                    C('Just X++')#36
st([sysArgs+6])                 #37
suba(1)                         #38 Check if writing into sound channel page (1..4)
anda(0xfc)                      #39
ora([vTmp])                     #40 Combine second condition with first
bne(pc()+3)                     #41
bra(pc()+3)                     #42
ld(0xfc);                       C('Unsafe')#43  Clear low channelMask bits so it becomes safe
ld(0xff);                       C('Safe')#43(!) No change to channelMask because already safe
anda([channelMask])             #44
st([channelMask])               #45
ld([sysArgs+4])                 #46
bne('.sysPi#49')                #47
# Execute code (don't care about checksum anymore)
ld([sysArgs+5]);                C('Execute')#48 Low run address
st([vLR])                       #49 https://forum.gigatron.io/viewtopic.php?p=29#p29
suba(2)                         #50
st([vPC])                       #51
ld([sysArgs+6])                 #52 High run address
st([vPC+1])                     #53
st([vLR+1])                     #54
ld(hi('REENTER'),Y)             #55
jmp(Y,'REENTER')                #56
ld(-60/2)                       #57
# Invalid checksum
label('.sysPi#19')
wait(25-19);                    C('Invalid checksum')#19 Reset checksum
# Unknown command
label('.sysPi#25')
ld(ord('g'));                   C('Unknown command')#25 Reset checksum
st([sysArgs+2])                 #26
ld(hi('REENTER'),Y)             #27
jmp(Y,'REENTER')                #28
ld(-32/2)                       #29
# Loading data
label('.sysPi#49')
ld([sysArgs+0]);                C('Loading data')#49 Continue checksum
suba(1,X)                       #50 Point at last byte
ld([Y,X])                       #51
st([sysArgs+2])                 #52
ld(hi('REENTER'),Y)             #53
jmp(Y,'REENTER')                #54
ld(-58/2)                       #55

#-----------------------------------------------------------------------
# Extension SYS_LoaderPayloadCopy_34
#-----------------------------------------------------------------------

# sysArgs[0:1] Source address
# sysArgs[4]   Copy count
# sysArgs[5:6] Destination address

label('SYS_LoaderPayloadCopy_34')
ld([sysArgs+4])                 #15 Copy count
beq('.sysCc#18')                #16
suba(1)                         #17
st([sysArgs+4])                 #18
ld([sysArgs+0],X)               #19 Current pointer
ld([sysArgs+1],Y)               #20
ld([Y,X])                       #21
ld([sysArgs+5],X)               #22 Target pointer
ld([sysArgs+6],Y)               #23
st([Y,X])                       #24
ld([sysArgs+5])                 #25 Increment target
adda(1)                         #26
st([sysArgs+5])                 #27
bra('.sysCc#30')                #28
label('.sysCc#18')
ld(hi('REENTER'),Y)             #18,29
wait(30-19)                     #19
label('.sysCc#30')
jmp(Y,'REENTER')                #30
ld(-34/2)                       #31

#-----------------------------------------------------------------------
#
#-----------------------------------------------------------------------

