# /* 
%{

/*
#   Copyright (c) 2021, LB3361
#
#    Redistribution and use in source and binary forms, with or
#    without modification, are permitted provided that the following
#    conditions are met:
#
#    1.  Redistributions of source code must retain the above copyright
#        notice, this list of conditions and the following disclaimer.
#
#    2. Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials
#       provided with the distribution.
#
#    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
#    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
#    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
#    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
#    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#    TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
#    ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
#    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#    POSSIBILITY OF SUCH DAMAGE.
*/


/*---- BEGIN HEADER --*/

#include "c.h"
#include <ctype.h>
#include <math.h>

#define NODEPTR_TYPE Node
#define OP_LABEL(p) ((p)->op)
#define LEFT_CHILD(p) ((p)->kids[0])
#define RIGHT_CHILD(p) ((p)->kids[1])
#define STATE_LABEL(p) ((p)->x.state)

static void address(Symbol, Symbol, long);
static void blkfetch(/* int, int, int, int */) {}
static void blkloop(/* int, int, int, int, int, int[] */) {}
static void blkstore(/* int, int, int, int */) {}
static void defaddress(Symbol);
static void defconst(int, int, Value);
static void defstring(int, char *);
static void defsymbol(Symbol);
static void doarg(Node);
static void emit2(/* Node */) {}
static void emit3(const char*, Node, Node*, short*);
static void myemitfmt(const char*, Node, Node*, short*);
static void export(Symbol);
static void clobber(Node);
static void preralloc(Node);
static void function(Symbol, Symbol [], Symbol [], int);
static void global(Symbol);
static void import(Symbol);
static void local(Symbol);
static void progbeg(int, char **);
static void progend(void);
static void segment(int);
static void space(int);
static void target(Node);

/* string lists */
typedef struct slist *SList;
struct slist { SList prev; SList next; char s[1]; };
static struct slist lhead = { &lhead, &lhead, 0 };
static void lprint(const char *fmt, ...);

static int in_function = 0;
static struct slist xhead = { &xhead, &xhead, 0 };
static void xprint_init(void);
static void xprint(const char *fmt, ...);
static void xprint_finish(void);


/* Cost functions */
static int  if_zpconst(Node);
static int  if_zpglobal(Node);
static int  if_rmw(Node,int);
static int  if_not_asgn_tmp(Node,int);
static int  if_cv_from(Node,int,int);
static int  if_arg_reg_only(Node);
static int  if_arg_stk(Node);

#define mincpu5(cost) ((cpu<5)?LBURG_MAX:(cost))
#define mincpu6(cost) ((cpu<6)?LBURG_MAX:(cost))
#define if_spill(cost) ((spilling)?cost:LBURG_MAX)

/* Registers */
static Symbol ireg[32], lreg[32], freg[32];
static Symbol iregw, lregw, fregw;

#define REGMASK_SAVED           0x000000ff
#define REGMASK_ARGS            0x0000ff00
#define REGMASK_MOREVARS        0x000fffff
#define REGMASK_TEMPS           0x007fff00

/* Misc */
static int codenum = 0;
static int cseg = 0;
static int cpu = 5;


/*---- END HEADER --*/
%}
# /*-- BEGIN TERMINALS --/

%start stmt

# From ./ops c=1 s=2 i=2 l=4 h=4 f=5 d=5 x=5 p=2
   
%term CNSTF5=5137
%term CNSTI1=1045 CNSTI2=2069 CNSTI4=4117
%term CNSTP2=2071
%term CNSTU1=1046 CNSTU2=2070 CNSTU4=4118

%term ARGB=41
%term ARGF5=5153
%term ARGI2=2085 ARGI4=4133
%term ARGP2=2087
%term ARGU2=2086 ARGU4=4134

%term ASGNB=57
%term ASGNF5=5169
%term ASGNI1=1077 ASGNI2=2101 ASGNI4=4149
%term ASGNP2=2103
%term ASGNU1=1078 ASGNU2=2102 ASGNU4=4150

%term INDIRB=73
%term INDIRF5=5185
%term INDIRI1=1093 INDIRI2=2117 INDIRI4=4165
%term INDIRP2=2119
%term INDIRU1=1094 INDIRU2=2118 INDIRU4=4166

%term CVFF5=5233
%term CVFI2=2165 CVFI4=4213
%term CVFU2=2166 CVFU4=4214

%term CVIF5=5249
%term CVII1=1157 CVII2=2181 CVII4=4229
%term CVIU1=1158 CVIU2=2182 CVIU4=4230

%term CVPU2=2198

%term CVUF5=5297
%term CVUI1=1205 CVUI2=2229 CVUI4=4277
%term CVUP2=2231
%term CVUU1=1206 CVUU2=2230 CVUU4=4278

%term NEGF5=5313
%term NEGI2=2245 NEGI4=4293

%term CALLB=217
%term CALLF5=5329
%term CALLI2=2261 CALLI4=4309
%term CALLP2=2263
%term CALLU2=2262 CALLU4=4310
%term CALLV=216

%term RETF5=5361
%term RETI2=2293 RETI4=4341
%term RETP2=2295
%term RETU2=2294 RETU4=4342
%term RETV=248

%term ADDRGP2=2311

%term ADDRFP2=2327

%term ADDRLP2=2343

%term ADDF5=5425
%term ADDI2=2357 ADDI4=4405
%term ADDP2=2359
%term ADDU2=2358 ADDU4=4406

%term SUBF5=5441
%term SUBI2=2373 SUBI4=4421
%term SUBP2=2375
%term SUBU2=2374 SUBU4=4422

%term LSHI2=2389 LSHI4=4437
%term LSHU2=2390 LSHU4=4438

%term MODI2=2405 MODI4=4453
%term MODU2=2406 MODU4=4454

%term RSHI2=2421 RSHI4=4469
%term RSHU2=2422 RSHU4=4470

%term BANDI2=2437 BANDI4=4485
%term BANDU2=2438 BANDU4=4486

%term BCOMI2=2453 BCOMI4=4501
%term BCOMU2=2454 BCOMU4=4502

%term BORI2=2469 BORI4=4517
%term BORU2=2470 BORU4=4518

%term BXORI2=2485 BXORI4=4533
%term BXORU2=2486 BXORU4=4534

%term DIVF5=5569
%term DIVI2=2501 DIVI4=4549
%term DIVU2=2502 DIVU4=4550

%term MULF5=5585
%term MULI2=2517 MULI4=4565
%term MULU2=2518 MULU4=4566

%term EQF5=5601
%term EQI2=2533 EQI4=4581
%term EQU2=2534 EQU4=4582

%term GEF5=5617
%term GEI2=2549 GEI4=4597
%term GEU2=2550 GEU4=4598

%term GTF5=5633
%term GTI2=2565 GTI4=4613
%term GTU2=2566 GTU4=4614

%term LEF5=5649
%term LEI2=2581 LEI4=4629
%term LEU2=2582 LEU4=4630

%term LTF5=5665
%term LTI2=2597 LTI4=4645
%term LTU2=2598 LTU4=4646

%term NEF5=5681
%term NEI2=2613 NEI4=4661
%term NEU2=2614 NEU4=4662

%term JUMPV=584

%term LABELV=600

%term LOADB=233
%term LOADF5=5345
%term LOADI1=1253 LOADI2=2277 LOADI4=4325
%term LOADP2=2279
%term LOADU1=1254 LOADU2=2278 LOADU4=4326

%term VREGP=711


# /*-- END TERMINALS --/
%%
# /*-- BEGIN RULES --/

# Once LCC has encoded a C function as a forest of trees, the
# LCC/Lburg code generator computes an optimal cover of the trees with
# assembly instructions in which only the registers are left to be
# specified. This optimality ignores the effects of the limited number
# of registers and the specialization of registers. When such problems
# occur, the register allocator spills registers to memory as needed
# for correctness, but without regard for optimality.  That does not
# work well for a simplistic CPU like the Gigagron VCPU.  Once vAC is
# allocated, there is nothing left one can do.  The following
# repurposes the LCC mechanisms in the following way.  The LCC
# register allocator no longer deals with actual registers but with a
# piece of page zero memory that we call registers.  Instead of
# computing a cover of the trees with instruction, we cover the trees
# with sequences of instructions that use the accumulator vAC and
# the scratch registers (T0..T3) as they see fit. The LBURG grammar 
# is no longer a tree grammar, but a transducer that converts tree 
# fragments into sequences. As a result, each nonterminal must be 
# defined by two components: the role it occupies on the tree grammar
# and the role it occupies in the sequence grammar.

# -- common rules for all LCC targets
reg:  INDIRI1(VREGP)     "# read register\n"
reg:  INDIRU1(VREGP)     "# read register\n"
reg:  INDIRI2(VREGP)     "# read register\n"
reg:  INDIRU2(VREGP)     "# read register\n"
reg:  INDIRP2(VREGP)     "# read register\n"
reg:  INDIRI4(VREGP)     "# read register\n"
reg:  INDIRU4(VREGP)     "# read register\n"
reg:  INDIRF5(VREGP)     "# read register\n"
stmt: ASGNI1(VREGP,reg)  "# write register\n"
stmt: ASGNU1(VREGP,reg)  "# write register\n"
stmt: ASGNI2(VREGP,reg)  "# write register\n"
stmt: ASGNU2(VREGP,reg)  "# write register\n"
stmt: ASGNP2(VREGP,reg)  "# write register\n"
stmt: ASGNI4(VREGP,reg)  "# write register\n"
stmt: ASGNU4(VREGP,reg)  "# write register\n"
stmt: ASGNF5(VREGP,reg)  "# write register\n"
reg: LOADI1(reg)  "\t%{#alsoVAC}%{src!=vAC:LD(%0);}%{dst!=vAC:ST(%c);}\n"   34
reg: LOADU1(reg)  "\t%{#alsoVAC}%{src!=vAC:LD(%0);}%{dst!=vAC:ST(%c);}\n"   34
reg: LOADI2(reg)  "\t%{#alsoVAC}%{src!=vAC:LDW(%0);}%{dst!=vAC:STW(%c);}\n" 40
reg: LOADU2(reg)  "\t%{#alsoVAC}%{src!=vAC:LDW(%0);}%{dst!=vAC:STW(%c);}\n" 40
reg: LOADP2(reg)  "\t%{#alsoVAC}%{src!=vAC:LDW(%0);}%{dst!=vAC:STW(%c);}\n" 40
reg: LOADI4(reg)  "\t%{#mov0}_MOVL(%0,%c);\n" 120
reg: LOADU4(reg)  "\t%{#mov0}_MOVL(%0,%c);\n" 120
reg: LOADF5(reg)  "\t%{#mov0}_MOVF(%0,%c);\n" 150
# -- these were missing, really
reg: LOADI1(conBs) "\t%{#alsoVAC}LDI(%0);%{dst!=vAC:ST(%c);}\n" 36
reg: LOADU1(conB)  "\t%{#alsoVAC}LDI(%0);%{dst!=vAC:ST(%c);}\n" 36
reg: LOADI2(con)   "\t%{#alsoVAC}_LDI(%0);%{dst!=vAC:STW(%c);}\n" 41
reg: LOADU2(con)   "\t%{#alsoVAC}_LDI(%0);%{dst!=vAC:STW(%c);}\n" 41
reg: LOADP2(con)   "\t%{#alsoVAC}_LDI(%0);%{dst!=vAC:STW(%c);}\n" 41

# -- constants
# These non terminal represent constants in the tree grammar
# and the arguments of immediate instructions in the sequence grammar
con0: CNSTI1  "%a"  range(a,0,0)
con0: CNSTU1  "%a"  range(a,0,0)
con0: CNSTI2  "%a"  range(a,0,0)
con0: CNSTU2  "%a"  range(a,0,0)
con0: CNSTP2  "%a"  range(a,0,0)
con1: CNSTI1  "%a"  range(a,1,1)
con1: CNSTU1  "%a"  range(a,1,1)
con1: CNSTI2  "%a"  range(a,1,1)
con1: CNSTU2  "%a"  range(a,1,1)
con1n: CNSTI1  "%a"  range(a,-1,-1)
con1n: CNSTI2  "%a"  range(a,-1,-1)
conB: CNSTI2  "%a"  range(a,0,255)
conB: CNSTU2  "%a"  range(a,0,255)
conB: CNSTP2  "%a"  if_zpconst(a)
conB: CNSTI1  "%a"
conB: CNSTU1  "%a"
conB: zddr    "%0"
conBn: CNSTI2  "%a"  range(a,-255,-1)
conBs: CNSTI2  "%a"  range(a,-128,127)
conBs: CNSTI1  "%a"
con: CNSTI1   "%a"
con: CNSTU1   "%a"
con: CNSTI2   "%a"
con: CNSTU2   "%a"
con: CNSTP2   "%a"
con: addr     "%0"

# -- addresses
# These non terminals represent addresses in the tree grammar
# and represent the operands of address-accepting instructions
# in the sequence gramma
# -- lddr represent a frame offset
# -- addr represent a simple address (equivalent to con)
# -- zddr represent a zero page address (equivalent to conB)
lddr: ADDRLP2 "%a+%F"
lddr: ADDRFP2 "%a+%F"
addr: ADDRGP2 "%a" 
addr: con "%0"
addr: zddr "%0"
zddr: ADDRGP2 "%a" if_zpglobal(a)
zddr: conB "%0"

# -- expressions
# All the following nonterminals represent expressions in the tree grammar.
# They differ by what they represent in the sequence grammar.
# -- reg is the boundary between instruction sequences and
#    the register allocator. When it appears on the left hand side of a rule,
#    it represents a completed sequence of instruction that stores
#    its result in a register to be allocated by ralloc.
#    When it appears on the right hand side of a rule, it represents the
#    register name containing the value of the expression.
# -- ac represents a sequence of instruction that places the expression value
#    into register vAC, potentially clobbeting LAC, FAC, and the scratch registers.
# -- lac and fac are the same but respectively compute long results and fp results
#    into registers LAC or FAC, potentially clobbering vAC, LAC, FAC, and T0-T3.
# -- eac is like ac but cannot clobber LAC, FAC or T2.
# -- ac0/eac0 mean that the high byte of ac is known to be zero

stmt: reg  ""
stmt: ac   "\t%0\n"
reg:  ac   "\t%{#alsoVAC}%0%{dst!=vAC:STW(%c);}\n" 20
ac:  reg   "%{src!=vAC:LDW(%0);}" 20
ac:  reg   "%{src!=vAC:LDW(%0);}" 20
ac:  ac0   "%0"
ac:  eac   "%0" 
ac0: eac0  "%0"
ac0: conB  "LDI(%0);" 16
ac:  con   "_LDI(%0);" 21
ac:  zddr  "LDI(%0);" 16
ac:  addr  "_LDI(%0);" 21
eac:  reg  "%{src!=vAC:LDW(%0);}" 20
eac:  eac0 "%0"
eac0: zddr "LDI(%0);" 16
eac:  addr "_LDI(%0);" 21
eac:  lddr "_SP(%0);"  41


# Loads
eac:  INDIRI2(eac) "%0DEEK();" 21
eac:  INDIRU2(eac) "%0DEEK();" 21
eac:  INDIRP2(eac) "%0DEEK();" 21
eac0: INDIRI1(eac) "%0PEEK();" 17 
eac0: INDIRU1(eac) "%0PEEK();" 17 
eac:  INDIRI2(zddr) "LDW(%0);" 20 
eac:  INDIRU2(zddr) "LDW(%0);" 20 
eac:  INDIRP2(zddr) "LDW(%0);" 20 
eac0: INDIRI1(zddr) "LD(%0);" 18
eac0: INDIRU1(zddr) "LD(%0);" 18
ac:  INDIRI2(ac) "%0DEEK();" 21
ac:  INDIRU2(ac) "%0DEEK();" 21
ac:  INDIRP2(ac) "%0DEEK();" 21
ac0: INDIRI1(ac) "%0PEEK();" 17
ac0: INDIRU1(ac) "%0PEEK();" 17

# -- iarg represents the argument of binary integer operations that
#    map to zero page locations in assembly instructions.  However the
#    spiller needs to be able to reload a register from an auto
#    variable without allocating a register. This is achieved by another
#    branch which defines two fragments using the alternate expansion 
#    mechanism defined by myemitfmt. The two fragments are a register name (T3)
#    and an instruction sequence.
iarg: reg "%0"
iarg: INDIRI2(zddr) "%0"
iarg: INDIRU2(zddr) "%0"
iarg: INDIRP2(zddr) "%0"

spill: ADDRLP2 "%a+%F" if_spill(50)
iarg: INDIRU2(spill) "T3|STW(T1);_MOVW([SP,%0],T3);LDW(T1);" 21
iarg: INDIRI2(spill) "T3|STW(T1);_MOVW([SP,%0],T3);LDW(T1);" 21
iarg: INDIRP2(spill) "T3|STW(T1);_MOVW([SP,%0],T3);LDW(T1);" 21

# Integer operations. This is verbose because there are variants for
# types I2, U2, P2, variants for argument ordering, and variants for
# constant arguments.
ac: ADDI2(ac,iarg)  "%0%[1b]ADDW(%1);" 28
ac: ADDU2(ac,iarg)  "%0%[1b]ADDW(%1);" 28
ac: ADDP2(ac,iarg)  "%0%[1b]ADDW(%1);" 28
ac: ADDI2(iarg,ac)  "%1%[0b]ADDW(%0);" 28
ac: ADDU2(iarg,ac)  "%1%[0b]ADDW(%0);" 28
ac: ADDP2(iarg,ac)  "%1%[0b]ADDW(%0);" 28
ac: ADDI2(ac,conB) "%0ADDI(%1);" 27
ac: ADDU2(ac,conB) "%0ADDI(%1);" 27
ac: ADDP2(ac,conB) "%0ADDI(%1);" 27
ac: ADDI2(ac,conBn) "%0SUBI(-(%1));" 27
ac: ADDU2(ac,conBn) "%0SUBI(-(%1));" 27
ac: ADDP2(ac,conBn) "%0SUBI(-(%1));" 27
ac: SUBI2(ac,iarg)  "%0%[1b]SUBW(%1);" 28
ac: SUBU2(ac,iarg)  "%0%[1b]SUBW(%1);" 28
ac: SUBP2(ac,iarg)  "%0%[1b]SUBW(%1);" 28
ac: SUBI2(ac,conB) "%0SUBI(%1);" 27
ac: SUBU2(ac,conB) "%0SUBI(%1);" 27
ac: SUBP2(ac,conB) "%0SUBI(%1);" 27
ac: SUBI2(ac,conBn) "%0ADDI(-(%1));" 27
ac: SUBU2(ac,conBn) "%0ADDI(-(%1));" 27
ac: SUBP2(ac,conBn) "%0ADDI(-(%1));" 27
ac: NEGI2(ac)       "%0STW(T3);LDI(0);SUBW(T3);" 68
ac: NEGI2(reg)      "LDI(0);SUBW(%0);" 48
ac: LSHI2(ac, con1) "%0LSLW();" 28
ac: LSHU2(ac, con1) "%0LSLW();" 28
ac: LSHI2(ac, conB) "%0_SHLI(%1);" 100
ac: LSHU2(ac, conB) "%0_SHLI(%1);" 100
ac: RSHI2(ac, conB) "%0_SHRIS(%1);" 100
ac: RSHU2(ac, conB) "%0_SHRIU(%1);" 100
ac: LSHI2(ac, iarg) "%0%[1b]_SHL(%1);" 200
ac: RSHI2(ac, iarg) "%0%[1b]_SHRS(%1);" 200
ac: LSHU2(ac, iarg) "%0%[1b]_SHL(%1);" 200
ac: RSHU2(ac, iarg) "%0%[1b]_SHRU(%1);" 200
ac: MULI2(conB, ac) "%1%{mul0}" 100
ac: MULI2(conBn, ac) "%1%{mul0}" 110
ac: MULI2(conB, reg) "%{mul0%1}" 100
ac: MULI2(conBn, reg) "%{mul0%1}" 110
ac: MULI2(con, ac)  "%1_MULI(%0);" 200
ac: MULI2(ac, iarg) "%0%[1b]_MUL(%1);" 200
ac: MULI2(iarg, ac) "%1%[0b]_MUL(%0);" 200
ac: MULU2(conB, ac) "%1%{mul0}" 100
ac: MULU2(con, ac)  "%1_MULI(%0);" 200
ac: MULU2(ac, iarg) "%0%[1b]_MUL(%1);" 200
ac: MULU2(iarg, ac) "%1%[0b]_MUL(%0);" 200
ac: DIVI2(ac, iarg) "%0%[1b]_DIVS(%1);" 200
ac: DIVU2(ac, iarg) "%0%[1b]_DIVU(%1);" 200
ac: MODI2(ac, iarg) "%0%[1b]_MODS(%1);" 200
ac: MODU2(ac, iarg) "%0%[1b]_MODU(%1);" 200
ac: DIVI2(ac, con)  "%0%[1b]_DIVIS(%1);" 200
ac: DIVU2(ac, con)  "%0%[1b]_DIVIU(%1);" 200
ac: MODI2(ac, con)  "%0%[1b]_MODIS(%1);" 200
ac: MODU2(ac, con)  "%0%[1b]_MODIU(%1);" 200
ac: BCOMI2(ac)      "%0STW(T3);_LDI(-1);XORW(T3);" 68
ac: BCOMU2(ac)      "%0STW(T3);_LDI(-1);XORW(T3);" 68
ac: BANDI2(ac,iarg)  "%0%[1b]ANDW(%1);" 28
ac: BANDU2(ac,iarg)  "%0%[1b]ANDW(%1);" 28
ac: BANDI2(iarg,ac)  "%1%[0b]ANDW(%0);" 28
ac: BANDU2(iarg,ac)  "%1%[0b]ANDW(%0);" 28
ac: BANDI2(ac,conB)  "%0ANDI(%1);" 16 
ac: BANDU2(ac,conB)  "%0ANDI(%1);" 16 
ac: BORI2(ac,iarg)  "%0%[1b]ORW(%1);" 28
ac: BORU2(ac,iarg)  "%0%[1b]ORW(%1);" 28
ac: BORI2(iarg,ac)  "%1%[0b]ORW(%0);" 28
ac: BORU2(iarg,ac)  "%1%[0b]ORW(%0);" 28
ac: BORI2(ac,conB)  "%0ORI(%1);" 16 
ac: BORU2(ac,conB)  "%0ORI(%1);" 16 
ac: BXORI2(ac,iarg)  "%0%[1b]XORW(%1);" 28
ac: BXORU2(ac,iarg)  "%0%[1b]XORW(%1);" 28
ac: BXORI2(iarg,ac)  "%1%[0b]XORW(%0);" 28
ac: BXORU2(iarg,ac)  "%1%[0b]XORW(%0);" 28
ac: BXORI2(ac,conB)  "%0XORI(%1);" 16 
ac: BXORU2(ac,conB)  "%0XORI(%1);" 

# A couple EAC variants
eac: ADDI2(eac,conB) "%0ADDI(%1);" 28
eac: ADDU2(eac,conB) "%0ADDI(%1);" 28
eac: ADDP2(eac,conB) "%0ADDI(%1);" 28
eac: ADDI2(eac,conBn) "%0SUBI(-(%1));" 28
eac: ADDU2(eac,conBn) "%0SUBI(-(%1));" 28
eac: ADDP2(eac,conBn) "%0SUBI(-(%1));" 28
eac: SUBI2(eac,conB) "%0SUBI(%1);" 28
eac: SUBU2(eac,conB) "%0SUBI(%1);" 28
eac: SUBP2(eac,conB) "%0SUBI(%1);" 28
eac: SUBI2(eac,conBn) "%0ADDI(-(%1));" 28
eac: SUBU2(eac,conBn) "%0ADDI(-(%1));" 28
eac: SUBP2(eac,conBn) "%0ADDI(-(%1));" 28
eac: LSHI2(eac, con1) "%0LSLW();" 28
eac: LSHU2(eac, con1) "%0LSLW();" 28
eac: LSHI2(eac, conB) "%0_SHLI(%1);" 100
eac: LSHU2(eac, conB) "%0_SHLI(%1);" 100
eac: MULI2(conB, eac) "%1%{mul0}" 100
eac: MULI2(conB, eac) "%1%{mul0}" 100
# More eac variants involving iarg because iarg spills preserve T2
eac: ADDI2(eac,iarg) "%0%[1b]ADDW(%1);" 28
eac: ADDU2(eac,iarg) "%0%[1b]ADDW(%1);" 28
eac: ADDP2(eac,iarg) "%0%[1b]ADDW(%1);" 28
eac: ADDI2(iarg,eac) "%1%[0b]ADDW(%0);" 28
eac: ADDU2(iarg,eac) "%1%[0b]ADDW(%0);" 28
eac: ADDP2(iarg,eac) "%1%[0b]ADDW(%0);" 28
eac: SUBI2(eac,iarg) "%0%[1b]SUBW(%1);" 28
eac: SUBU2(eac,iarg) "%0%[1b]SUBW(%1);" 28
eac: SUBP2(eac,iarg) "%0%[1b]SUBW(%1);" 28

# More assignments (indirect and explicit addresses)
stmt: ASGNP2(zddr,ac)  "\t%1STW(%0);\n" 20
stmt: ASGNP2(iarg,ac)  "\t%1%[0b]DOKE(%0);\n" 28
stmt: ASGNI2(zddr,ac)  "\t%1STW(%0);\n" 20
stmt: ASGNI2(iarg,ac)  "\t%1%[0b]DOKE(%0);\n" 28
stmt: ASGNU2(zddr,ac)  "\t%1STW(%0);\n" 20
stmt: ASGNU2(iarg,ac)  "\t%1%[0b]DOKE(%0);\n" 28
stmt: ASGNI1(zddr,ac)  "\t%1ST(%0);\n" 20
stmt: ASGNI1(iarg,ac)  "\t%1%[0b]POKE(%0);\n" 26
stmt: ASGNU1(zddr,ac)  "\t%1ST(%0);\n" 20
stmt: ASGNU1(iarg,ac)  "\t%1%[0b]POKE(%0);\n" 26

# Conditional branches
stmt: EQI2(ac,con0)  "\t%0_BEQ(%a);\n" 28
stmt: EQI2(ac,conB)  "\t%0XORI(%1);_BEQ(%a);\n" 42
stmt: EQI2(ac,iarg)  "\t%0%[1b]XORW(%1);_BEQ(%a);\n" 54
stmt: EQI2(iarg,ac)  "\t%1%[0b]XORW(%0);_BEQ(%a);\n" 54
stmt: NEI2(ac,con0)  "\t%0_BNE(%a);\n" 28
stmt: NEI2(ac,conB)  "\t%0XORI(%1);_BNE(%a);\n" 42
stmt: NEI2(ac,iarg)  "\t%0%[1b]XORW(%1);_BNE(%a);\n" 54
stmt: NEI2(iarg,ac)  "\t%1%[0b]XORW(%0);_BNE(%a);\n" 54
stmt: EQU2(ac,con0)  "\t%0_BEQ(%a);\n" 28
stmt: EQU2(ac,conB)  "\t%0XORI(%1);_BEQ(%a);\n" 42
stmt: EQU2(ac,iarg)  "\t%0%[1b]XORW(%1);_BEQ(%a);\n" 54
stmt: EQU2(iarg,ac)  "\t%1%[0b]XORW(%0);_BEQ(%a);\n" 54
stmt: NEU2(ac,con0)  "\t%0_BNE(%a);\n" 28
stmt: NEU2(ac,conB)  "\t%0XORI(%1);_BNE(%a);\n" 42
stmt: NEU2(ac,iarg)  "\t%0%[1b]XORW(%1);_BNE(%a);\n" 54
stmt: NEU2(iarg,ac)  "\t%1%[0b]XORW(%0);_BNE(%a);\n" 54
stmt: LTI2(ac,con0) "\t%0_BLT(%a);\n" 28
stmt: LEI2(ac,con0) "\t%0_BLE(%a);\n" 28
stmt: GTI2(ac,con0) "\t%0_BGT(%a);\n" 28
stmt: GEI2(ac,con0) "\t%0_BGE(%a);\n" 28
stmt: GTU2(ac,con0) "\t%0_BNE(%a);\n" 28
stmt: LEU2(ac,con0) "\t%0_BEQ(%a);\n" 28
stmt: LTI2(ac,conB) "\t%0_CMPIS(%1);_BLT(%a);\n" 80
stmt: LEI2(ac,conB) "\t%0_CMPIS(%1);_BLE(%a);\n" 80
stmt: GTI2(ac,conB) "\t%0_CMPIS(%1);_BGT(%a);\n" 80
stmt: GEI2(ac,conB) "\t%0_CMPIS(%1);_BGE(%a);\n" 80
stmt: LTU2(ac,conB) "\t%0_CMPIU(%1);_BLT(%a);\n" 80
stmt: LEU2(ac,conB) "\t%0_CMPIU(%1);_BLE(%a);\n" 80
stmt: GTU2(ac,conB) "\t%0_CMPIU(%1);_BGT(%a);\n" 80
stmt: GEU2(ac,conB) "\t%0_CMPIU(%1);_BGE(%a);\n" 80
stmt: LTI2(ac,iarg) "\t%0%[1b]_CMPWS(%1);_BLT(%a);\n" 100
stmt: LEI2(ac,iarg) "\t%0%[1b]_CMPWS(%1);_BLE(%a);\n" 100
stmt: GTI2(ac,iarg) "\t%0%[1b]_CMPWS(%1);_BGT(%a);\n" 100
stmt: GEI2(ac,iarg) "\t%0%[1b]_CMPWS(%1);_BGE(%a);\n" 100
stmt: LTU2(ac,iarg) "\t%0%[1b]_CMPWU(%1);_BLT(%a);\n" 100
stmt: LEU2(ac,iarg) "\t%0%[1b]_CMPWU(%1);_BLE(%a);\n" 100
stmt: GTU2(ac,iarg) "\t%0%[1b]_CMPWU(%1);_BGT(%a);\n" 100
stmt: GEU2(ac,iarg) "\t%0%[1b]_CMPWU(%1);_BGE(%a);\n" 100
stmt: LTI2(iarg,ac) "\t%1%[0b]_CMPWS(%0);_BGT(%a);\n" 100
stmt: LEI2(iarg,ac) "\t%1%[0b]_CMPWS(%0);_BGE(%a);\n" 100
stmt: GTI2(iarg,ac) "\t%1%[0b]_CMPWS(%0);_BLT(%a);\n" 100
stmt: GEI2(iarg,ac) "\t%1%[0b]_CMPWS(%0);_BLE(%a);\n" 100
stmt: LTU2(iarg,ac) "\t%1%[0b]_CMPWU(%0);_BGT(%a);\n" 100
stmt: LEU2(iarg,ac) "\t%1%[0b]_CMPWU(%0);_BGE(%a);\n" 100
stmt: GTU2(iarg,ac) "\t%1%[0b]_CMPWU(%0);_BLT(%a);\n" 100
stmt: GEU2(iarg,ac) "\t%1%[0b]_CMPWU(%0);_BLE(%a);\n" 100

# Nonterminals for assignments with MOVM/MOVL/MOVF:
#   stmt: ASGNx(vdst,xAC) "\t%1%[0b]_xMOV(xAC,%0);\n"
#   stmt: ASGNx(vdst,reg) "\t%{#mov1}%[0b]_xMOV(%1,%0);\n"
#   stmt: ASGNx(addr,INDIRx(asrc)) "\t%[1b]_xMOV(%1,%0);\n"
#   stmt: ASGNx(ac,  INDIRx(asrc)) "\t%0STW(T2);%[1b]_xMOV(%1,[T2]);\n"
#   stmt: ASGNx(lddr,INDIRx(lsrc)) "\t_xMOV(%1,[SP,%0]);\n"
vdst: addr "%0"
vdst: lddr "[SP,%0]" 60
vdst: eac "[vAC]|%0" 20
asrc: addr "%0"
asrc: lddr "[SP,%0]" 40
asrc: eac "[vAC]|%0"
lsrc: addr "%0"

# Structs
stmt: ARGB(INDIRB(asrc))        "\t_SP(%c);STW(T2);%[0b]_MOVM(%0,[T2],%a,%b);\n"  200
stmt: ASGNB(addr,INDIRB(asrc)) "\t%[1b]_MOVM(%1,%0,%a,%b);\n" 200
stmt: ASGNB(ac,  INDIRB(asrc)) "\t%0STW(T2);%[1b]_MOVM(%1,[T2],%a,%b);\n" 200
stmt: ASGNB(lddr,INDIRB(lsrc)) "\t_MOVM(%1,[SP,%0],%a,%b);\n" 200

# Longs
# - larg represent argument expressions in binary tree nodes,
#   as well as sequence of instructions that compute the address 
#   holding the expressiong result.
stmt: lac "\t%0\n"
larg: reg "LDI(%0);" 21
larg: INDIRI4(eac) "%0" 
larg: INDIRU4(eac) "%0" 
reg: lac "\t%{#alsoLAC}%0%{dst!=LAC:_MOVL(LAC,%c);}\n" 120
reg: INDIRI4(ac)   "\t%0_MOVL([vAC],%c);\n" 120
reg: INDIRU4(ac)   "\t%0_MOVL([vAC],%c);\n" 120
reg: INDIRI4(lddr) "\t_MOVL([SP,%0],%c);\n" 160
reg: INDIRU4(lddr) "\t_MOVL([SP,%0],%c);\n" 160
reg: INDIRI4(addr) "\t_MOVL(%0,%c);\n" 120
reg: INDIRU4(addr) "\t_MOVL(%0,%c);\n" 120
lac: reg "%{src!=LAC:_MOVL(%0,LAC);}" 120
lac: INDIRI4(ac)   "%0_MOVL([vAC],LAC);" 120
lac: INDIRU4(ac)   "%0_MOVL([vAC],LAC);" 120
lac: INDIRU4(lddr) "_MOVL([SP,%0],LAC);" 160
lac: INDIRU4(lddr) "_MOVL([SP,%0],LAC);" 160
lac: INDIRI4(addr) "_MOVL(%0,LAC);" 120
lac: INDIRU4(addr) "_MOVL(%0,LAC);" 120
lac: ADDI4(lac,larg) "%0%1_LADD();" 200
lac: ADDU4(lac,larg) "%0%1_LADD();" 200
lac: ADDI4(larg,lac) "%1%0_LADD();" 200
lac: ADDU4(larg,lac) "%1%0_LADD();" 200
lac: SUBI4(lac,larg) "%0%1_LSUB();" 200
lac: SUBU4(lac,larg) "%0%1_LSUB();" 200
lac: MULI4(lac,larg) "%0%1_LMUL();" 200
lac: MULU4(lac,larg) "%0%1_LMUL();" 200
lac: MULI4(larg,lac) "%1%0_LMUL();" 200
lac: MULU4(larg,lac) "%1%0_LMUL();" 200
lac: DIVI4(lac,larg) "%0%1_LDIVS();" 200
lac: DIVU4(lac,larg) "%0%1_LDIVU();" 200
lac: MODI4(lac,larg) "%0%1_LMODS();" 200
lac: MODU4(lac,larg) "%0%1_LMODU();" 200
lac: LSHI4(lac,reg)  "%0LDW(%1);_LSHL();" 200
lac: LSHI4(lac,conB) "%0LDI(%1);_LSHL();"  200
lac: LSHU4(lac,reg)  "%0LDW(%1);_LSHL();"  200
lac: LSHU4(lac,conB) "%0LDI(%1);_LSHL();"  200
lac: RSHI4(lac,reg)  "%0LDW(%1);_LSHRS();" 200
lac: RSHI4(lac,conB) "%0LDI(%1);_LSHRS();" 200
lac: RSHU4(lac,reg)  "%0LDW(%1);_LSHRU();" 200
lac: RSHU4(lac,conB) "%0LDI(%1);_LSHRU();" 200
lac: NEGI4(lac) "%0_LNEG();" 200
lac: BCOMU4(lac) "%0_LCOM();" 200
lac: BANDU4(lac,larg) "%0%1_LAND();" 200
lac: BANDU4(larg,lac) "%1%0_LAND();" 200
lac: BORU4(lac,larg) "%0%1_LOR();" 200
lac: BORU4(larg,lac) "%1%0_LOR();" 200
lac: BXORU4(lac,larg) "%0%1_LXOR();" 200
lac: BXORU4(larg,lac) "%1%0_LXOR();" 200
lac: BCOMI4(lac) "%0_LCOM();" 200
lac: BANDI4(lac,larg) "%0%1_LAND();" 200
lac: BANDI4(larg,lac) "%1%0_LAND();" 200
lac: BORI4(lac,larg) "%0%1_LOR();" 200
lac: BORI4(larg,lac) "%1%0_LOR();" 200
lac: BXORI4(lac,larg) "%0%1_LXOR();" 200
lac: BXORI4(larg,lac) "%1%0_LXOR();" 200
stmt: LTI4(lac,larg) "\t%0%1_LCMPS();_BLT(%a);\n" 200
stmt: LEI4(lac,larg) "\t%0%1_LCMPS();_BLE(%a);\n" 200
stmt: GTI4(lac,larg) "\t%0%1_LCMPS();_BGT(%a);\n" 200
stmt: GEI4(lac,larg) "\t%0%1_LCMPS();_BGE(%a);\n" 200
stmt: LTU4(lac,larg) "\t%0%1_LCMPU();_BLT(%a);\n" 200
stmt: LEU4(lac,larg) "\t%0%1_LCMPU();_BLE(%a);\n" 200
stmt: GTU4(lac,larg) "\t%0%1_LCMPU();_BGT(%a);\n" 200
stmt: GEU4(lac,larg) "\t%0%1_LCMPU();_BGE(%a);\n" 200
stmt: NEI4(lac,larg) "\t%0%1_LCMPX();_BNE(%a);\n" 100
stmt: EQI4(lac,larg) "\t%0%1_LCMPX();_BEQ(%a);\n" 100
stmt: NEU4(lac,larg) "\t%0%1_LCMPX();_BNE(%a);\n" 100
stmt: EQU4(lac,larg) "\t%0%1_LCMPX();_BEQ(%a);\n" 100
stmt: LTI4(larg,lac) "\t%1%0_LCMPS();_BGT(%a);\n" 200
stmt: LEI4(larg,lac) "\t%1%0_LCMPS();_BGE(%a);\n" 200
stmt: GTI4(larg,lac) "\t%1%0_LCMPS();_BLT(%a);\n" 200
stmt: GEI4(larg,lac) "\t%1%0_LCMPS();_BLE(%a);\n" 200
stmt: LTU4(larg,lac) "\t%1%0_LCMPU();_BGT(%a);\n" 200
stmt: LEU4(larg,lac) "\t%1%0_LCMPU();_BGE(%a);\n" 200
stmt: GTU4(larg,lac) "\t%1%0_LCMPU();_BLT(%a);\n" 200
stmt: GEU4(larg,lac) "\t%1%0_LCMPU();_BLE(%a);\n" 200
stmt: NEI4(larg,lac) "\t%1%0_LCMPX();_BNE(%a);\n" 100
stmt: EQI4(larg,lac) "\t%1%0_LCMPX();_BEQ(%a);\n" 100
stmt: NEU4(larg,lac) "\t%1%0_LCMPX();_BNE(%a);\n" 100
stmt: EQU4(larg,lac) "\t%1%0_LCMPX();_BEQ(%a);\n" 100
stmt: ASGNI4(vdst,lac)           "\t%1%[0b]_MOVL(LAC,%0);\n" 120
stmt: ASGNI4(vdst,reg)           "\t%{#mov1}%[0b]_MOVL(%1,%0);\n" 120
stmt: ASGNI4(addr,INDIRI4(asrc)) "\t%[1b]_MOVL(%1,%0);\n" 120
stmt: ASGNI4(ac,  INDIRI4(asrc)) "\t%0STW(T2);%[1b]_MOVL(%1,[T2]);\n" 120
stmt: ASGNI4(lddr,INDIRI4(lsrc)) "\t_MOVL(%1,[SP,%0]);\n" 120
stmt: ASGNU4(vdst,lac)           "\t%1%[0b]_MOVL(LAC,%0);\n" 120
stmt: ASGNU4(vdst,reg)           "\t%{#mov1}%[0b]_MOVL(%1,%0);\n" 120
stmt: ASGNU4(addr,INDIRU4(asrc)) "\t%[1b]_MOVL(%1,%0);\n" 120
stmt: ASGNU4(ac,  INDIRU4(asrc)) "\t%0STW(T2);%[1b]_MOVL(%1,[T2]);\n" 120
stmt: ASGNU4(lddr,INDIRU4(lsrc)) "\t_MOVL(%1,[SP,%0]);\n" 120

# Floats
stmt: fac "\t%0\n"
farg: reg "LDI(%0);" 21
farg: INDIRF5(eac) "%0"
reg: fac "\t%{#alsoFAC}%0%{dst!=FAC:_MOVF(FAC,%c);}\n" 180
reg: INDIRF5(ac)   "\t%0_MOVF([vAC],%c);\n" 150
reg: INDIRF5(lddr) "\t_MOVF([SP,%0],%c);\n" 190
reg: INDIRF5(addr) "\t_MOVF(%0,%c);\n" 150
fac: reg            "%{src!=FAC:_MOVF(%0,FAC);}" 150
fac: INDIRF5(ac)    "%0_MOVF([vAC],FAC);" 180
fac: INDIRF5(lddr)  "_MOVF([SP,%0],FAC);" 220
fac: INDIRF5(addr)  "_MOVF(%0,FAC);" 180
fac: ADDF5(fac,farg) "%0%1_FADD();" 200
fac: ADDF5(farg,fac) "%1%0_FADD();" 200
fac: SUBF5(fac,farg) "%0%1_FSUB();" 200
fac: SUBF5(farg,fac) "%1_FNEG();%0_FADD();" 200+50
fac: MULF5(fac,farg) "%0%1_FMUL();" 200
fac: MULF5(farg,fac) "%1%0_FMUL();" 200
fac: DIVF5(fac,farg) "%0%1_FDIV();" 200
fac: NEGF5(fac)      "%0_FNEG();" 50
stmt: EQF5(fac,farg) "\t%0%1_FCMP();_BEQ(%a);\n" 200
stmt: NEF5(fac,farg) "\t%0%1_FCMP();_BNE(%a);\n" 200
stmt: LTF5(fac,farg) "\t%0%1_FCMP();_BLT(%a);\n" 200
stmt: LEF5(fac,farg) "\t%0%1_FCMP();_BLE(%a);\n" 200
stmt: GTF5(fac,farg) "\t%0%1_FCMP();_BGT(%a);\n" 200
stmt: GEF5(fac,farg) "\t%0%1_FCMP();_BGE(%a);\n" 200
stmt: EQF5(farg,fac) "\t%1%0_FCMP();_BEQ(%a);\n" 200
stmt: NEF5(farg,fac) "\t%1%0_FCMP();_BNE(%a);\n" 200
stmt: LTF5(farg,fac) "\t%1%0_FCMP();_BGT(%a);\n" 200
stmt: LEF5(farg,fac) "\t%1%0_FCMP();_BGE(%a);\n" 200
stmt: GTF5(farg,fac) "\t%1%0_FCMP();_BLT(%a);\n" 200
stmt: GEF5(farg,fac) "\t%1%0_FCMP();_BLE(%a);\n" 200
stmt: ASGNF5(vdst,fac) "\t%1%[0b]_MOVF(FAC,%0);\n" 180
stmt: ASGNF5(vdst,reg) "\t%{#mov1}%[0b]_MOVF(%1,%0);\n"    150
stmt: ASGNF5(addr,INDIRF5(asrc)) "\t%[1b]_MOVF(%1,%0);\n" 150
stmt: ASGNF5(ac,  INDIRF5(asrc)) "\t%0STW(T2);%[1b]_MOVF(%1,[T2]);\n" 150
stmt: ASGNF5(lddr,INDIRF5(lsrc)) "\t_MOVF(%1,[SP,%0]);\n" 150

# Calls
fac: CALLF5(addr) "CALLI(%0);" mincpu5(28)
fac: CALLF5(reg)  "CALL(%0);" 26
fac: CALLF5(ac)   "%0CALL(vAC);" 26
lac: CALLI4(addr) "CALLI(%0);" mincpu5(28)
lac: CALLI4(reg)  "CALL(%0);" 26
lac: CALLI4(ac)   "%0CALL(vAC);" 26
lac: CALLU4(addr) "CALLI(%0);" mincpu5(28)
lac: CALLU4(reg)  "CALL(%0);" 26
lac: CALLU4(ac)   "%0CALL(vAC);" 26
ac: CALLI2(addr)  "CALLI(%0);" mincpu5(28)
ac: CALLI2(reg)   "CALL(%0);" 26
ac: CALLI2(ac)    "%0CALL(vAC);" 26
ac: CALLU2(addr)  "CALLI(%0);" mincpu5(28)
ac: CALLU2(reg)   "CALL(%0);" 26
ac: CALLU2(ac)    "%0CALL(vAC);" 26
ac: CALLP2(addr)  "CALLI(%0);" mincpu5(28)
ac: CALLP2(reg)   "CALL(%0);" 26
ac: CALLP2(ac)    "%0CALL(vAC);" 26
stmt: CALLV(addr) "\tCALLI(%0);\n" mincpu5(28)
stmt: CALLV(reg)  "\tCALL(%0);\n" 26
stmt: CALLV(ac)   "\t%0CALL(vAC);\n" 26
stmt: ARGF5(reg)  "\t_MOVF(%0,[SP,%c]);\n"  if_arg_stk(a)
stmt: ARGI4(reg)  "\t_MOVL(%0,[SP,%c]);\n"  if_arg_stk(a)
stmt: ARGU4(reg)  "\t_MOVL(%0,[SP,%c]);\n"  if_arg_stk(a)
stmt: ARGI2(reg)  "\t_MOVW(%0,[SP,%c]);\n"  if_arg_stk(a)
stmt: ARGU2(reg)  "\t_MOVW(%0,[SP,%c]);\n"  if_arg_stk(a)
stmt: ARGP2(reg)  "\t_MOVW(%0,[SP,%c]);\n"  if_arg_stk(a)
stmt: ARGF5(reg)  "# arg\n"  if_arg_reg_only(a)
stmt: ARGI4(reg)  "# arg\n"  if_arg_reg_only(a)
stmt: ARGU4(reg)  "# arg\n"  if_arg_reg_only(a)
stmt: ARGI2(reg)  "# arg\n"  if_arg_reg_only(a)
stmt: ARGU2(reg)  "# arg\n"  if_arg_reg_only(a)
stmt: ARGP2(reg)  "# arg\n"  if_arg_reg_only(a)
stmt: RETF5(fac)  "\t%0\n"  1
stmt: RETI4(lac)  "\t%0\n"  1
stmt: RETU4(lac)  "\t%0\n"  1
stmt: RETI2(ac)   "\t%0\n"  1
stmt: RETU2(ac)   "\t%0\n"  1
stmt: RETP2(ac)   "\t%0\n"  1

# Conversions
#            I1   U1
#              \ /
#              I2 - U2 - P
#            /  | X |
#         F5 - I4 - U4
# 1) prelabel changes all truncations into LOADs
ac0: LOADI1(reg) "LD(%0);" 18
ac0: LOADU1(reg) "LD(%0);" 18
ac: LOADI1(ac) "%0"
ac: LOADU1(ac) "%0"
ac: LOADI2(ac) "%0"
ac: LOADU2(ac) "%0"
ac: LOADP2(ac) "%0"
ac: LOADI2(reg) "LDW(%0);" 20
ac: LOADU2(reg) "LDW(%0);" 20
ac: LOADP2(reg) "LDW(%0);" 20
ac: LOADI2(lac) "%0LDW(LAC);" 20
ac: LOADU2(lac) "%0LDW(LAC);" 20
ac: LOADP2(lac) "%0LDW(LAC);" 20
lac: LOADI4(lac) "%0"
lac: LOADU4(lac) "%0"
fac: LOADF5(fac) "%0"

# 2) extensions
ac: CVII2(reg) "LD(%0);XORI(128);SUBI(128);" if_cv_from(a,1,66)
ac: CVUI2(reg) "LD(%0);" if_cv_from(a,1,18)
ac: CVII2(ac0) "%0XORI(128);SUBI(128);" if_cv_from(a,1,48)
ac: CVUI2(ac0) "%0" if_cv_from(a,1,0)
ac: CVII2(ac) "%0LD(vACL);XORI(128);SUBI(128);" if_cv_from(a,1,66)
ac: CVUI2(ac) "%0LD(vACL);" if_cv_from(a,1,18)
lac: CVIU4(ac) "%0_STLU(LAC);" 50
lac: CVII4(ac) "%0_STLS(LAC);" 50
lac: CVUU4(ac) "%0_STLU(LAC);" 50
lac: CVUI4(ac) "%0_STLU(LAC);" 50
reg: CVIU4(ac) "\t%0_STLU(%c);\n" 50
reg: CVII4(ac) "\t%0_STLS(%c);\n" 50
reg: CVUU4(ac) "\t%0_STLU(%c);\n" 50
reg: CVUI4(ac) "\t%0_STLU(%c);\n" 50
# 3) floating point conversions
ac: CVFU2(fac)  "%0_FTOU();LDW(LAC);" 200
lac: CVFU4(fac) "%0_FTOU();" 200
fac: CVUF5(ac)  "%0_STLU(LAC);_FCVU();" if_cv_from(a,2,180)
fac: CVUF5(lac) "%0_FCVU();" if_cv_from(a,4,200)
ac: CVFI2(fac)  "%0_FTOI();LDW(LAC);" 200
lac: CVFI4(fac) "%0_FTOI();" 200
fac: CVIF5(ac)  "%0_STLS(LAC);_FCVI();" if_cv_from(a,2,180)
fac: CVIF5(lac) "%0_FCVI();" if_cv_from(a,4,200)

# Labels and jumps
stmt: LABELV       "\tlabel(%a);\n"
stmt: JUMPV(addr)  "\t_BRA(%0);\n"    14
stmt: JUMPV(reg)   "\tCALL(%0);\n"    14
stmt: JUMPV(ac)    "\t%0CALL(vAC);\n" 14

# More about spills: we want to save/restore vAC when genspill() inserts
# instructions because preralloc might have decided to use vAC at this
# precise point.
stmt: ASGNI2(spill,reg) "\tSTW(T3);_MOVW(%1,[SP,%0]);LDW(T3) #genspill\n" 0
stmt: ASGNU2(spill,reg) "\tSTW(T3);_MOVW(%1,[SP,%0]);LDW(T3) #genspill\n" 0
stmt: ASGNP2(spill,reg) "\tSTW(T3);_MOVW(%1,[SP,%0]);LDW(T3) #genspill\n" 0
stmt: ASGNI4(spill,reg) "\tSTW(T3);_MOVL(%1,[SP,%0]);LDW(T3) #genspill\n" 0
stmt: ASGNU4(spill,reg) "\tSTW(T3);_MOVL(%1,[SP,%0]);LDW(T3) #genspill\n" 0
stmt: ASGNF5(spill,reg) "\tSTW(T3);_MOVF(%1,[SP,%0]);LDW(T3) #genspill\n" 0

# More opcodes for cpu=6
reg: LOADI1(reg)  "\t%{#keepsAC}MOVB(%0,%c)\n" mincpu6(28)
reg: LOADU1(reg)  "\t%{#keepsAC}MOVB(%0,%c)\n" mincpu6(28)
reg: LOADI2(reg)  "\t%{#keepsAC}MOVW(%0,%c)\n" mincpu6(22+30)
reg: LOADU2(reg)  "\t%{#keepsAC}MOVW(%0,%c)\n" mincpu6(22+30)
reg: LOADP2(reg)  "\t%{#keepsAC}MOVW(%0,%c)\n" mincpu6(22+30)
reg: LOADP2(reg)  "\t%{#keepsAC}MOVW(%0,%c)\n" mincpu6(22+30)
reg: LOADI4(reg)  "\t%{#keepsAC}MOVL(%0,%c)\n" mincpu6(22+40)
reg: LOADU4(reg)  "\t%{#keepsAC}MOVL(%0,%c)\n" mincpu6(22+40)
stmt: ASGNP2(ac,iarg)  "\t%0%[1b]DOKEA(%1);\n" mincpu6(30)
stmt: ASGNI2(ac,iarg)  "\t%0%[1b]DOKEA(%1);\n" mincpu6(30)
stmt: ASGNU2(ac,iarg)  "\t%0%[1b]DOKEA(%1);\n" mincpu6(30)
stmt: ASGNI1(ac,iarg)  "\t%0%[1b]POKEA(%1);\n" mincpu6(28)
stmt: ASGNU1(ac,iarg)  "\t%0%[1b]POKEA(%1);\n" mincpu6(28)
stmt: ASGNP2(ac,con)   "\t%0%[1b]DOKEI(%1);\n" mincpu6(29)
stmt: ASGNI2(ac,con)   "\t%0%[1b]DOKEI(%1);\n" mincpu6(29)
stmt: ASGNU2(ac,con)   "\t%0%[1b]DOKEI(%1);\n" mincpu6(29)
stmt: ASGNI1(ac,conB)  "\t%0%[1b]POKEI(%1);\n" mincpu6(27)
stmt: ASGNI1(ac,conBs) "\t%0%[1b]POKEI(%1);\n" mincpu6(27)
stmt: ASGNU1(ac,conB)  "\t%0%[1b]POKEI(%1);\n" mincpu6(28)
stmt: ASGNP2(addr,ac)  "\t%1STW2(%0);\n" mincpu6(22+28)
stmt: ASGNI2(addr,ac)  "\t%1STW2(%0);\n" mincpu6(22+28)
stmt: ASGNU2(addr,ac)  "\t%1STW2(%0);\n" mincpu6(22+28)
stmt: ASGNI1(addr,ac)  "\t%1STB2(%0);\n" mincpu6(22+26)
stmt: ASGNU1(addr,ac)  "\t%1STB2(%0);\n" mincpu6(22+26)
reg: INDIRI2(ac) "\t%{#canVAC}%0%{dst!=vAC:DEEKA(%c)}%{dst==vAC:DEEK()};\n" mincpu6(30)
reg: INDIRU2(ac) "\t%{#canVAC}%0%{dst!=vAC:DEEKA(%c)}%{dst==vAC:DEEK()};\n" mincpu6(30)
reg: INDIRP2(ac) "\t%{#canVAC}%0%{dst!=vAC:DEEKA(%c)}%{dst==vAC:DEEK()};\n" mincpu6(30)
reg: INDIRI1(ac) "\t%{#canVAC}%0%{dst!=vAC:PEEKA(%c)}%{dst==vAC:PEEK()};\n" mincpu6(28)
reg: INDIRU1(ac) "\t%{#canVAC}%0%{dst!=vAC:PEEKA(%c)}%{dst==vAC:PEEK()};\n" mincpu6(28)
ac: INDIRI2(reg) "%{src!=vAC:DEEKV(%0)}%{src==vAC:DEEK()};" mincpu6(30)
ac: INDIRU2(reg) "%{src!=vAC:DEEKV(%0)}%{src==vAC:DEEK()};" mincpu6(30)
ac: INDIRP2(reg) "%{src!=vAC:DEEKV(%0)}%{src==vAC:DEEK()};" mincpu6(30)
ac: INDIRI1(reg) "%{src!=vAC:PEEKV(%0)}%{src==vAC:PEEK()};" mincpu6(30)
ac: INDIRU1(reg) "%{src!=vAC:PEEKV(%0)}%{src==vAC:PEEK()};" mincpu6(30)
ac: ADDI2(ac,con) "%0ADDWI(%1);" mincpu6(22+27)
ac: ADDU2(ac,con) "%0ADDWI(%1);" mincpu6(22+27)
ac: ADDP2(ac,con) "%0ADDWI(%1);" mincpu6(22+27)
ac: SUBI2(ac,con) "%0SUBWI(%1);" mincpu6(22+27)
ac: SUBU2(ac,con) "%0SUBWI(%1);" mincpu6(22+27)
ac: SUBP2(ac,con) "%0SUBWI(%1);" mincpu6(22+27)
ac: BCOMI2(ac)    "%0NOTW(vAC);" mincpu6(48)
ac: BCOMU2(ac)    "%0NOTW(vAC);" mincpu6(48)
ac: BANDI2(ac,con) "%0ANDWI(%1);" mincpu6(22+25)
ac: BANDU2(ac,con) "%0ANDWI(%1);" mincpu6(22+25)
ac: BORI2(ac,con)  "%0ORWI(%1);" mincpu6(22+21)
ac: BORU2(ac,con)  "%0ORWI(%1);" mincpu6(22+21)
ac: BXORI2(ac,con) "%0XORWI(%1);" mincpu6(22+21)
ac: BXORU2(ac,con) "%0XORWI(%1);" mincpu6(22+21)
ac: BANDI2(reg,conB) "ANDBK(%0,%1);" mincpu6(29)
ac: BANDU2(reg,conB) "ANDBK(%0,%1);" mincpu6(29)
ac: BORI2(CVUI2(reg),conB) "ORBK(%0,%1);" mincpu6(29)
ac: BORU2(CVUI2(reg),conB) "ORBK(%0,%1);" mincpu6(29)
ac: BXORI2(CVUI2(reg),conB) "XORBK(%0,%1);" mincpu6(29)
ac: BXORU2(CVUI2(reg),conB) "XORBK(%0,%1);" mincpu6(29)
ac: NEGI2(ac)  "%0NEGW(vAC);" mincpu6(48)
ac: ADDI2(ac,CVUI2(iarg)) "%0ADDBA(%1);" mincpu6(28)
ac: ADDU2(ac,CVUI2(iarg)) "%0ADDBA(%1);" mincpu6(28)
ac: SUBU2(ac,CVUI2(iarg)) "%0SUBBA(%1);" mincpu6(28)
ac: SUBI2(ac,CVUI2(iarg)) "%0SUBBA(%1);" mincpu6(28)

# Read-modify-write
rmw: VREGP "%a"
rmw: zddr "%0"
stmt: ASGNU1(rmw, LOADU1(ADDI2(CVUI2(INDIRU1(rmw)), con1))) "\t%{#keepsAC}INC(%0);\n" if_rmw(a,16)
stmt: ASGNI1(rmw, LOADI1(ADDI2(CVII2(INDIRI1(rmw)), con1))) "\t%{#keepsAC}INC(%0);\n" if_rmw(a,16)
stmt: ASGNU1(rmw, LOADU1(SUBI2(CVUI2(INDIRU1(rmw)), con1))) "\t%{#keepsAC}DEC(%0);\n" mincpu6(if_rmw(a,16))
stmt: ASGNI1(rmw, LOADI1(SUBI2(CVII2(INDIRI1(rmw)), con1))) "\t%{#keepsAC}DEC(%0);\n" mincpu6(if_rmw(a,16))

stmt: ASGNP2(rmw, ADDP2(INDIRP2(rmw), con1)) "\t%{#keepsAC}INCW(%0);\n" mincpu6(if_rmw(a, 26))
stmt: ASGNU2(rmw, ADDU2(INDIRU2(rmw), con1)) "\t%{#keepsAC}INCW(%0);\n" mincpu6(if_rmw(a, 26))
stmt: ASGNI2(rmw, ADDI2(INDIRI2(rmw), con1)) "\t%{#keepsAC}INCW(%0);\n" mincpu6(if_rmw(a, 26))
stmt: ASGNP2(rmw, SUBP2(INDIRP2(rmw), con1)) "\t%{#keepsAC}DECW(%0);\n" mincpu6(if_rmw(a, 26))
stmt: ASGNU2(rmw, SUBU2(INDIRU2(rmw), con1)) "\t%{#keepsAC}DECW(%0);\n" mincpu6(if_rmw(a, 26))
stmt: ASGNI2(rmw, SUBI2(INDIRI2(rmw), con1)) "\t%{#keepsAC}DECW(%0);\n" mincpu6(if_rmw(a, 26))
stmt: ASGNP2(rmw, ADDP2(INDIRP2(rmw), con1n)) "\t%{#keepsAC}DECW(%0);\n" mincpu6(if_rmw(a, 26))
stmt: ASGNU2(rmw, ADDU2(INDIRU2(rmw), con1n)) "\t%{#keepsAC}DECW(%0);\n" mincpu6(if_rmw(a, 26))
stmt: ASGNI2(rmw, ADDI2(INDIRI2(rmw), con1n)) "\t%{#keepsAC}DECW(%0);\n" mincpu6(if_rmw(a, 26))
stmt: ASGNI2(rmw, NEGI2(INDIRI2(rmw))) "\t%{#keepsAC}NEGW(%0);\n" mincpu6(if_rmw(a, 48))
stmt: ASGNI2(rmw, BCOMI2(INDIRI2(rmw))) "\%{#keepsAC}tNOTW(%0);\n" mincpu6(if_rmw(a, 48))
stmt: ASGNU2(rmw, BCOMU2(INDIRU2(rmw))) "\t%{#keepsAC}NOTW(%0);\n" mincpu6(if_rmw(a, 48))
stmt: ASGNI2(rmw, LSHI2(INDIRI2(rmw),con1)) "\t%{#keepsAC}LSLV(%0);\n" mincpu6(if_rmw(a, 56))
stmt: ASGNU2(rmw, LSHU2(INDIRU2(rmw),con1)) "\t%{#keepsAC}LSLV(%0);\n" mincpu6(if_rmw(a, 56))
stmt: ASGNU2(rmw, RSHU2(INDIRU2(rmw),con1)) "\t%{#keepsAC}LSRV(%0);\n" mincpu6(if_rmw(a, 56))

stmt: ASGNP2(rmw, ADDP2(INDIRP2(rmw), conB)) "\tADDVI(%2,%0);\n" mincpu6(if_rmw(a, 50))
stmt: ASGNU2(rmw, ADDU2(INDIRU2(rmw), conB)) "\tADDVI(%2,%0);\n" mincpu6(if_rmw(a, 50))
stmt: ASGNI2(rmw, ADDI2(INDIRI2(rmw), conB)) "\tADDVI(%2,%0);\n" mincpu6(if_rmw(a, 50))
stmt: ASGNP2(rmw, ADDP2(INDIRP2(rmw), conBn)) "\tSUBVI(-(%2),%0);\n" mincpu6(if_rmw(a, 50))
stmt: ASGNU2(rmw, ADDU2(INDIRU2(rmw), conBn)) "\tSUBVI(-(%2),%0);\n" mincpu6(if_rmw(a, 50))
stmt: ASGNI2(rmw, ADDI2(INDIRI2(rmw), conBn)) "\tSUBVI(-(%2),%0);\n" mincpu6(if_rmw(a, 50))
stmt: ASGNP2(rmw, SUBP2(INDIRP2(rmw), conB)) "\tSUBVI(%2,%0);\n" mincpu6(if_rmw(a, 50))
stmt: ASGNU2(rmw, SUBU2(INDIRU2(rmw), conB)) "\tSUBVI(%2,%0);\n" mincpu6(if_rmw(a, 50))
stmt: ASGNI2(rmw, SUBI2(INDIRI2(rmw), conB)) "\tSUBVI(%2,%0);\n" mincpu6(if_rmw(a, 50))
stmt: ASGNP2(rmw, ADDP2(INDIRP2(rmw), iarg)) "\t%[2b]ADDVW(%2,%0);\n" mincpu6(if_rmw(a, 52))
stmt: ASGNU2(rmw, ADDU2(INDIRU2(rmw), iarg)) "\t%[2b]ADDVW(%2,%0);\n" mincpu6(if_rmw(a, 52))
stmt: ASGNI2(rmw, ADDI2(INDIRI2(rmw), iarg)) "\t%[2b]ADDVW(%2,%0);\n" mincpu6(if_rmw(a, 52))
stmt: ASGNP2(rmw, SUBP2(INDIRP2(rmw), iarg)) "\t%[2b]SUBVW(%2,%0);\n" mincpu6(if_rmw(a, 52))
stmt: ASGNU2(rmw, SUBU2(INDIRU2(rmw), iarg)) "\t%[2b]SUBVW(%2,%0);\n" mincpu6(if_rmw(a, 52))
stmt: ASGNI2(rmw, SUBI2(INDIRI2(rmw), iarg)) "\t%[2b]SUBVW(%2,%0);\n" mincpu6(if_rmw(a, 52))

stmt: ASGNI2(rmw, conB) "\t%{#keepsAC}MOVQW(%1,%0)\n" mincpu6(if_not_asgn_tmp(a,29))
stmt: ASGNU2(rmw, conB) "\t%{#keepsAC}MOVQW(%1,%0)\n" mincpu6(if_not_asgn_tmp(a,29))
stmt: ASGNP2(rmw, conB) "\t%{#keepsAC}MOVQW(%1,%0)\n" mincpu6(if_not_asgn_tmp(a,29))
stmt: ASGNI1(rmw, conB) "\t%{#keepsAC}MOVQB(%1,%0)\n" mincpu6(if_not_asgn_tmp(a,27))
stmt: ASGNU1(rmw, conB) "\t%{#keepsAC}MOVQB(%1,%0)\n" mincpu6(if_not_asgn_tmp(a,27))

reg: LOADI1(conBs)  "\t%{#keepsAC}MOVQB(%0,%c)\n" mincpu6(27)
reg: LOADU1(conB)   "\t%{#keepsAC}MOVQB(%0,%c)\n" mincpu6(27)
reg: LOADI2(conB)   "\t%{#keepsAC}MOVQW(%0,%c)\n" mincpu6(29)
reg: LOADU2(conB)   "\t%{#keepsAC}MOVQW(%0,%c)\n" mincpu6(29)
reg: LOADP2(conB)   "\t%{#keepsAC}MOVQW(%0,%c)\n" mincpu6(29)



# /*-- END RULES --/
%%
/*---- BEGIN CODE --*/


static const char *segname() {
  if (cseg == CODE) return "CODE";
  if (cseg == DATA) return "DATA";
  if (cseg == LIT)  return "DATA";
  if (cseg == BSS)  return "BSS";
  return "?";
}

static void lprint(const char *fmt, ...) {
  char buf[1024];
  SList n;
  va_list ap;
  va_start(ap, fmt);
  vfprint(NULL, buf, fmt, ap);
  va_end(ap);
  n = allocate(sizeof(struct slist)+strlen(buf), PERM);
  strcpy(n->s, buf);
  n->next = &lhead;
  n->prev = lhead.prev;
  n->next->prev = n;
  n->prev->next = n;
}

/* Sometimes gen outputs data in the middle of a function.
   We don't want that here. */
static void xprint_init(void)
{
  in_function = 1;
  xhead.prev = xhead.next = &xhead;
}
static void xprint(const char *fmt, ...)
{
  char buf[1024];
  SList n;
  va_list ap;
  va_start(ap, fmt);
  vfprint(NULL, buf, fmt, ap);
  va_end(ap);
  if (in_function) {
    n = allocate(sizeof(struct slist)+strlen(buf), FUNC);
    strcpy(n->s, buf);
    n->next = &xhead;
    n->prev = xhead.prev;
    n->next->prev = n;
    n->prev->next = n;
  } else {
    print("%s", buf);
  }
}
static void xprint_finish(void)
{
  SList n;
  for (n = xhead.next; n != &xhead; n = n->next)
    print("%s", n->s);
  xhead.prev = xhead.next = &xhead;
  in_function = 0;
}

static int bitcount(unsigned mask) {
  unsigned i, n = 0;
  for (i = 1; i; i <<= 1)
    if (mask&i)
      n++;
  return n;
}

static int if_arg_reg_only(Node p)
{
  return p->syms[2] ? LBURG_MAX : 1;
}

static int if_arg_stk(Node p)
{
  return p->syms[2] ? 1 : LBURG_MAX;
}

static int if_zpconst(Node p)
{
  Symbol s = p->syms[0];
  assert(specific(p->op) == CNST+P);
  if ((size_t)s->u.c.v.p == ((size_t)s->u.c.v.p & 0xff))
    return 0;
  return LBURG_MAX;
}

static int if_zpglobal(Node p)
{
  /* TODO */
  return LBURG_MAX;
}

static int sametree(Node p, Node q) {
  return p == NULL && q == NULL
    || p && q && p->op == q->op && p->syms[0] == q->syms[0]
    && sametree(p->kids[0], q->kids[0])
    && sametree(p->kids[1], q->kids[1]);
}

static int if_rmw(Node a, int cost)
{
  Node r;
  assert(a);
  assert(generic(a->op) == ASGN);
  assert(a->kids[0]);
  assert(a->kids[1]);
  r = a->kids[1];
  while (generic(r->op) != INDIR)
    {
      if (r->kids[0])
        r = r->kids[0];
      else
        return LBURG_MAX;
    }
  if (sametree(a->kids[0], r->kids[0]))
    return cost;
  if (r->syms[RX]->temporary && r->syms[RX]->generated && r->syms[RX]->u.t.cse)
    {
      r = r->syms[RX]->u.t.cse;
      if (generic(r->op) == LOAD && r->kids[0])
        r = r->kids[0];
      if (generic(r->op) == INDIR && sametree(a->kids[0], r->kids[0]))
        return cost;
    }
  return LBURG_MAX;
}

static int if_not_asgn_tmp(Node p, int cost)
{
  Node n;
  assert(p);
  assert(generic(p->op) == ASGN);
  assert(p->kids[0]);
  n = p->kids[0];
  if (specific(n->op) == VREG+P && n->syms[0]->temporary)
    return LBURG_MAX;
  return cost;
}

static int if_cv_from(Node p, int sz, int cost)
{
  assert(p->syms[0]);
  assert(p->syms[0]->scope == CONSTANTS);
  assert(p->syms[0]->type = inttype);
  if (p->syms[0]->u.c.v.i == sz)
    return cost;
  return LBURG_MAX;
}

static void progend(void)
{
  extern char *firstfile; /* From input.c */
  SList s;
  print("# ======== (epilog)\n");
  print("code=[\n");
  for (s = lhead.next; s != &lhead; s = s->next)
    print("\t%s%s", s->s, (s->next == &lhead) ? "" : ",\n");
  print(" ]\n");
  print("module(code=code, ");
  if (firstfile)
    print("name='%s', ", firstfile);
  print("cpu=%d);\n", cpu);
  print("\n# Local Variables:"
        "\n# mode: python"
        "\n# indent-tabs-mode: t"
        "\n# End:\n");
}

static void progbeg(int argc, char *argv[])
{
  int i;
  /* Parse flags */
  parseflags(argc, argv);
  for (i=0; i<argc; i++)
    if (!strcmp(argv[i],"-cpu=4"))
      cpu = 4;
    else if (!strcmp(argv[i],"-cpu=5"))
      cpu = 5; /* Has CALLI,CMPHI,CMPHS. */
    else if (!strcmp(argv[i],"-cpu=6"))
      cpu = 6; /* TBD */
    else if (!strncmp(argv[i],"-cpu=",5))
      warning("invalid cpu %s\n", argv[i]+5);
  /* Print header */
  print("#VCPUv%d\n\n",cpu);
  /* Prepare registers */
  ireg[23] = mkreg("SP", 23, 1, IREG);
  for (i=0; i<23; i++)
    ireg[i] = mkreg("R%d", i, 1, IREG);
  for (i=0; i+1<23; i++)
    lreg[i] = mkreg("L%d", i, 3, IREG);
  for (i=0; i+2<23; i++)
    freg[i] = mkreg("F%d", i, 7, IREG);
  /* vAC/LAC/FAC */
  ireg[31] = mkreg("vAC", 24, 1, IREG);
  lreg[31] = mkreg("LAC", 26, 3, IREG);
  freg[31] = mkreg("FAC", 25, 7, IREG);
  /* Prepare wildcards */
  iregw = mkwildcard(ireg);
  lregw = mkwildcard(lreg);
  fregw = mkwildcard(freg);
  tmask[IREG] = REGMASK_TEMPS; /* nonleaf */
  vmask[IREG] = REGMASK_SAVED; /* nonleaf */
  tmask[FREG] = vmask[FREG] = 0;
  /* No segment */
  cseg = -1;
}

static Symbol rmap(int opk)
{
  switch(optype(opk)) {
  case I: case U:
    return (opsize(opk)==4) ? lregw : iregw;
  case P: case B:
    return iregw;
  case F:
    return fregw;
  default:
    return 0;
  }
}
  
static Symbol argreg(int argno, int ty, int sz, int *roffset)
{
  Symbol r = 0;
  if (argno == 0)
    *roffset = 8; /* First register is R8 */
  if (*roffset >= 16)
    return 0;
  if (ty == I || ty == U || ty == P)
    if (sz <= 2)
      r = ireg[*roffset];
    else
      r = lreg[*roffset];
  else if (ty == F)
    r = freg[*roffset];
  if (r == 0 || r->x.regnode->mask & ~REGMASK_ARGS)
    return 0;
  *roffset += roundup(sz,2)/2;
  return r;
}

static void target(Node p)
{
  assert(p);
  switch (specific(p->op))
    {
    case ARG+F: case ARG+I: case ARG+P: case ARG+U:
      if (p->syms[1])
        rtarget(p, 0, p->syms[1]);
      break;
    }
}

static int inst_contains_call(Node p)
{
  if ((generic(p->op) == CALL) ||
      (p->kids[0] && !p->kids[0]->x.inst && inst_contains_call(p->kids[0])) ||
      (p->kids[1] && !p->kids[1]->x.inst && inst_contains_call(p->kids[1])) )
    return 1;
  return 0;
}

static void clobber(Node p)
{
  static unsigned argmask = 0;
  assert(p);
  if (generic(p->op) == ARG) {
    /* Mark argument register as used so that it is not allocated as a
       temporary while preparing the remaining args. */
    Symbol r = p->syms[1];
    if (p->x.argno == 0)
      argmask = 0;
    if (r && r->x.regnode) {
      assert(r->x.regnode->set == 0);
      argmask |= r->x.regnode->mask;
      freemask[0] &= ~r->x.regnode->mask;
    }
  }
  if (inst_contains_call(p)) {
    /* Clobber all caller-saved registers before a call. */
    unsigned mask =  REGMASK_TEMPS & ~REGMASK_SAVED;
    if (p->x.registered && p->syms[2] && p->syms[2]->x.regnode->set == IREG)
      mask &= ~p->syms[2]->x.regnode->mask;
    if (mask)
      spill(mask, IREG, p);
  }
  if (argmask && p->x.next && inst_contains_call(p->x.next)) {
    /* Free all argument registers before the call */
    freemask[0] |= argmask;
    argmask = 0;
  }
}


/* Helper for preralloc */
static const char *skip_comment_in_template(const char *tpl)
{
  const char *s;
  for (s = tpl; s && *s; s++) {
    if (! strncmp(s, "%{#", 3)) {
      if (! (s = strchr(s, '}')))
        return tpl;
    } else if (!isspace(*s))
      break;
  }
  return s;
}

/* Helper for preralloc */
static int find_reguse(Node p, int nt, Symbol sym,
                       const char **lefttpl, int *leftkid, Node *leftkn,
                       const char **tpl)
{
  Node n = p;
  int rulenum = (*IR->x._rule)(n->x.state, nt);
  const short *nts = IR->x._nts[rulenum];
  const char *template = IR->x._templates[rulenum];
  const char *s;
  Node kids[10];
  int leftidx = -1;
  int count = 0;
  int i;

  if (tpl)
    *tpl = template;
  s = skip_comment_in_template(template);
  if (s[0] == '%' && isdigit(s[1]))
    leftidx = s[1] - '0';
  (*IR->x._kids)(n, rulenum, kids);
  for (i=0; nts[i] && i<NELEMS(kids); i++)
    {
      Node k = kids[i];
      int knt = nts[i];
      if (knt == k->x.inst) {
        if (k->syms[RX] == sym) {
          count += 1;
          if (lefttpl && leftidx < 0) {
            *lefttpl = template;
            *leftkid = i;
            *leftkn = k;
          }
        }
      } else if (i == leftidx) {
        count += find_reguse(k, knt, sym, lefttpl, leftkid, leftkn, 0);
      } else {
        count += find_reguse(k, knt, sym, 0, 0, 0, 0);
      }
    }
  return count;
}

/* Helper for preralloc */
static void change_sym_to_ac(Node p, Symbol sym, Symbol ac)
{
  assert(p->syms[RX] == sym);
  p->syms[RX] = ac;
  p->x.registered = 1;
  if (sym->temporary) {
    Node *q = &sym->x.lastuse;
    while (*q) {
      if (*q == p)
        *q = (*q)->x.prevuse;
      else
        q = &(*q)->x.prevuse;
    }
  }
  p->x.prevuse = 0;
}

/* Helper for preralloc */
static int scan_ac_preserving_instructions(Symbol sym, Symbol r, Node q, Symbol pr)
{
  int count, usecount;
  const char *lefttpl, *tpl;
  int leftkid;
  Node p;
  /* count temporary variable uses */
  usecount = -1;
  for (p = sym->x.lastuse; p; p = p->x.prevuse)
    usecount += 1;
  /* scan instructions until finding the last use of sym */
  for(; ; q = q->x.next)
    {
      /* Acceptable instructions here fall in two categories:
         instructions that leave vAC/LAC/FAC unchanged, and
         instructions that load sym into vAC/LAC/FAC. Since
         we do not have the infrastructure to do this generally,
         we only recognize a couple common cases. */
      if (! q)
        return 0;
      /* Instructions that generate no code */
      if (generic(q->op)==INDIR && specific(q->kids[0]->op) == VREG+P)
        /* matches INDIR(VREGP) rule which does not generate code */
        continue;
      if (generic(q->op)==ASGN && specific(q->kids[0]->op) == VREG+P
          && q->kids[1]->x.nt == _reg_NT)
        /* matches ASGN(VREGP,reg) which does not generate code */
        continue;
      /* For instructions that generate code, we start
         by counting how many times the instruction uses sym
         and obtaining its template */
      lefttpl = tpl = 0;
      count = find_reguse(q, q->x.inst, sym, &lefttpl, &leftkid, &p, &tpl);
      usecount -= count;
      if (tpl && count == 0 && !strncmp(tpl, "\t%{#keepsAC}", 12) )
        /* instruction is known to preseve vAC/LAC/FAC and does not use sym */
        continue;
      if (generic(q->op)==LOAD && q->kids[0]->x.inst == _reg_NT
          && generic(q->kids[0]->op) == INDIR
          && specific(q->kids[0]->kids[0]->op) == VREG+P
          && q->kids[0]->kids[0]->syms[0] == sym)
        /* matches a move instruction whose source is sym */
        {
          if (pr && lefttpl && count == 1 && p)
            change_sym_to_ac(p, sym, pr);
          if (usecount == 0)
            /* signal that we've reached the last use of sym */
            return 1;
          continue;
        }
      /* Next instruction is not known to preserve ac.
         But its first opcode might be the last use of sym. */
      if (lefttpl) {
        char okay = 0;
        char buf0[16];
        char buf1[12];
        char buf2[4];
        sprintf(buf0,"%%{src!=%s:", r->x.name);
        sprintf(buf1, "%%{#mov%d}", leftkid);
        sprintf(buf2, "%%%d", leftkid);
        if (strstr(lefttpl, buf1))
          for(; *lefttpl; lefttpl++)
            if (*lefttpl == ';' && !strstr(lefttpl, buf2)) {
              /* First opcode is a _MOV[LF] instruction
                 and no reference to %leftkid follows. */
              if (count == 1 && usecount == 0)
                /* signal we've reached the last use of sym */
                return 1;
              break;
            }
        lefttpl = skip_comment_in_template(lefttpl);
        if (leftkid == 0 && ! strncmp(lefttpl, buf0, strlen(buf0)))
          for(; *lefttpl && *lefttpl != '}'; lefttpl++)
            if (*lefttpl == ';' && !strstr(lefttpl, buf2)) {
              /* First opcode can be elided
                 and no reference to %leftkid follows. */
              if (pr)
                change_sym_to_ac(p, sym, pr);
              if (count == 1 && usecount == 0)
                /* signal we've reached the last use of sym */
                return 1;
              break;
            }
      }
      return 0;
    }
}

static void preralloc(Node p)
{
  Symbol sym = p->syms[RX];
  int rulenum = (*IR->x._rule)(p->x.state, p->x.inst);
  short *nts = IR->x._nts[rulenum];
  const char *template = IR->x._templates[rulenum];
  Symbol r = 0;
  Node q = p->x.next;
  
  /* Try to eliminate useless data moving operations between
     successive trees in the same forest, by using vAC/LAC/FAC instead
     of allocating a temporary register. */
  Symbol patch = 0;
  if (strstr(template,"%{#alsoVAC}"))
    patch = r = ireg[31];
  else if (strstr(template,"%{#alsoLAC}"))
    patch = r = lreg[31];
  else if (strstr(template,"%{#alsoFAC}"))
    patch = r = freg[31];
  else if (sym->temporary && strstr(template,"%{#canVAC}"))
    r = ireg[31];
  if (r && q && generic(q->op) == ASGN && q->kids[0]->op == VREG+P)
    {
      /* in order to use vAC/LAC/FAC instead of a temporary register,
         we must be sure that the value of vAC/LAC/FAC is unchanged
         whenever the code needs it.*/
      if (scan_ac_preserving_instructions(sym, r, q->x.next, patch)
          && sym->temporary)
        {
          /* optimize the temporary out of existence because 
             we have reached its last use. */
          r->x.lastuse = sym->x.lastuse;
          for (q = sym->x.lastuse; q; q = q->x.prevuse) {
            q->syms[RX] = r;
            q->x.registered = 1;
          }
        }
    }
}

static void myemitfmt(const char *fmt, Node p, Node *kids, short *nts)
{
  /* Enhancements of emitfmt with respect to the original version:

     - delegates %{...} delegates to the IR function emit3(),
       which can call emitasm/emitfmt() recursively.

     - Templates might be split in sections with |. Writing $0 to $9
       only prints the first section of the specified kid template.
       The other sections can be accessed with syntax $[0b] where '0'
       is the kid number and 'b' is a letter indicating 
       which section to process. 
  */
  static int alt_s;
  int s = alt_s;
  alt_s = 0;
  for (; *fmt; fmt++)
    if (*fmt == '|' && s == 0)
      break;
    else if (*fmt == '|')
      s -= 1;
    else if (s > 0)
      continue;
    else if (*fmt != '%')
      (void)putchar(*fmt);
    else if (*++fmt == 'F')                                   /* %F */
      print("%d", framesize);
    else if (*fmt >= 'a' && *fmt < 'a' + NELEMS(p->syms))     /* %a..%c */
      fputs(p->syms[*fmt - 'a']->x.name, stdout);
    else if (*fmt >= '0' && *fmt <= '9')                      /* %0..%9 */
      emitasm(kids[*fmt - '0'], nts[*fmt - '0']);
    else if (*fmt == '[' && fmt[3] == ']' && fmt[1]>='0' && fmt[1]<='9'
             && fmt[2] >= 'a' && fmt[2] <= 'z') {      /* %[0a] */
      fmt += 3;
      alt_s = fmt[-1] - 'a';
      emitasm(kids[fmt[-2] - '0'], nts[fmt[-2] - '0']);
      alt_s = 0;
    } else if (*fmt == '{') {
      int level = 0;
      const char *s;
      for (s=fmt++; *s; s++)
        if (*s=='{')
          level += 1;
        else if (*s=='}' && !--level)
          break;
      assert(!level);
      emit3(stringn(fmt, s-fmt), p, kids, nts);
      fmt = s;
    } else
      (void)putchar(*fmt);
}

static void emit3(const char *fmt, Node p, Node *kids, short *nts)
{
  int i = 0;
  /* %{#COMMENT} -- comment [nothing emitted]  */
  if (fmt[0]=='#')
    {
      /* This is mostly used to help preralloc.
         %{#keepsAC} : instruction preserves vAC/LAC/FAC.
         %{#alsoVAC} : contents of destination reg is also left in vAC.
         %{#alsoLAC} : contents of destination reg is also left in LAC.
         %{#alsoFAC} : contents of destination reg is also left in FAC.
         %{#canVAC}  : destination reg could be vAC.
         %{#movN}    : _MOVx instruction with source register %N. */
      return;
    }
  while (fmt[i] && fmt[i++] != ':')
    { }
  /* %{dst!=XXX:YYY} */
  if (!strncmp(fmt,"dst!=",5) && fmt[i])
    {
      if (p->syms[RX]->x.name != stringn(fmt+5,i-6))
        myemitfmt(fmt+i, p, kids, nts);
      return;
    }
  /* %{src!=XXX:YYY} */
  else if (!strncmp(fmt,"src!=", 5) && fmt[i])
    {
      if (!kids[0] || !kids[0]->syms[RX] || kids[0]->syms[RX]->x.name != stringn(fmt+5,i-6))
        myemitfmt(fmt+i, p, kids, nts);
      return;
    }
  /* %{dst==XXX:YYY} */
  if (!strncmp(fmt,"dst==",5) && fmt[i])
    {
      if (p->syms[RX]->x.name == stringn(fmt+5,i-6))
        myemitfmt(fmt+i, p, kids, nts);
      return;
    }
  /* %{src==XXX:YYY} */
  else if (!strncmp(fmt,"src==", 5) && fmt[i])
    {
      if (kids[0] && kids[0]->syms[RX] && kids[0]->syms[RX]->x.name == stringn(fmt+5,i-6))
        myemitfmt(fmt+i, p, kids, nts);
      return;
    }
  /* %{mulC[:R]} -- multiplication by a small constant */
  else if (!strncmp(fmt,"mul", 3) && fmt[3] >= '0' && fmt[3] <= '9')
    {
      int c;
      Node k;
      const char *r = "T3";
      i = fmt[3] - '0';
      k = kids[i];
      assert(k);
      if (! (k->syms[0] && k->syms[0]->scope == CONSTANTS))
        if (generic(k->op) == INDIR && k->syms[2] && k->syms[2]->u.t.cse)
          k = k->syms[2]->u.t.cse;
      assert(k->syms[0] && k->syms[0]->scope == CONSTANTS);
      c = k->syms[0]->u.c.v.i;
      if (fmt[4]) {
        assert(fmt[4]=='%' && fmt[5]>='0' && fmt[5]<='9' && !fmt[6]);
        assert(kids[fmt[5]-'0'] && kids[fmt[5]-'0']->syms[RX]);
        r = kids[fmt[5]-'0']->syms[RX]->x.name;
      }
      if(c == 0) {
        print("LDI(0);");
      } else {
        int m = 0x80;
        int x = (c >= 0) ? c : -c;
        assert(x>=0 && x<256);
        while (m && !(m & x))
          m >>= 1;
        if (fmt[4] && c < 0)
          print("LDI(0);SUBW(%s);", r);
        else if (fmt[4])
          print("LDW(%s);", r);
        else if (c < 0)
          print("STW(T3);LDI(0);SUBW(T3);");
        else if (x & (m-1))
          print("STW(T3);");
        for (m >>= 1; m; m >>= 1) {
          print("LSLW();");
          if (m & x)
            print("%s(%s);", (c > 0) ? "ADDW" : "SUBW", r);
        }
      }
      return;
    }
  /* otherwise complain */
  assert(0);
}

static void doarg(Node p)
{
  /* Important change in arg passing:
     - When calling a function, all arguments beyond
       those specified in the prototype are written to the stack 
       in addition to being possibly passed in registers.
       In particular this happens for all arguments
       when calling a non prototyped function,
       and this happens for all supplementary arguments
       when the function has a variadic prototype.
     - When defining a function, prototyped or not,
       arguments are read from registers whenever possible.
     - The stdarg macros always read arguments from the stack.
  */
  static int argno;
  static int argmaxno;
  static int roffset;
  int offset;
  Symbol r;
  Node c;
  if (argoffset == 0) {
    argno = 0;
    argmaxno = 0;
    argoffset = 2;
    for (c=p; c; c=c->link)
      if (generic(c->op) == CALL ||
          (generic(c->op) == ASGN && generic(c->kids[1]->op) == CALL &&
           (c = c->kids[1]) ) )
        break;
    if (c && c->syms[0]->type->u.f.proto)
      while (c->syms[0]->type->u.f.proto[argmaxno] &&
             c->syms[0]->type->u.f.proto[argmaxno] != voidtype)
        argmaxno += 1;
  }
  r  = argreg(argno, optype(p->op), opsize(p->op), &roffset);
  offset = mkactual(p->syms[1]->u.c.v.i, p->syms[0]->u.c.v.i);
  p->x.argno = argno++;
  p->syms[2] = (r && p->x.argno < argmaxno) ? 0 : intconst(offset);
  if (optype(p->op) != B) /* keep alignment info for structures */
    p->syms[1] = r;
}

static void local(Symbol p)
{
  /* The size check restricts allocating registers for longs and
     floats local variables (but not temporaries). The register
     benefit for longs and floats is minimal. Best to heuristically
     keep enough of them for ints and pointers. */
  if (p->type->size > 2 && ! p->temporary)
    if (bitcount(vmask[IREG] & freemask[IREG]) < p->type->size)
      p->sclass = AUTO;
  if (askregvar(p, rmap(ttob(p->type))) == 0)
    mkauto(p);
}

static void printregmask(unsigned mask) {
  unsigned i, m;
  char *prefix = "R";
  for (i=0, m=1; i<31; i++, m<<=1)
    if (mask & m) {
      print("%s%d", prefix, i);
      prefix = ",";
      if (i<30 && (mask & (m+m))) {
        for (; i<30 && (mask & (m+m)); i++, m<<=1) {}
        print("-%d", i);
      }
    }
  if (!mask)
    print("None");
}

static void function(Symbol f, Symbol caller[], Symbol callee[], int ncalls)
{
  /* stack frame:
  |                    n bytes                   : arguments
  |    SP+Framesize -> 2 bytes                   : saved vLR
  |                    maxoffset bytes           : local variables
  |                    sizesave bytes            : saved registers
  |                    maxargoffset bytes        : argument building area
  |              SP -> 2 bytes                   : buffer where callees can save vLR
  **/

  int i, roffset, sizesave, ty;
  unsigned savemask;
  Symbol r;
  usedmask[0] = usedmask[1] = 0;
  freemask[0] = freemask[1] = ~(unsigned)0;
  offset = maxoffset = 0;
  maxargoffset = 2;
  assert(f->type && f->type->type);
  ty = ttob(f->type->type);
  if (ncalls) {
    tmask[IREG] = REGMASK_TEMPS;
    vmask[IREG] = REGMASK_SAVED;
  } else {
    tmask[IREG] = REGMASK_TEMPS;
    vmask[IREG] = REGMASK_MOREVARS;
  }
  /* locate incoming arguments */
  offset = 2;
  roffset = 0;
  for (i = 0; callee[i]; i++) {
    Symbol p = callee[i];
    Symbol q = caller[i];
    assert(q);
    offset = roundup(offset, q->type->align);
    p->x.offset = q->x.offset = offset;
    p->x.name = q->x.name = stringd(offset);
    r = argreg(i, optype(ttob(q->type)), q->type->size, &roffset);
    offset += q->type->size;
    if (r) {
      if (ncalls == 0 && !p->addressed && p->ref > 0) {
        /* Leaf function: leave register arguments in place */
        p->sclass = q->sclass = REGISTER;
        askregvar(p, r);
        assert(p->x.regnode && p->x.regnode->vbl == p);
        q->x = p->x;
        q->type = p->type;
      } else {
        /* Aggressively ask new registers for args passed in registers */
        if (!p->addressed && p->ref > 0)
          p->sclass = REGISTER;
        /* Let gencode know args were passed in register */
        q->sclass = REGISTER;
        q->x = r->x;
      }
    }
    if (p->sclass == REGISTER && !p->x.regnode && p->ref > 0) {
      /* Allocate register argument. Gencode does the rest. */
      askregvar(p, rmap(ttob(p->type)));
    }
  }
  /* gen code */
  assert(!caller[i]);
  offset = 0;
  gencode(caller, callee);
  /* compute framesize */
  savemask = usedmask[IREG] & REGMASK_SAVED;
  sizesave = 2 * bitcount(savemask);
  maxargoffset = (maxargoffset + 1) & ~0x1;
  maxoffset = (maxoffset + 1) & ~0x1;
  framesize = maxargoffset + sizesave + maxoffset;
  assert(framesize >= 2);
  /* can we make a frameless leaf function */
  if (ncalls == 0 && framesize == 2 && (tmask[IREG] & ~usedmask[IREG]))
    framesize = 0;
  if (IR->longmetric.align == 4)
    framesize = (framesize + 3) & ~0x3;
  /* prologue */
  xprint_init();
  segment(CODE);
  lprint("('%s', %s, code%d)", segname(), f->x.name, codenum);
  print("# ======== %s\n", lhead.prev->s);
  print("def code%d():\n", codenum++);
  print("\tlabel(%s);\n", f->x.name);
  if (framesize == 0) {
    print("\tPUSH();\n");
  } else {
    print("\t_PROLOGUE(%d,%d,0x%x); # save=", framesize, maxargoffset, savemask);
    printregmask(savemask);
    print("\n");
  }
  /* Emit actual code */
  emitcode();
  /* Epilogue */
  if (framesize == 0) {
    print("\ttryhop(2);POP();RET()\n");
  } else {
    const char *saveac = "";
    if (opsize(ty) <= 2 && (optype(ty) == I || optype(ty) == U || optype(ty) == P))
      saveac = ",saveAC=True";
    printf("\t_EPILOGUE(%d,%d,0x%x%s);\n", framesize, maxargoffset, savemask, saveac);
  }
  /* print delayed data */
  xprint_finish();
}

static void defconst(int suffix, int size, Value v)
{
  if (suffix == F) {
    double d = v.d;
    int exp;
    unsigned long mantissa;
    assert(size == 5);
    assert(isfinite(d));
    mantissa = (unsigned long)(frexp(fabs(d),&exp) * 4294967296.0 + 0.4999999995343387);
    if (exp < -127)
      mantissa = 0;
    if (mantissa == 0) 
      xprint("\tbytes(0,0,0,0,0);");
    else if (exp > 127)
      error("floating point constant overflow\n");
    else
      xprint("\tbytes(%d,%d,%d,%d,%d);",
             exp+128, ((mantissa>>24)&0x7f)|((d<0.0)?0x80:0x00),
             (mantissa>>16)&0xff, (mantissa>>8)&0xff, (mantissa&0xff) );
    xprint(" # %g\n", d);
  } else {
    unsigned long x = (suffix == P) ? (unsigned)(size_t)v.p : (suffix == I) ? v.i : v.u;
    if (size == 1) 
      print("\tbytes(%d);", x&0xff);
    else if (size == 2)
      print("\twords(%d);", x&0xffff);
    else if (size == 4)
      print("\twords(%d,%d);", x&0xffff, (x>>16)&0xffff);
    if (suffix == I)
      xprint(" # %D", (long)x);
    else if (suffix == U)
      xprint(" # %U", (unsigned long)x);
    xprint("\n");
  }
}

static void defaddress(Symbol p)
{
  xprint("\twords(%s);\n", p->x.name);
}

static void defstring(int n, char *str)
{
  int i;
  for (i=0; i<n; i++)
    xprint( ((i&7)==0) ? "\tbytes(%d" : ((i&7)==7) ? ",%d);\n" : ",%d", (int)str[i]&0xff );
  if (i&7)
    xprint(");\n");
}

static void import(Symbol p)
{
  if (p->ref > 0 && strncmp(p->x.name, "'__glink_weak_", 14) != 0)
    lprint("('IMPORT', %s)", p->x.name);
}

static void export(Symbol p)
{
  if (p->u.seg != BSS)
    lprint("('EXPORT', %s)", p->x.name);
}

static void defsymbol(Symbol p)
{
  if (p->scope >= LOCAL && p->sclass == STATIC)
    p->x.name = stringf("'.%d'", genlabel(1));
  else if (p->generated)
    p->x.name = stringf("'.%s'", p->name);
  else if (p->scope == GLOBAL || p->sclass == EXTERN)
    p->x.name = stringf("'%s'", p->name);
  else
    p->x.name = p->name;
}

static void address(Symbol q, Symbol p, long n)
{
  if (p->scope == GLOBAL || p->sclass == STATIC || p->sclass == EXTERN)
    q->x.name = stringf("v(%s)%s%D", p->x.name, n >= 0 ? "+" : "", n);
  else {
    assert(n <= INT_MAX && n >= INT_MIN);
    q->x.offset = p->x.offset + n;
    q->x.name = stringd(q->x.offset);
  }
}

static void global(Symbol p)
{
  unsigned int size = p->type->size;
  const char *s = segname();
  if (p->u.seg == BSS && p->sclass != STATIC)
    s = "COMMON";
  if (p->u.seg == LIT)
    size = 0; /* unreliable in switch tables */
  lprint("('%s', %s, code%d, %d, %d)",
          s, p->x.name, codenum, size, p->type->align);
  xprint("# ======== %s\n", lhead.prev->s);
  xprint("def code%d():\n", codenum++);
  if (p->type->align > 1)
    xprint("\talign(%d);\n", p->type->align);
  xprint("\tlabel(%s);\n", p->x.name);
  if (p->u.seg == BSS)
    xprint("\tspace(%d);\n", p->type->size);
}

static void segment(int n)
{
  cseg = n;
}

static void space(int n)
{
  if (cseg != BSS)
    xprint("\tspace(%d);\n", n);
}

Interface gigatronIR = {
        1, 1, 0,  /* char */
        2, 2, 0,  /* short */
        2, 2, 0,  /* int */
        4, 4, 1,  /* long */
        4, 4, 1,  /* long long */
        5, 1, 1,  /* float */
        5, 1, 1,  /* double */
        5, 1, 1,  /* long double */
        2, 2, 0,  /* pointer */
        0, 1, 0,  /* struct */
        1,        /* little_endian */
        0,        /* mulops_calls */
        0,        /* wants_callb */
        1,        /* wants_argb */
        1,        /* left_to_right */
        0,        /* wants_dag */
        1,        /* unsigned_char !!!! */
        1,        /* wants_cvfu_cvuf */
        address,
        blockbeg,
        blockend,
        defaddress,
        defconst,
        defstring,
        defsymbol,
        emit,
        export,
        function,
        gen,
        global,
        import,
        local,
        progbeg,
        progend,
        segment,
        space,
        /* stabblock, stabend, 0, stabinit, stabline, stabsym, stabtype */
        0, 0, 0, 0, 0, 0, 0,
        /* Xinterface */
        { 1,
          rmap,
          blkfetch, blkstore, blkloop,
          _label,
          _rule,
          _nts,
          _kids,
          _string,
          _templates,
          _isinstruction,
          _ntname,
          emit2,
          myemitfmt,
          doarg,
          target,
          clobber,
          preralloc,
        }
};

/*---- END CODE --*/

/* Local Variables: */
/* mode: c */
/* c-basic-offset: 2 */
/* indent-tabs-mode: () */
/* End: */
