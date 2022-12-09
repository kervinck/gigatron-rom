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
static void emit3(const char*, int, Node, int, Node*, short*);
static void emitfmt1(const char*, Node, int, Node*, short*);
static void export(Symbol);
static void clobber(Node);
static void preralloc(Node);
static Node pregen(Node);
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
static int  if_incr(Node,int,int);      /* cost hint */
static int  if_zoffset(Node,int,int);   /* cost hint */
static int  if_cnstreuse(Node,int,int); /* cost hint */
static int  if_rmw(Node,int);
static int  if_rmw_a(Node,int,int);
static int  if_rmw_incr(Node,int,int);
static int  if_not_asgn_tmp(Node,int);
static int  if_cv_from(Node,int,int);
static int  if_arg_reg_only(Node);
static int  if_arg_stk(Node,int);

#define mincpu5(cost) ((cpu<5)?LBURG_MAX:(cost))
#define mincpu6(cost) ((cpu<6)?LBURG_MAX:(cost))
#define mincpu7(cost) ((cpu<7)?LBURG_MAX:(cost))
#define ifcpu7(c1,c2) ((cpu<7)?(c2):(c1))
#define if_spill()    ((spilling)?0:LBURG_MAX)

/* Registers */
static Symbol ireg[32], lreg[32], freg[32];
static Symbol iregw, lregw, fregw;

#define REGMASK_SAVED           0x000000ff
#define REGMASK_ARGS            0x0000ff00
#define REGMASK_MOREVARS        0x000fffff
#define REGMASK_TEMPS           0x00ffff00

/* Misc */
static int codenum = 0;
static int cseg = 0;
static int cpu = 5;

/* Register equivalences for the emitter state machine */
static int vac_clobbered;
static int xac_clobbered;
static Symbol vac_constval;
static Symbol vac_memval, lac_memval, fac_memval;
static unsigned vac_equiv, lac_equiv, fac_equiv;


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
# computing a cover of the trees with instructions, we cover the trees
# with sequences of instructions that use the accumulator vAC and
# the scratch registers (T0..T3) as they see fit. The LBURG grammar
# is no longer a tree grammar, but a transducer that converts tree
# fragments into sequences. As a result, each nonterminal must be
# defined by two components: the role it occupies on the tree grammar
# and the role it occupies in the sequence grammar.

# The template syntax contains several augmentations.
# * Templates might be split in sections with |. Writing $0 to $9
#   only prints the first section of the specified kid template.
#   The other sections can be accessed with syntax $[0b] where '0'
#   is the kid number and 'b' is a letter indicating
#   which section to process.
# * Templates can contain complex constructs %{...} which are
#   processed by function emit3. For instance %{mul..} is used
#   to inline multiplications by small constants.
# * Conditional constructs %{?x==...:yyy:nnn} and %{?x=~...:yyy:nnn}
#   where x is 0..9 or a..c processes yyy if %x is equal to ...
#   and otherwise processes no. Comparison with == are always literal.
#   When the right hand side of a comparison with =~ is an accumulator,
#   the yes branch is also processed when a state machine in the emitter
#   knows the accumulator to be equal to register or constant %x.
# * Clobbering information is deduced from the nonterminal
#   identity.  For instance rules for nonterminal ac are assumed to
#   clobber vAC.  When this is the case, annotations ${=x} where x is
#   %0..9 %a--b, or an accumulator, can be used to indicate the value
#   taken by the clobbered register. This drives a state machine in
#   the emitter that conservatively knows when accumulator or registers
#   contain the same value. This informs conditional constructs.
# * Clobbering annotations %{!...} documents additional clobbered
#   registers beyond those implied by the nonterminal.
#   Inside the braces, letter A, L, and F indicate that vAC, LAC,
#   and FAC are affected. Letters 4 and 5 are like A but apply only
#   for cpu versions less than 4 or 5. Note that L implies F and A,
#   and F implies L and A.

#   Program lburg has also been enhanced in several ways. First it
#   accepts comments in the rule section (this is one) and keeps track
#   of line numbers. Second, it has a more refined cost computation
#   for mayrecalc. Third, it accepts nonconstant costs in nonterminal
#   closures provided that the cost string starts with a + (to mark
#   that we know what we are doing.)


# -- common rules
reg:  INDIRI1(VREGP)     "# read register\n"
reg:  INDIRU1(VREGP)     "# read register\n"
reg:  INDIRI2(VREGP)     "# read register\n"
reg:  INDIRU2(VREGP)     "# read register\n"
reg:  INDIRP2(VREGP)     "# read register\n"
reg:  INDIRI4(VREGP)     "# read register\n"
reg:  INDIRU4(VREGP)     "# read register\n"
reg:  INDIRF5(VREGP)     "# read register\n"
stmt: ASGNI1(VREGP,reg) "# write register\n"
stmt: ASGNU1(VREGP,reg) "# write register\n"
stmt: ASGNI2(VREGP,reg) "# write register\n"
stmt: ASGNU2(VREGP,reg) "# write register\n"
stmt: ASGNP2(VREGP,reg) "# write register\n"
stmt: ASGNI4(VREGP,reg) "# write register\n"
stmt: ASGNU4(VREGP,reg) "# write register\n"
stmt: ASGNF5(VREGP,reg) "# write register\n"

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
conB: CNSTI2  "%a"  range(a,0,255)
conB: CNSTU2  "%a"  range(a,0,255)
conB: CNSTP2  "%a"  if_zpconst(a)
conB: CNSTI1  "%a"
conB: CNSTU1  "%a"
conB: zddr    "%0"
conBn: CNSTI2 "%a"  range(a,-255,-1)
conBs: CNSTI2 "%a"  range(a,-128,+127)
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
# -- reg and regx are the boundary between instruction sequences and
#    the register allocator. When they appear on the left hand side of a rule,
#    they represents a completed sequence of instruction that stores
#    its result in a register to be allocated by ralloc.
#    When they appears on the right hand side of a rule, they represent the
#    register name containing the value of the expression. The only difference
#    is that the allocator (preralloc) cannot allocate an accumulator to a regx.
# -- ac represents a sequence of instruction that places the expression value
#    into register vAC, potentially clobbeting LAC, FAC, and the scratch registers.
# -- lac and fac are the same but respectively compute long results and fp results
#    into registers LAC or FAC, potentially clobbering vAC, LAC, FAC, and T0-T3.
# -- eac is like ac but cannot clobber LAC, FAC or T2.
# -- ac0/eac0 mean that the high byte of ac is known to be zero

stmt: reg  ""
stmt: asgn "%0"
stmt: ac   "\t%0\n"
regx: reg  "%{=%0}%0"
reg:  regx "%{=%0}%0"
reg:  ac   "\t%{=vAC}%0%{?c==vAC::STW(%c);}\n" 19

ac0:  eac0  "%{=%0}%0"
eac:  eac0  "%{=%0}%0"
ac:   ac0   "%{=%0}%0"
ac:   eac   "%{=%0}%0"
eac:  reg   "%{=%0}%{?0=~vAC::LDW(%0);}"  20
eac:  lddr  "%{=%0}%{?0=~vAC::_SP(%0);}"  41
eac0: conB  "%{=%0}%{?0=~vAC::LDI(%0);}"  +if_cnstreuse(a,16,8)
eac:  conBn "%{=%0}%{?0=~vAC::LDNI(%0);}" +mincpu6(if_cnstreuse(a,16,8))
eac:  con   "%{=%0}%{?0=~vAC::LDWI(%0);}" +if_cnstreuse(a,21,8)

# Loads
eac:  INDIRI2(eac)  "%{?*0=~vAC::%0DEEK();}" 21
eac:  INDIRU2(eac)  "%{?*0=~vAC::%0DEEK();}" 21
eac:  INDIRP2(eac)  "%{?*0=~vAC::%0DEEK();}" 21
eac0: INDIRI1(eac)  "%0PEEK();" 17
eac0: INDIRU1(eac)  "%0PEEK();" 17
eac:  INDIRI2(zddr) "%{?*0=~vAC::LDW(%0);}" 20
eac:  INDIRU2(zddr) "%{?*0=~vAC::LDW(%0);}" 20
eac:  INDIRP2(zddr) "%{?*0=~vAC::LDW(%0);}" 20
eac0: INDIRI1(zddr) "LD(%0);" 18
eac0: INDIRU1(zddr) "LD(%0);" 18
ac:   INDIRI2(ac) "%{?*0=~vAC::%0DEEK();}" 21
ac:   INDIRU2(ac) "%{?*0=~vAC::%0DEEK();}" 21
ac:   INDIRP2(ac) "%{?*0=~vAC::%0DEEK();}" 21
ac0:  INDIRI1(ac) "%0PEEK();" 17
ac0:  INDIRU1(ac) "%0PEEK();" 17

# -- iarg represents the argument of binary integer operations that
#    map to zero page locations in assembly instructions.  However the
#    spiller needs to be able to reload a register from an auto
#    variable without allocating a register. This is achieved by another
#    branch which defines two fragments using the alternate expansion
#    mechanism defined by emitfmt1. The two fragments are a register name (T0)
#    and an instruction sequence.
iarg: regx "%0"
iarg: INDIRI2(zddr) "%0"
iarg: INDIRU2(zddr) "%0"
iarg: INDIRP2(zddr) "%0"

spill: ADDRLP2 "%a+%F" if_spill()
iarg: INDIRU2(spill) "T0|STW(B0);_LDLW(%0);STW(T0);LDW(B0);" 20
iarg: INDIRI2(spill) "T0|STW(B0);_LDLW(%0);STW(T0);LDW(B0);" 20
iarg: INDIRP2(spill) "T0|STW(B0);_LDLW(%0);STW(T0);LDW(B0);" 20

# Integer operations. This is verbose because there are variants for
# types I2, U2, P2, variants for argument ordering, and variants for
# constant arguments.
ac: ADDI2(ac,iarg)  "%0%[1b]ADDW(%1);" 28
ac: ADDU2(ac,iarg)  "%0%[1b]ADDW(%1);" 28
ac: ADDP2(ac,iarg)  "%0%[1b]ADDW(%1);" 28
ac: ADDI2(iarg,ac)  "%1%[0b]ADDW(%0);" 28
ac: ADDU2(iarg,ac)  "%1%[0b]ADDW(%0);" 28
ac: ADDP2(iarg,ac)  "%1%[0b]ADDW(%0);" 28
ac: ADDI2(LSHI2(iarg,con1),ac) "%2%[0b]ADDW(%0);ADDW(%0);" 56
ac: ADDU2(LSHU2(iarg,con1),ac) "%2%[0b]ADDW(%0);ADDW(%0);" 56
ac: ADDP2(LSHI2(iarg,con1),ac) "%2%[0b]ADDW(%0);ADDW(%0);" 56
ac: ADDP2(LSHU2(iarg,con1),ac) "%2%[0b]ADDW(%0);ADDW(%0);" 56
ac: ADDI2(ac,conB)  "%0ADDI(%1);"      if_incr(a,27,10)
ac: ADDU2(ac,conB)  "%0ADDI(%1);"      if_incr(a,27,10)
ac: ADDP2(ac,conB)  "%0ADDI(%1);"      if_incr(a,27,10)
ac: ADDI2(ac,conBn) "%0SUBI(-(%1));"   if_incr(a,27,10)
ac: ADDU2(ac,conBn) "%0SUBI(-(%1));"   if_incr(a,27,10)
ac: ADDP2(ac,conBn) "%0SUBI(-(%1));"   if_incr(a,27,10)
ac: SUBI2(ac,iarg)  "%0%[1b]SUBW(%1);" 28
ac: SUBU2(ac,iarg)  "%0%[1b]SUBW(%1);" 28
ac: SUBP2(ac,iarg)  "%0%[1b]SUBW(%1);" 28
ac: SUBI2(ac,conB)  "%0SUBI(%1);"      27
ac: SUBU2(ac,conB)  "%0SUBI(%1);"      27
ac: SUBP2(ac,conB)  "%0SUBI(%1);"      27
ac: SUBI2(ac,conBn) "%0ADDI(-(%1));"   27
ac: SUBU2(ac,conBn) "%0ADDI(-(%1));"   27
ac: SUBP2(ac,conBn) "%0ADDI(-(%1));"   27
ac: NEGI2(ac)       "%0STW(T3);LDI(0);SUBW(T3);" 68
ac: NEGI2(regx)     "LDI(0);SUBW(%0);" 48
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
ac: MULI2(conB, ac)    "%1%{mul0}" 150
ac: MULI2(conBn, ac)   "%1%{mul0}" 160
ac: MULI2(conB, regx)  "%{mul0%1}" 150
ac: MULI2(conBn, regx) "%{mul0%1}" 160
ac: MULI2(con, ac)     "%1_MULI(%0);" 200
ac: MULI2(ac, iarg)    "%0%[1b]_MUL(%1);" 200
ac: MULI2(iarg, ac)    "%1%[0b]_MUL(%0);" 200
ac: MULU2(conB, ac)    "%1%{mul0}" 150
ac: MULU2(con, ac)     "%1_MULI(%0);" 200
ac: MULU2(ac, iarg)    "%0%[1b]_MUL(%1);" 200
ac: MULU2(iarg, ac)    "%1%[0b]_MUL(%0);" 200
ac: DIVI2(ac, iarg) "%0%[1b]_DIVS(%1);" 200
ac: DIVU2(ac, iarg) "%0%[1b]_DIVU(%1);" 200
ac: MODI2(ac, iarg) "%0%[1b]_MODS(%1);" 200
ac: MODU2(ac, iarg) "%0%[1b]_MODU(%1);" 200
ac: DIVI2(ac, con)  "%0%[1b]_DIVIS(%1);" 180
ac: DIVU2(ac, con)  "%0%[1b]_DIVIU(%1);" 180
ac: MODI2(ac, con)  "%0%[1b]_MODIS(%1);" 180
ac: MODU2(ac, con)  "%0%[1b]_MODIU(%1);" 180
ac: BCOMI2(ac)      "%0STW(T3);_LDI(-1);XORW(T3);" 68
ac: BCOMU2(ac)      "%0STW(T3);_LDI(-1);XORW(T3);" 68
ac: BANDI2(ac,iarg) "%0%[1b]ANDW(%1);" 28
ac: BANDU2(ac,iarg) "%0%[1b]ANDW(%1);" 28
ac: BANDI2(iarg,ac) "%1%[0b]ANDW(%0);" 28
ac: BANDU2(iarg,ac) "%1%[0b]ANDW(%0);" 28
ac: BANDI2(ac,conB) "%0ANDI(%1);" 22
ac: BANDU2(ac,conB) "%0ANDI(%1);" 22
ac: BORI2(ac,iarg)  "%0%[1b]ORW(%1);" 28
ac: BORU2(ac,iarg)  "%0%[1b]ORW(%1);" 28
ac: BORI2(iarg,ac)  "%1%[0b]ORW(%0);" 28
ac: BORU2(iarg,ac)  "%1%[0b]ORW(%0);" 28
ac: BORI2(ac,conB)  "%0ORI(%1);" 14
ac: BORU2(ac,conB)  "%0ORI(%1);" 14
ac: BXORI2(ac,iarg) "%0%[1b]XORW(%1);" 28
ac: BXORU2(ac,iarg) "%0%[1b]XORW(%1);" 28
ac: BXORI2(iarg,ac) "%1%[0b]XORW(%0);" 28
ac: BXORU2(iarg,ac) "%1%[0b]XORW(%0);" 28
ac: BXORI2(ac,conB) "%0XORI(%1);" 14
ac: BXORU2(ac,conB) "%0XORI(%1);" 14

# A couple EAC variants
eac: ADDI2(eac,conB)  "%0ADDI(%1);" 28
eac: ADDU2(eac,conB)  "%0ADDI(%1);" 28
eac: ADDP2(eac,conB)  "%0ADDI(%1);" 28
eac: ADDI2(eac,conBn) "%0SUBI(-(%1));" 28
eac: ADDU2(eac,conBn) "%0SUBI(-(%1));" 28
eac: ADDP2(eac,conBn) "%0SUBI(-(%1));" 28
eac: SUBI2(eac,conB)  "%0SUBI(%1);" 28
eac: SUBU2(eac,conB)  "%0SUBI(%1);" 28
eac: SUBP2(eac,conB)  "%0SUBI(%1);" 28
eac: SUBI2(eac,conBn) "%0ADDI(-(%1));" 28
eac: SUBU2(eac,conBn) "%0ADDI(-(%1));" 28
eac: SUBP2(eac,conBn) "%0ADDI(-(%1));" 28
eac: LSHI2(eac, con1) "%0LSLW();" 28
eac: LSHU2(eac, con1) "%0LSLW();" 28
eac: LSHI2(eac, conB) "%0_SHLI(%1);" 100
eac: LSHU2(eac, conB) "%0_SHLI(%1);" 100
eac: MULI2(conB, eac) "%1%{mul0}" 150
eac: MULI2(conB, eac) "%1%{mul0}" 150
# More eac variants involving iarg because iarg spills preserve T2
eac: ADDI2(eac,iarg) "%0%[1b]ADDW(%1);" 28
eac: ADDU2(eac,iarg) "%0%[1b]ADDW(%1);" 28
eac: ADDP2(eac,iarg) "%0%[1b]ADDW(%1);" 28
eac: ADDI2(iarg,eac) "%1%[0b]ADDW(%0);" 28
eac: ADDU2(iarg,eac) "%1%[0b]ADDW(%0);" 28
eac: ADDP2(iarg,eac) "%1%[0b]ADDW(%0);" 28
eac: ADDI2(LSHI2(iarg,con1),eac) "%2%[0b]ADDW(%0);ADDW(%0);" 56
eac: ADDU2(LSHU2(iarg,con1),eac) "%2%[0b]ADDW(%0);ADDW(%0);" 56
eac: ADDP2(LSHI2(iarg,con1),eac) "%2%[0b]ADDW(%0);ADDW(%0);" 56
eac: ADDP2(LSHU2(iarg,con1),eac) "%2%[0b]ADDW(%0);ADDW(%0);" 56
eac: SUBI2(eac,iarg) "%0%[1b]SUBW(%1);" 28
eac: SUBU2(eac,iarg) "%0%[1b]SUBW(%1);" 28
eac: SUBP2(eac,iarg) "%0%[1b]SUBW(%1);" 28

# More assignments (indirect and explicit addresses)
asgn: ASGNP2(zddr,ac)  "\t%{=vAC}%1STW(%0);\n"       20
asgn: ASGNP2(iarg,ac)  "\t%{=vAC}%1%[0b]DOKE(%0);\n" 28
asgn: ASGNI2(zddr,ac)  "\t%{=vAC}%1STW(%0);\n"       20
asgn: ASGNI2(iarg,ac)  "\t%{=vAC}%1%[0b]DOKE(%0);\n" 28
asgn: ASGNU2(zddr,ac)  "\t%{=vAC}%1STW(%0);\n"       20
asgn: ASGNU2(iarg,ac)  "\t%{=vAC}%1%[0b]DOKE(%0);\n" 28
asgn: ASGNI1(zddr,ac)  "\t%1ST(%0);\n"        16
asgn: ASGNI1(iarg,ac)  "\t%1%[0b]POKE(%0);\n" 26
asgn: ASGNU1(zddr,ac)  "\t%1ST(%0);\n"        16
asgn: ASGNU1(iarg,ac)  "\t%1%[0b]POKE(%0);\n" 26

# Conditional branches
stmt: EQI2(ac,con0)  "\t%0_BEQ(%a);\n" 28
stmt: EQI2(ac,conB)  "\t%0XORI(%1);_BEQ(%a)%{!A};\n" 42
stmt: EQI2(ac,iarg)  "\t%0%[1b]XORW(%1);_BEQ(%a)%{!A};\n" 54
stmt: EQI2(iarg,ac)  "\t%1%[0b]XORW(%0);_BEQ(%a)%{!A};\n" 54
stmt: NEI2(ac,con0)  "\t%0_BNE(%a);\n" 28
stmt: NEI2(ac,conB)  "\t%0XORI(%1);_BNE(%a)%{!A};\n" 42
stmt: NEI2(ac,iarg)  "\t%0%[1b]XORW(%1);_BNE(%a)%{!A};\n" 54
stmt: NEI2(iarg,ac)  "\t%1%[0b]XORW(%0);_BNE(%a)%{!A};\n" 54
stmt: EQU2(ac,con0)  "\t%0_BEQ(%a);\n" 28
stmt: EQU2(ac,conB)  "\t%0XORI(%1);_BEQ(%a)%{!A};\n" 42
stmt: EQU2(ac,iarg)  "\t%0%[1b]XORW(%1);_BEQ(%a)%{!A};\n" 54
stmt: EQU2(iarg,ac)  "\t%1%[0b]XORW(%0);_BEQ(%a)%{!A};\n" 54
stmt: NEU2(ac,con0)  "\t%0_BNE(%a);\n" 28
stmt: NEU2(ac,conB)  "\t%0XORI(%1);_BNE(%a)%{!A};\n" 42
stmt: NEU2(ac,iarg)  "\t%0%[1b]XORW(%1);_BNE(%a)%{!A};\n" 54
stmt: NEU2(iarg,ac)  "\t%1%[0b]XORW(%0);_BNE(%a)%{!A};\n" 54
stmt: LTI2(ac,con0) "\t%0_BLT(%a);\n" 28
stmt: LEI2(ac,con0) "\t%0_BLE(%a);\n" 28
stmt: GTI2(ac,con0) "\t%0_BGT(%a);\n" 28
stmt: GEI2(ac,con0) "\t%0_BGE(%a);\n" 28
stmt: GTU2(ac,con0) "\t%0_BNE(%a);\n" 28
stmt: LEU2(ac,con0) "\t%0_BEQ(%a);\n" 28
stmt: LTI2(ac,conB) "\t%0_CMPIS(%1);_BLT(%a)%{!A};\n" ifcpu7(56,64)
stmt: LEI2(ac,conB) "\t%0_CMPIS(%1);_BLE(%a)%{!A};\n" ifcpu7(56,64)
stmt: GTI2(ac,conB) "\t%0_CMPIS(%1);_BGT(%a)%{!A};\n" ifcpu7(56,64)
stmt: GEI2(ac,conB) "\t%0_CMPIS(%1);_BGE(%a)%{!A};\n" ifcpu7(56,64)
stmt: LTU2(ac,conB) "\t%0_CMPIU(%1);_BLT(%a)%{!A};\n" ifcpu7(56,64)
stmt: LEU2(ac,conB) "\t%0_CMPIU(%1);_BLE(%a)%{!A};\n" ifcpu7(56,64)
stmt: GTU2(ac,conB) "\t%0_CMPIU(%1);_BGT(%a)%{!A};\n" ifcpu7(56,64)
stmt: GEU2(ac,conB) "\t%0_CMPIU(%1);_BGE(%a)%{!A};\n" ifcpu7(56,64)
stmt: LTI2(ac,iarg) "\t%0%[1b]_CMPWS(%1);_BLT(%a)%{!A};\n" ifcpu7(56,84)
stmt: LEI2(ac,iarg) "\t%0%[1b]_CMPWS(%1);_BLE(%a)%{!A};\n" ifcpu7(56,84)
stmt: GTI2(ac,iarg) "\t%0%[1b]_CMPWS(%1);_BGT(%a)%{!A};\n" ifcpu7(56,84)
stmt: GEI2(ac,iarg) "\t%0%[1b]_CMPWS(%1);_BGE(%a)%{!A};\n" ifcpu7(56,84)
stmt: LTU2(ac,iarg) "\t%0%[1b]_CMPWU(%1);_BLT(%a)%{!A};\n" ifcpu7(56,84)
stmt: LEU2(ac,iarg) "\t%0%[1b]_CMPWU(%1);_BLE(%a)%{!A};\n" ifcpu7(56,84)
stmt: GTU2(ac,iarg) "\t%0%[1b]_CMPWU(%1);_BGT(%a)%{!A};\n" ifcpu7(56,84)
stmt: GEU2(ac,iarg) "\t%0%[1b]_CMPWU(%1);_BGE(%a)%{!A};\n" ifcpu7(56,84)
stmt: LTI2(iarg,ac) "\t%1%[0b]_CMPWS(%0);_BGT(%a)%{!A};\n" ifcpu7(56,84)
stmt: LEI2(iarg,ac) "\t%1%[0b]_CMPWS(%0);_BGE(%a)%{!A};\n" ifcpu7(56,84)
stmt: GTI2(iarg,ac) "\t%1%[0b]_CMPWS(%0);_BLT(%a)%{!A};\n" ifcpu7(56,84)
stmt: GEI2(iarg,ac) "\t%1%[0b]_CMPWS(%0);_BLE(%a)%{!A};\n" ifcpu7(56,84)
stmt: LTU2(iarg,ac) "\t%1%[0b]_CMPWU(%0);_BGT(%a)%{!A};\n" ifcpu7(56,84)
stmt: LEU2(iarg,ac) "\t%1%[0b]_CMPWU(%0);_BGE(%a)%{!A};\n" ifcpu7(56,84)
stmt: GTU2(iarg,ac) "\t%1%[0b]_CMPWU(%0);_BLT(%a)%{!A};\n" ifcpu7(56,84)
stmt: GEU2(iarg,ac) "\t%1%[0b]_CMPWU(%0);_BLE(%a)%{!A};\n" ifcpu7(56,84)

# Nonterminals for assignments with MOVM/MOVL/MOVF:
#   asgn: ASGNx(vdst,xAC) "\t%1%[0b]_xMOV(xAC,%0);\n"
#   asgn: ASGNx(vdst,reg) "\t%[0b]_xMOV(%1,%0);\n"
#   asgn: ASGNx(addr,INDIRx(asrc)) "\t%[1b]_xMOV(%1,%0);\n"
#   asgn: ASGNx(ac,  INDIRx(asrc)) "\t%0STW(T2);%[1b]_xMOV(%1,[T2]);\n"
#   asgn: ASGNx(lddr,INDIRx(lsrc)) "\t_xMOV(%1,[SP,%0]);\n"
vdst: addr "%0"
vdst: lddr "[SP,%0]" 60
vdst: eac "[vAC]|%0" 20
asrc: addr "%0"
asrc: lddr "[SP,%0]" 40
asrc: eac "[vAC]|%0"
lsrc: addr "%0"

# Structs
asgn: ARGB(INDIRB(asrc))        "\t_SP(%c)%{!A};STW(T2);%[0b]_MOVM(%0,[T2],%a,%b)%{!A};\n"  200
asgn: ASGNB(addr,INDIRB(asrc)) "\t%[1b]_MOVM(%1,%0,%a,%b)%{!A};\n" 200
asgn: ASGNB(ac,  INDIRB(asrc)) "\t%0STW(T2);%[1b]_MOVM(%1,[T2],%a,%b)%{!A};\n" 200
asgn: ASGNB(lddr,INDIRB(lsrc)) "\t_MOVM(%1,[SP,%0],%a,%b)%{!A};\n" 200

# Longs
# - larg represent argument expressions in binary tree nodes,
#   as well as sequence of instructions that compute the address
#   holding the expressiong result.
stmt: lac          "\t%0\n"
larg: regx         "LDI(%0)%{!A};" 21
larg: INDIRI4(eac) "%0"
larg: INDIRU4(eac) "%0"
reg:  lac          "\t%{=LAC}%0%{?c==LAC::_MOVL(LAC,%c);}%{!5}\n" 119
reg: INDIRI4(ac)   "\t%0_MOVL([vAC],%c)%{!A};\n" 120
reg: INDIRU4(ac)   "\t%0_MOVL([vAC],%c)%{!A};\n" 120
reg: INDIRI4(lddr) "\t_MOVL([SP,%0],%c)%{!A};\n" 160
reg: INDIRU4(lddr) "\t_MOVL([SP,%0],%c)%{!A};\n" 160
reg: INDIRI4(addr) "\t_MOVL(%0,%c)%{!A};\n" 120
reg: INDIRU4(addr) "\t_MOVL(%0,%c)%{!A};\n" 120
lac: reg           "%{=%0}%{?0=~LAC::_MOVL(%0,LAC);}%{!5}" 120
lac: INDIRI4(ac)   "%{?*0=~LAC::%0_MOVL([vAC],LAC)%{!A};}" 120
lac: INDIRU4(ac)   "%{?*0=~LAC::%0_MOVL([vAC],LAC)%{!A};}" 120
lac: INDIRU4(lddr) "%{?*0=~LAC::_MOVL([SP,%0],LAC)%{!A};}" 160
lac: INDIRU4(lddr) "%{?*0=~LAC::_MOVL([SP,%0],LAC)%{!A};}" 160
lac: INDIRI4(addr) "%{?*0=~LAC::_MOVL(%0,LAC)%{!A};}" 120
lac: INDIRU4(addr) "%{?*0=~LAC::_MOVL(%0,LAC)%{!A};}" 120
lac: ADDI4(lac,larg) "%0%1_LADD()%{!5};" 200
lac: ADDU4(lac,larg) "%0%1_LADD()%{!5};" 200
lac: ADDI4(larg,lac) "%1%0_LADD()%{!5};" 200
lac: ADDU4(larg,lac) "%1%0_LADD()%{!5};" 200
lac: SUBI4(lac,larg) "%0%1_LSUB()%{!5};" 200
lac: SUBU4(lac,larg) "%0%1_LSUB()%{!5};" 200
lac: MULI4(lac,larg) "%0%1_LMUL()%{!A};" 200
lac: MULU4(lac,larg) "%0%1_LMUL()%{!A};" 200
lac: MULI4(larg,lac) "%1%0_LMUL()%{!A};" 200
lac: MULU4(larg,lac) "%1%0_LMUL()%{!A};" 200
lac: DIVI4(lac,larg) "%0%1_LDIVS()%{!A};" 200
lac: DIVU4(lac,larg) "%0%1_LDIVU()%{!A};" 200
lac: MODI4(lac,larg) "%0%1_LMODS()%{!A};" 200
lac: MODU4(lac,larg) "%0%1_LMODU()%{!A};" 200
lac: LSHI4(lac,reg)  "%0LDW(%1);_LSHL()%{!A};"  200
lac: LSHI4(lac,conB) "%0LDI(%1);_LSHL()%{!A};"  200
lac: LSHU4(lac,reg)  "%0LDW(%1);_LSHL()%{!A};"  200
lac: LSHU4(lac,conB) "%0LDI(%1);_LSHL()%{!A};"  200
lac: RSHI4(lac,reg)  "%0LDW(%1);_LSHRS()%{!A};" 200
lac: RSHI4(lac,conB) "%0LDI(%1);_LSHRS()%{!A};" 200
lac: RSHU4(lac,reg)  "%0LDW(%1);_LSHRU()%{!A};" 200
lac: RSHU4(lac,conB) "%0LDI(%1);_LSHRU()%{!A};" 200
lac: NEGI4(lac)       "%0_LNEG()%{!5};"   200
lac: BCOMU4(lac)      "%0_LCOM()%{!A};"   200
lac: BANDU4(lac,larg) "%0%1_LAND()%{!A};" 200
lac: BANDU4(larg,lac) "%1%0_LAND()%{!A};" 200
lac: BORU4(lac,larg)  "%0%1_LOR()%{!A};"  200
lac: BORU4(larg,lac)  "%1%0_LOR()%{!A};"  200
lac: BXORU4(lac,larg) "%0%1_LXOR()%{!A};" 200
lac: BXORU4(larg,lac) "%1%0_LXOR()%{!A};" 200
lac: BCOMI4(lac)      "%0_LCOM()%{!A};"   200
lac: BANDI4(lac,larg) "%0%1_LAND()%{!A};" 200
lac: BANDI4(larg,lac) "%1%0_LAND()%{!A};" 200
lac: BORI4(lac,larg) "%0%1_LOR()%{!A};"   200
lac: BORI4(larg,lac) "%1%0_LOR()%{!A};"   200
lac: BXORI4(lac,larg) "%0%1_LXOR()%{!A};" 200
lac: BXORI4(larg,lac) "%1%0_LXOR()%{!A};" 200
stmt: LTI4(lac,larg) "\t%0%1_LCMPS();_BLT(%a)%{!A};\n" 200
stmt: LEI4(lac,larg) "\t%0%1_LCMPS();_BLE(%a)%{!A};\n" 200
stmt: GTI4(lac,larg) "\t%0%1_LCMPS();_BGT(%a)%{!A};\n" 200
stmt: GEI4(lac,larg) "\t%0%1_LCMPS();_BGE(%a)%{!A};\n" 200
stmt: LTU4(lac,larg) "\t%0%1_LCMPU();_BLT(%a)%{!A};\n" 200
stmt: LEU4(lac,larg) "\t%0%1_LCMPU();_BLE(%a)%{!A};\n" 200
stmt: GTU4(lac,larg) "\t%0%1_LCMPU();_BGT(%a)%{!A};\n" 200
stmt: GEU4(lac,larg) "\t%0%1_LCMPU();_BGE(%a)%{!A};\n" 200
stmt: NEI4(lac,larg) "\t%0%1_LCMPX();_BNE(%a)%{!A};\n" 100
stmt: EQI4(lac,larg) "\t%0%1_LCMPX();_BEQ(%a)%{!A};\n" 100
stmt: NEU4(lac,larg) "\t%0%1_LCMPX();_BNE(%a)%{!A};\n" 100
stmt: EQU4(lac,larg) "\t%0%1_LCMPX();_BEQ(%a)%{!A};\n" 100
stmt: LTI4(larg,lac) "\t%1%0_LCMPS();_BGT(%a)%{!A};\n" 200
stmt: LEI4(larg,lac) "\t%1%0_LCMPS();_BGE(%a)%{!A};\n" 200
stmt: GTI4(larg,lac) "\t%1%0_LCMPS();_BLT(%a)%{!A};\n" 200
stmt: GEI4(larg,lac) "\t%1%0_LCMPS();_BLE(%a)%{!A};\n" 200
stmt: LTU4(larg,lac) "\t%1%0_LCMPU();_BGT(%a)%{!A};\n" 200
stmt: LEU4(larg,lac) "\t%1%0_LCMPU();_BGE(%a)%{!A};\n" 200
stmt: GTU4(larg,lac) "\t%1%0_LCMPU();_BLT(%a)%{!A};\n" 200
stmt: GEU4(larg,lac) "\t%1%0_LCMPU();_BLE(%a)%{!A};\n" 200
stmt: NEI4(larg,lac) "\t%1%0_LCMPX();_BNE(%a)%{!A};\n" 100
stmt: EQI4(larg,lac) "\t%1%0_LCMPX();_BEQ(%a)%{!A};\n" 100
stmt: NEU4(larg,lac) "\t%1%0_LCMPX();_BNE(%a)%{!A};\n" 100
stmt: EQU4(larg,lac) "\t%1%0_LCMPX();_BEQ(%a)%{!A};\n" 100
asgn: ASGNI4(vdst,lac)           "\t%{=LAC}%1%[0b]_MOVL(LAC,%0)%{!A};\n"   120
asgn: ASGNI4(vdst,reg)           "\t%[0b]_MOVL(%1,%0)%{!A};\n"             120
asgn: ASGNI4(addr,INDIRI4(asrc)) "\t%[1b]_MOVL(%1,%0)%{!A};\n"             120
asgn: ASGNI4(ac,  INDIRI4(asrc)) "\t%0STW(T2);%[1b]_MOVL(%1,[T2])%{!A};\n" 120
asgn: ASGNI4(lddr,INDIRI4(lsrc)) "\t_MOVL(%1,[SP,%0])%{!A};\n"             160
asgn: ASGNU4(vdst,lac)           "\t%{=LAC}%1%[0b]_MOVL(LAC,%0)%{!A};\n"   120
asgn: ASGNU4(vdst,reg)           "\t%[0b]_MOVL(%1,%0)%{!A};\n"             120
asgn: ASGNU4(addr,INDIRU4(asrc)) "\t%[1b]_MOVL(%1,%0)%{!A};\n"             120
asgn: ASGNU4(ac,  INDIRU4(asrc)) "\t%0STW(T2);%[1b]_MOVL(%1,[T2])%{!A};\n" 120
asgn: ASGNU4(lddr,INDIRU4(lsrc)) "\t_MOVL(%1,[SP,%0])%{!A};\n"             160

# Floats
stmt: fac "\t%0\n"
farg: regx "LDI(%0)%{!A};" 21
farg: INDIRF5(eac) "%0"
reg:  fac           "\t%{=FAC}%0%{?c==FAC::_MOVF(FAC,%c);}%{!A}\n" 179
reg: INDIRF5(ac)    "\t%0_MOVF([vAC],%c)%{!A};\n" 150
reg: INDIRF5(lddr)  "\t_MOVF([SP,%0],%c)%{!A};\n" 190
reg: INDIRF5(addr)  "\t_MOVF(%0,%c)%{!A%c};\n"    150
fac: reg            "%{=%0}%{?0=~FAC::_MOVF(%0,FAC)%{!A};}"  179
fac: INDIRF5(ac)    "%{?*0=~FAC::%0_MOVF([vAC],FAC)%{!A};}"  180
fac: INDIRF5(lddr)  "%{?*0=~FAC::_MOVF([SP,%0],FAC)%{!A};}"  220
fac: INDIRF5(addr)  "%{?*0=~FAC::_MOVF(%0,FAC)%{!A};}"       180
fac: ADDF5(fac,farg) "%0%1_FADD()%{!A};"          200
fac: ADDF5(farg,fac) "%1%0_FADD()%{!A};"          200
fac: SUBF5(fac,farg) "%0%1_FSUB()%{!A};"          200
fac: SUBF5(farg,fac) "%1_FNEG();%0_FADD()%{!A};"  200+50
fac: MULF5(fac,farg) "%0%1_FMUL()%{!A};"          200
fac: MULF5(farg,fac) "%1%0_FMUL()%{!A};"          200
fac: DIVF5(fac,farg) "%0%1_FDIV()%{!A};"          200
fac: NEGF5(fac)      "%0_FNEG()%{!A};"            50
stmt: EQF5(fac,farg) "\t%0%1_FCMP();_BEQ(%a)%{!A};\n" 200
stmt: NEF5(fac,farg) "\t%0%1_FCMP();_BNE(%a)%{!A};\n" 200
stmt: LTF5(fac,farg) "\t%0%1_FCMP();_BLT(%a)%{!A};\n" 200
stmt: LEF5(fac,farg) "\t%0%1_FCMP();_BLE(%a)%{!A};\n" 200
stmt: GTF5(fac,farg) "\t%0%1_FCMP();_BGT(%a)%{!A};\n" 200
stmt: GEF5(fac,farg) "\t%0%1_FCMP();_BGE(%a)%{!A};\n" 200
stmt: EQF5(farg,fac) "\t%1%0_FCMP();_BEQ(%a)%{!A};\n" 200
stmt: NEF5(farg,fac) "\t%1%0_FCMP();_BNE(%a)%{!A};\n" 200
stmt: LTF5(farg,fac) "\t%1%0_FCMP();_BGT(%a)%{!A};\n" 200
stmt: LEF5(farg,fac) "\t%1%0_FCMP();_BGE(%a)%{!A};\n" 200
stmt: GTF5(farg,fac) "\t%1%0_FCMP();_BLT(%a)%{!A};\n" 200
stmt: GEF5(farg,fac) "\t%1%0_FCMP();_BLE(%a)%{!A};\n" 200
asgn: ASGNF5(vdst,fac) "\t%{=FAC}%1%[0b]_MOVF(FAC,%0)%{!A};\n"             180
asgn: ASGNF5(vdst,reg) "\t%[0b]_MOVF(%1,%0)%{!A};\n"                       150
asgn: ASGNF5(addr,INDIRF5(asrc)) "\t%[1b]_MOVF(%1,%0)%{!A};\n"             150
asgn: ASGNF5(ac,  INDIRF5(asrc)) "\t%0STW(T2);%[1b]_MOVF(%1,[T2])%{!A};\n" 150
asgn: ASGNF5(lddr,INDIRF5(lsrc)) "\t_MOVF(%1,[SP,%0])%{!A};\n"             190

# Calls
fac: CALLF5(addr) "CALLI(%0)%{!ALF};" mincpu5(28)
fac: CALLF5(reg)  "CALL(%0)%{!ALF};" 26
fac: CALLF5(ac)   "%0CALL(vAC)%{!ALF};" 26
lac: CALLI4(addr) "CALLI(%0)%{!ALF};" mincpu5(28)
lac: CALLI4(reg)  "CALL(%0)%{!ALF};" 26
lac: CALLI4(ac)   "%0CALL(vAC)%{!ALF};" 26
lac: CALLU4(addr) "CALLI(%0)%{!ALF};" mincpu5(28)
lac: CALLU4(reg)  "CALL(%0)%{!ALF};" 26
lac: CALLU4(ac)   "%0CALL(vAC)%{!ALF};" 26
ac: CALLI2(addr)  "CALLI(%0)%{!ALF};" mincpu5(28)
ac: CALLI2(reg)   "CALL(%0)%{!ALF};" 26
ac: CALLI2(ac)    "%0CALL(vAC)%{!ALF};" 26
ac: CALLU2(addr)  "CALLI(%0)%{!ALF};" mincpu5(28)
ac: CALLU2(reg)   "CALL(%0)%{!ALF};" 26
ac: CALLU2(ac)    "%0CALL(vAC)%{!ALF};" 26
ac: CALLP2(addr)  "CALLI(%0)%{!ALF};" mincpu5(28)
ac: CALLP2(reg)   "CALL(%0)%{!ALF};" 26
ac: CALLP2(ac)    "%0CALL(vAC)%{!ALF};" 26
stmt: CALLV(addr) "\tCALLI(%0)%{!ALF};\n" mincpu5(28)
stmt: CALLV(reg)  "\tCALL(%0)%{!ALF};\n" 26
stmt: CALLV(ac)   "\t%0CALL(vAC)%{!ALF};\n" 26
asgn: ARGF5(reg)  "\t_MOVF(%0,[SP,%c])%{!A};\n"  if_arg_stk(a,100)
asgn: ARGI4(reg)  "\t_MOVL(%0,[SP,%c])%{!A};\n"  if_arg_stk(a,100)
asgn: ARGU4(reg)  "\t_MOVL(%0,[SP,%c])%{!A};\n"  if_arg_stk(a,100)
asgn: ARGI2(reg)  "\t_STLW(%c,src=%0)%{!A};\n"   if_arg_stk(a,100)
asgn: ARGU2(reg)  "\t_STLW(%c,src=%0)%{!A};\n"   if_arg_stk(a,100)
asgn: ARGP2(reg)  "\t_STLW(%c,src=%0)%{!A};\n"   if_arg_stk(a,100)
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
eac0: LOADI1(reg) "LD(%0);" 22
eac0: LOADU1(reg) "LD(%0);" 22
ac: LOADI1(ac) "%{=%0}%0"
ac: LOADU1(ac) "%{=%0}%0"
ac: LOADI2(ac) "%{=%0}%0"
ac: LOADU2(ac) "%{=%0}%0"
ac: LOADP2(ac) "%{=%0}%0"
eac: LOADI1(eac) "%{=%0}%0"
eac: LOADU1(eac) "%{=%0}%0"
eac: LOADI2(eac) "%{=%0}%0"
eac: LOADU2(eac) "%{=%0}%0"
eac: LOADP2(eac) "%{=%0}%0"
ac: LOADI2(reg)  "%{=%0}%{?0=~vAC::LDW(%0);}" 20
ac: LOADU2(reg)  "%{=%0}%{?0=~vAC::LDW(%0);}" 20
ac: LOADP2(reg)  "%{=%0}%{?0=~vAC::LDW(%0);}" 20
ac: LOADI2(lac) "%0LDW(LAC);" 20
ac: LOADU2(lac) "%0LDW(LAC);" 20
ac: LOADP2(lac) "%0LDW(LAC);" 20
lac: LOADI4(lac) "%{=%0}%0"
lac: LOADU4(lac) "%{=%0}%0"
fac: LOADF5(fac) "%{=%0}%0"

reg: LOADI1(reg)   "\t%{?0=~vAC::LD(%0);}{?c==vAC::ST(%c);}%{!A}\n"   38
reg: LOADU1(reg)   "\t%{?0=~vAC::LD(%0);}{?c==vAC::ST(%c);}%{!A}\n"   38
reg: LOADI1(ac)    "\t%0%{?c==vAC::ST(%c);}\n"   16
reg: LOADU1(ac)    "\t%0%{?c==vAC::ST(%c);}\n"   16
reg: LOADI4(reg)   "\t_MOVL(%0,%c)%{!5};\n" 120
reg: LOADU4(reg)   "\t_MOVL(%0,%c)%{!5};\n" 120
regx: LOADF5(regx) "\t_MOVF(%0,%c)%{!5};\n" 150

# 2) extensions
ac: CVII2(reg) "LD(%0);XORI(128);SUBI(128);" if_cv_from(a,1,66)
ac: CVUI2(reg) "LD(%0);" if_cv_from(a,1,18)
ac: CVII2(ac0) "%0XORI(128);SUBI(128);" if_cv_from(a,1,48)
ac: CVUI2(ac0) "%0" if_cv_from(a,1,0)
ac: CVII2(ac) "%0LD(vACL);XORI(128);SUBI(128);" if_cv_from(a,1,66)
ac: CVUI2(ac) "%0LD(vACL);" if_cv_from(a,1,18)
eac: CVII2(eac0) "%0XORI(128);SUBI(128);" if_cv_from(a,1,48)
eac: CVUI2(eac0) "%0" if_cv_from(a,1,0)
eac: CVII2(eac) "%0LD(vACL);XORI(128);SUBI(128);" if_cv_from(a,1,66)
eac: CVUI2(eac) "%0LD(vACL);" if_cv_from(a,1,18)
lac: CVIU4(ac) "%0_STLU(LAC);" 50
lac: CVII4(ac) "%0_STLS(LAC);" 50
lac: CVUU4(ac) "%0_STLU(LAC);" 50
lac: CVUI4(ac) "%0_STLU(LAC);" 50
reg: CVIU4(ac) "\t%0_STLU(%c)%{!5};\n" 50
reg: CVII4(ac) "\t%0_STLS(%c)%{!5};\n" 50
reg: CVUU4(ac) "\t%0_STLU(%c)%{!5};\n" 50
reg: CVUI4(ac) "\t%0_STLU(%c)%{!5};\n" 50
# 3) floating point conversions
ac: CVFU2(fac)  "%0_FTOU();LDW(LAC)%{!L};" 200
lac: CVFU4(fac) "%0_FTOU();" 200
fac: CVUF5(ac)  "%0_STLU(LAC);_FCVU();" if_cv_from(a,2,180)
fac: CVUF5(lac) "%0_FCVU();" if_cv_from(a,4,200)
ac: CVFI2(fac)  "%0_FTOI();LDW(LAC)%{!L};" 200
lac: CVFI4(fac) "%0_FTOI();" 200
fac: CVIF5(ac)  "%0_STLS(LAC);_FCVI();" if_cv_from(a,2,180)
fac: CVIF5(lac) "%0_FCVI();" if_cv_from(a,4,200)

# Labels and jumps
stmt: LABELV       "\tlabel(%a)%{!ALF};\n"
stmt: JUMPV(addr)  "\t_BRA(%0);\n"          14
stmt: JUMPV(reg)   "\tCALL(%0)%{!ALF};\n"    14
stmt: JUMPV(ac)    "\t%0CALL(vAC)%{!ALF};\n" 14

# More about spills: we want to save/restore vAC when genspill() inserts
# instructions because preralloc might have decided to use vAC at this
# precise point.
asgn: ASGNI2(spill,reg) "\tSTW(B0);_STLW(%0,src=%1);LDW(B0)  #genspill\n" 20
asgn: ASGNU2(spill,reg) "\tSTW(B0);_STLW(%0,src=%1);LDW(B0)  #genspill\n" 20
asgn: ASGNP2(spill,reg) "\tSTW(B0);_STLW(%0,src=%1);LDW(B0)  #genspill\n" 20
asgn: ASGNI4(spill,reg) "\tSTW(B0);_MOVL(%1,[SP,%0]);LDW(B0) #genspill\n" 20
asgn: ASGNU4(spill,reg) "\tSTW(B0);_MOVL(%1,[SP,%0]);LDW(B0) #genspill\n" 20
asgn: ASGNF5(spill,reg) "\tSTW(B0);_MOVF(%1,[SP,%0]);LDW(B0) #genspill\n" 20

# Additional rules for cpu > 5
ac:  MULI2(con,ac)  "%1_MULI(%0);"  mincpu7(80)
ac:  MULU2(con,ac)  "%1_MULI(%0);"  mincpu7(80)
ac:  CVII2(ac)      "%0LDSB(vACL);" mincpu7(if_cv_from(a,1,26))
ac:  NEGI2(ac)      "%0NEGV(vAC);"  mincpu6(26)
eac: MULI2(con,eac)  "%1_MULI(%0);"  mincpu7(80)
eac: MULU2(con,eac)  "%1_MULI(%0);"  mincpu7(80)
eac: CVII2(reg)     "LDSB(%0);"     mincpu7(if_cv_from(a,1,26))
eac: CVII2(eac)     "%0LDSB(vACL);" mincpu7(if_cv_from(a,1,26))
eac: NEGI2(eac)     "%0NEGV(vAC);"  mincpu6(26)
lac: NEGI4(lac)     "%0NEGVL(LAC);" mincpu6(58)
asgn: ASGNP2(ac,iarg)  "\t%{=%1}%0%[1b]DOKEA(%1);\n" mincpu6(28)
asgn: ASGNI2(ac,iarg)  "\t%{=%1}%0%[1b]DOKEA(%1);\n" mincpu6(28)
asgn: ASGNU2(ac,iarg)  "\t%{=%1}%0%[1b]DOKEA(%1);\n" mincpu6(28)
asgn: ASGNI1(ac,iarg)  "\t%{=%1}%0%[1b]POKEA(%1);\n" mincpu6(28)
asgn: ASGNU1(ac,iarg)  "\t%{=%1}%0%[1b]POKEA(%1);\n" mincpu6(28)
asgn: ASGNP2(ac,con)   "\t%{=%1}%0%[1b]DOKEI(%1);\n" mincpu6(29)
asgn: ASGNI2(ac,con)   "\t%{=%1}%0%[1b]DOKEI(%1);\n" mincpu6(29)
asgn: ASGNU2(ac,con)   "\t%{=%1}%0%[1b]DOKEI(%1);\n" mincpu6(29)
asgn: ASGNP2(ac,conB)  "\t%{=%1}%0%[1b]DOKEQ(%1);\n" mincpu7(21)
asgn: ASGNI2(ac,conB)  "\t%{=%1}%0%[1b]DOKEQ(%1);\n" mincpu7(21)
asgn: ASGNU2(ac,conB)  "\t%{=%1}%0%[1b]DOKEQ(%1);\n" mincpu7(21)
asgn: ASGNI1(ac,conBs) "\t%0%[1b]POKEQ(%1);\n" mincpu6(19)
asgn: ASGNU1(ac,conB)  "\t%0%[1b]POKEQ(%1);\n" mincpu6(19)
eac: INDIRI2(reg)     "%{?*0=~vAC::%{?0=~vAC:DEEK():DEEKV(%0)};}" mincpu6(28)
eac: INDIRU2(reg)     "%{?*0=~vAC::%{?0=~vAC:DEEK():DEEKV(%0)};}" mincpu6(28)
eac: INDIRP2(reg)     "%{?*0=~vAC::%{?0=~vAC:DEEK():DEEKV(%0)};}" mincpu6(28)
eac0: INDIRI1(reg)     "%{?0=~vAC:PEEK():PEEKV(%0)};" mincpu6(28)
eac0: INDIRU1(reg)     "%{?0=~vAC:PEEK():PEEKV(%0)};" mincpu6(28)
reg: INDIRI2(ac)     "\t%{?*0=~vAC:STW(%c):%0%{?c==vAC:DEEK():DEEKA(%c)};}\n" mincpu6(30)
reg: INDIRU2(ac)     "\t%{?*0=~vAC:STW(%c):%0%{?c==vAC:DEEK():DEEKA(%c)};}\n" mincpu6(30)
reg: INDIRP2(ac)     "\t%{?*0=~vAC:STW(%c):%0%{?c==vAC:DEEK():DEEKA(%c)};}\n" mincpu6(30)
reg: INDIRI1(ac)     "\t%0%{?c==vAC:PEEK():PEEKA(%c)};\n" mincpu6(24+5)
reg: INDIRU1(ac)     "\t%0%{?c==vAC:PEEK():PEEKA(%c)};\n" mincpu6(24+5)
asgn: ASGNI1(rmw, conBs) "\t%{?1=~vAC:ST(%0):MOVQB(%1,%0)};\n"  mincpu6(if_not_asgn_tmp(a,27))
asgn: ASGNU1(rmw, conB)  "\t%{?1=~vAC:ST(%0):MOVQB(%1,%0)};\n"  mincpu6(if_not_asgn_tmp(a,27))
asgn: ASGNI2(rmw, conB)  "\t%{?1=~vAC:STW(%0):MOVQW(%1,%0)};\n" mincpu6(if_not_asgn_tmp(a,29))
asgn: ASGNU2(rmw, conB)  "\t%{?1=~vAC:STW(%0):MOVQW(%1,%0)};\n" mincpu6(if_not_asgn_tmp(a,29))
asgn: ASGNP2(rmw, conB)  "\t%{?1=~vAC:STW(%0):MOVQW(%1,%0)};\n" mincpu6(if_not_asgn_tmp(a,29))
asgn: ASGNI2(rmw, con)   "\t%{?1=~vAC:STW(%0):MOVIW(%1,%0)};\n" mincpu7(if_not_asgn_tmp(a,31))
asgn: ASGNU2(rmw, con)   "\t%{?1=~vAC:STW(%0):MOVIW(%1,%0)};\n" mincpu7(if_not_asgn_tmp(a,31))
asgn: ASGNP2(rmw, con)   "\t%{?1=~vAC:STW(%0):MOVIW(%1,%0)};\n" mincpu7(if_not_asgn_tmp(a,31))
regx: LOADI1(conBs)      "\t%{?0=~vAC:ST(%c):MOVQB(%0,%c)};\n"  mincpu6(27)
regx: LOADU1(conB)       "\t%{?0=~vAC:ST(%c):MOVQB(%0,%c)};\n"  mincpu6(27)
regx: LOADI2(conB)       "\t%{?0=~vAC:STW(%c):MOVQW(%0,%c)};\n" mincpu6(29)
regx: LOADU2(conB)       "\t%{?0=~vAC:STW(%c):MOVQW(%0,%c)};\n" mincpu6(29)
regx: LOADP2(conB)       "\t%{?0=~vAC:STW(%c):MOVQW(%0,%c)};\n" mincpu6(29)
regx: LOADI2(con)        "\t%{?0=~vAC:STW(%c):MOVIW(%0,%c)};\n" mincpu7(31)
regx: LOADU2(con)        "\t%{?0=~vAC:STW(%c):MOVIW(%0,%c)};\n" mincpu7(31)
regx: LOADP2(con)        "\t%{?0=~vAC:STW(%c):MOVIW(%0,%c)};\n" mincpu7(31)
eac: INDIRI2(ADDP2(reg,con)) "LDXW(%0,%1);" mincpu7(60)
eac: INDIRU2(ADDP2(reg,con)) "LDXW(%0,%1);" mincpu7(60)
eac: INDIRP2(ADDP2(reg,con)) "LDXW(%0,%1);" mincpu7(60)
ac: INDIRI2(ADDP2(ac,con))  "%0LDXW(vAC,%1);" mincpu7(60)
ac: INDIRU2(ADDP2(ac,con))  "%0LDXW(vAC,%1);" mincpu7(60)
ac: INDIRP2(ADDP2(ac,con))  "%0LDXW(vAC,%1);" mincpu7(60)
eac: INDIRI2(ADDP2(eac,con)) "%0LDXW(vAC,%1);" mincpu7(60)
eac: INDIRU2(ADDP2(eac,con)) "%0LDXW(vAC,%1);" mincpu7(60)
eac: INDIRP2(ADDP2(eac,con)) "%0LDXW(vAC,%1);" mincpu7(60)
asgn: ASGNI2(ADDP2(reg,con),ac) "\t%2STXW(%0,%1);\n" mincpu7(58)
asgn: ASGNU2(ADDP2(reg,con),ac) "\t%2STXW(%0,%1);\n" mincpu7(58)
asgn: ASGNP2(ADDP2(reg,con),ac) "\t%2STXW(%0,%1);\n" mincpu7(58)
eac: INDIRI2(lddr)  "%{?*0=~vAC::_LDLW(%0);}"  mincpu7(if_zoffset(a,38,60))
eac: INDIRU2(lddr)  "%{?*0=~vAC::_LDLW(%0);}"  mincpu7(if_zoffset(a,38,60))
eac: INDIRP2(lddr)  "%{?*0=~vAC::_LDLW(%0);}"  mincpu7(if_zoffset(a,38,60))
asgn: ASGNI2(lddr,ac) "\t%{=vAC}%1_STLW(%0);\n"  mincpu7(if_zoffset(a,38,60))
asgn: ASGNU2(lddr,ac) "\t%{=vAC}%1_STLW(%0);\n"  mincpu7(if_zoffset(a,38,60))
asgn: ASGNP2(lddr,ac) "\t%{=vAC}%1_STLW(%0);\n"  mincpu7(if_zoffset(a,38,60))
asgn: ARGI2(reg) "\t%{?0=~vAC::LDW(%0);}_STLW(%c)%{!A};\n"  mincpu7(if_arg_stk(a,50))
asgn: ARGU2(reg) "\t%{?0=~vAC::LDW(%0);}_STLW(%c)%{!A};\n"  mincpu7(if_arg_stk(a,50))
asgn: ARGP2(reg) "\t%{?0=~vAC::LDW(%0);}_STLW(%c)%{!A};\n"  mincpu7(if_arg_stk(a,50))

# Read-modify-write
rmw: VREGP "%a"
rmw: zddr "%0"
asgn: ASGNU1(rmw, LOADU1(ADDI2(CVUI2(INDIRU1(rmw)), con1))) "\tINC(%0);\n" if_rmw(a,16)
asgn: ASGNI1(rmw, LOADI1(ADDI2(CVII2(INDIRI1(rmw)), con1))) "\tINC(%0);\n" if_rmw(a,16)
asgn: ASGNI2(rmw, NEGI2(INDIRI2(rmw))) "\tNEGV(%0);\n" mincpu6(if_rmw(a, 26))
asgn: ASGNI4(rmw, NEGI4(INDIRI4(rmw))) "\tNEGVL(%0);\n" mincpu6(if_rmw(a, 58))
asgn: ASGNP2(rmw, ADDP2(INDIRP2(rmw), con1)) "\tINCV(%0);\n" mincpu6(if_rmw(a, 22))
asgn: ASGNU2(rmw, ADDU2(INDIRU2(rmw), con1)) "\tINCV(%0);\n" mincpu6(if_rmw(a, 22))
asgn: ASGNI2(rmw, ADDI2(INDIRI2(rmw), con1)) "\tINCV(%0);\n" mincpu6(if_rmw(a, 22))
asgn: ASGNI2(rmw, ADDI2(INDIRI2(rmw), ac)) "\t%2ADDV(%0);\n" mincpu7(if_rmw(a, 30))
asgn: ASGNU2(rmw, ADDU2(INDIRU2(rmw), ac)) "\t%2ADDV(%0);\n" mincpu7(if_rmw(a, 30))
asgn: ASGNP2(rmw, ADDP2(INDIRP2(rmw), ac)) "\t%2ADDV(%0);\n" mincpu7(if_rmw(a, 30))
asgn: ASGNI2(rmw, ADDI2(ac, INDIRI2(rmw))) "\t%1ADDV(%0);\n" mincpu7(if_rmw_a(a, 1, 30))
asgn: ASGNU2(rmw, ADDU2(ac, INDIRU2(rmw))) "\t%1ADDV(%0);\n" mincpu7(if_rmw_a(a, 1, 30))
asgn: ASGNP2(rmw, ADDP2(ac, INDIRP2(rmw))) "\t%1ADDV(%0);\n" mincpu7(if_rmw_a(a, 1, 30))
asgn: ASGNI2(rmw, SUBI2(INDIRI2(rmw), ac)) "\t%2SUBV(%0);\n" mincpu7(if_rmw(a, 30))
asgn: ASGNU2(rmw, SUBU2(INDIRU2(rmw), ac)) "\t%2SUBV(%0);\n" mincpu7(if_rmw(a, 30))
asgn: ASGNP2(rmw, SUBP2(INDIRP2(rmw), ac)) "\t%2SUBV(%0);\n" mincpu7(if_rmw(a, 30))
asgn: ASGNP2(rmw, ADDP2(INDIRP2(rmw), conB))  "\t%{?2=~vAC:ADDV(%0):ADDIV(%2,%0)};\n"    mincpu7(if_rmw_incr(a, 40, 10))
asgn: ASGNU2(rmw, ADDU2(INDIRU2(rmw), conB))  "\t%{?2=~vAC:ADDV(%0):ADDIV(%2,%0)};\n"    mincpu7(if_rmw_incr(a, 40, 10))
asgn: ASGNI2(rmw, ADDI2(INDIRI2(rmw), conB))  "\t%{?2=~vAC:ADDV(%0):ADDIV(%2,%0)};\n"    mincpu7(if_rmw_incr(a, 40, 10))
asgn: ASGNP2(rmw, ADDP2(INDIRP2(rmw), conBn)) "\t%{?2=~vAC:ADDV(%0):SUBIV(-(%2),%0)};\n" mincpu7(if_rmw_incr(a, 40, 10))
asgn: ASGNU2(rmw, ADDU2(INDIRU2(rmw), conBn)) "\t%{?2=~vAC:ADDV(%0):SUBIV(-(%2),%0)};\n" mincpu7(if_rmw_incr(a, 40, 10))
asgn: ASGNI2(rmw, ADDI2(INDIRI2(rmw), conBn)) "\t%{?2=~vAC:ADDV(%0):SUBIV(-(%2),%0)};\n" mincpu7(if_rmw_incr(a, 40, 10))
asgn: ASGNP2(rmw, SUBP2(INDIRP2(rmw), conB))  "\t%{?2=~vAC:SUBV(%0):SUBIV(%2,%0)};\n"    mincpu7(if_rmw_incr(a, 40, 10))
asgn: ASGNU2(rmw, SUBU2(INDIRU2(rmw), conB))  "\t%{?2=~vAC:SUBV(%0):SUBIV(%2,%0)};\n"    mincpu7(if_rmw_incr(a, 40, 10))
asgn: ASGNI2(rmw, SUBI2(INDIRI2(rmw), conB))  "\t%{?2=~vAC:SUBV(%0):SUBIV(%2,%0)};\n"    mincpu7(if_rmw_incr(a, 40, 10))


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

/* Collect lines for the module manifest */
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

/* Count mask bits */
static int bitcount(unsigned mask) {
  unsigned i, n = 0;
  for (i = 1; i; i <<= 1)
    if (mask&i)
      n++;
  return n;
}

/* Compare trees */
static int sametree(Node p, Node q) {
  return p == NULL && q == NULL
    || p && q && p->op == q->op && p->syms[0] == q->syms[0]
    && sametree(p->kids[0], q->kids[0])
    && sametree(p->kids[1], q->kids[1]);
}

/* Compare constants (aggregating signed and unsigned types) */
static int samecnst(Symbol a, Symbol b)
{
  if (! (a && b))
    return 0;
  if (a == b)
    return 1;
  if (a->scope == CONSTANTS && isint(a->type)
      && b->scope == CONSTANTS && isint(b->type)
      && (a->type->size == b->type->size)
      && (a->u.c.v.i == b->u.c.v.i) )
    return 1;
  return 0;
}

/* Find next tree (in forest or in next forest) */
static Node peektrees(Node tree, int n, Node trees[])
{
  int i = 0;
  Code cp = 0;
  while (i < n) {
    if (tree->link) {
      trees[i++] = tree = tree->link;
    } else {
      if (! cp)
        for (cp = &codehead; cp; cp = cp->next)
          if (cp->kind == Gen && cp->u.forest == tree)
            break;
      if (cp)
        while ((cp = cp->next))
          if (cp->kind != Defpoint)
            break;
      if (cp && cp->kind == Gen)
        trees[i++] = tree = cp->u.forest;
      else
        break;
    }
  }
  while (i < n)
    trees[i++] = 0;
}

/* Test whether tree is a simple use of a constant cnst*/
static int simplecnstuse(Node p, Symbol cnst)
{
  if (p && generic(p->op) == ASGN) {
    int op = specific(p->kids[0]->op);
    if (op != VREG+P && op != ADDRL+P && op != ADDRF+P)
      return 0;
    p = p->kids[1];
    op = generic(p->op);
    if (cpu >= 7 && (op == ADD || op == SUB)
        && p->kids[0] && generic(p->kids[0]->op) == INDIR) {
      op = specific(p->kids[0]->kids[0]->op);
      if (op != VREG+P && op != ADDRL+P && op != ADDRF+P)
        return 0;
      p = p->kids[1];
      op = generic(p->op);
    }
    if (p && op == CNST && p->syms[0] == cnst)
      return 1;
    if (p && op == INDIR && p->kids[0]->syms[0] == cnst)
      return 1;
  }
  return 0;
}

/* Cost predicates */
static int if_arg_reg_only(Node p)
{
  return p->syms[2] ? LBURG_MAX : 1;
}

static int if_arg_stk(Node p, int c)
{
  return p->syms[2] ? c : LBURG_MAX;
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
  Symbol s = p->syms[0];
  if (s && s->type && fnqual(s->type) == NEAR)
    return 0;
  return LBURG_MAX;
}

static int if_incr(Node a, int cost, int bonus)
{
  /* Reduces the cost of an increment/decrement operation
     when there is evidence that the previous value was preserved in a
     temporary. This is used to prefer dialect LDW(r);ADDW(i);STW(r)
     or ADDIV(i,r) over the potentially more efficient LDI(i);ADDW(r);STW(r)
     or LDI(i);ADDV(r). This is hacky and very limited in fact. */
  extern Node head; /* declared in gen.c */
  Node k;
  Symbol syma;
  if (a && (generic(a->op)==ADD || generic(a->op)==SUB) &&
      (k = a->kids[0]) && generic(k->op) == INDIR &&
      k->kids[0] && specific(k->kids[0]->op) == VREG+P &&
      (syma = k->kids[0]->syms[0]) && syma->temporary )
    {
      Node h;
      int c = cost;
      for(h = head; h; h = h->link) {
        if (h->kids[0] == a || h->kids[1] == a)
          return c;
        if (generic(h->op) == ASGN &&
            h->kids[0] && specific(h->kids[0]->op) == VREG+P &&
            h->kids[0]->syms[0] == syma && syma->u.t.cse == h->kids[1]) 
          c = cost - bonus;
      }
    }
  return cost;
}

static int if_cnstreuse(Node a, int cost, int bonus)
{
  /* Gives a bonus to a load-constant-into-ac opcode when there is
     evidence that this constant will be used in the following
     instruction */
  extern Node head; /* declared in gen.c */
  Node h;
  /* Find ourselves in forest */
  for (h=head; h; h=h->link)
    if (generic(h->op) == ASGN && h->kids[1])
      if (h->kids[1] == a || h->kids[1]->kids[1] && h->kids[1] == a)
        break;
  if (! h) {
    return cost;
  } else if (h->kids[1]->syms[RX] && h->kids[1]->syms[RX]->temporary
             && h->kids[1]->syms[RX]->generated
             && h->kids[1]->syms[RX]->u.t.cse == h->kids[1]) {
    /* This is a constant subexpression. */
    Node trees[2];
    peektrees(h, 2, trees);
    if (simplecnstuse(trees[0], h->kids[1]->syms[RX]) &&
        simplecnstuse(trees[1], h->kids[1]->syms[RX]) )
      return cost - bonus;
  } else {
    Node trees[1];
    peektrees(h, 1, trees);
    if (simplecnstuse(trees[0], a->syms[0]))
      return cost - bonus;
  }
  return cost;
}

static int if_zoffset(Node a, int c1, int c2)
{
  /* Because framesize is not known until much later, we guess it on
     the basis of current offsets in order to give a cost to _LDLW and
     _STLW which can use an actual LDLW and STLW or use the more
     expensive LDXW and STXW. */
  int op;
  Symbol s;
  if (a && a->kids[0] && (op = a->kids[0]->op)
      && (generic(op) == ADDRL || generic(op) == ADDRF)
      && (s = a->kids[0]->syms[0]) )
    {
      int mao = (argoffset > maxargoffset) ? argoffset : maxargoffset;
      int mo = (offset > maxoffset) ? offset : maxoffset;
      int guess = s->x.offset + (mo|3) + (mao|3) + 18;
      return (guess >= 0 && guess < 256) ? c1 : c2;
    }
  return LBURG_MAX;
}

static int if_rmw(Node a, int cost)
{
  return if_rmw_a(a, 0, cost);
}

static int if_rmw_a(Node a, int arg, int cost)
{
  Node r;
  assert(a);
  assert(generic(a->op) == ASGN);
  assert(a->kids[0]);
  assert(a->kids[1]);
  r = a->kids[1];
  if (arg != 0 && r->kids[arg])
    r = r->kids[arg];
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

static int if_rmw_incr(Node a, int cost, int bonus)
{
  cost = if_rmw(a, cost);
  if (cost < LBURG_MAX && (a = a->kids[1]))
    if (generic(a->op) == ADD || generic(a->op) == SUB)
      return if_incr(a, cost, bonus);
  return cost;
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

/* Utilities for the emitter state machine that conservatively tracks
   which registers or accumulators are equal and to what. */

static Symbol make_derived_symbol(const char *what, Symbol s)
{
  /* We masquerade frame offsets and memory values as string
     constants. Genuine string constants at this stage only appear as
     static identifiers. */
  Value v;
  Symbol r;
  v.p = stringf("%s\003%s", what, s->name);
  r = constant(array(chartype,strlen(v.p),0), v);
  return r;
}

static Symbol get_cnst_or_reg(Node p, int nt, int starred)
{
  if (p)
    {
      p = reuse(p, nt);
      if (generic(p->op) == CNST || generic(p->op) == ADDRG)
        return (starred) ? make_derived_symbol("*", p->syms[0]) : p->syms[0];
      if (p->syms[0] && (generic(p->op) == ADDRL || generic(p->op) == ADDRF))
        return make_derived_symbol((starred) ? "*%" : "%", p->syms[0]);
      if (generic(p->op) == INDIR && specific(p->kids[0]->op) == VREG+P && !starred)
        return (p->x.inst && p->syms[RX]) ? p->syms[RX] : p->kids[0]->syms[0];
    }
  return 0;
}

static Symbol get_source_sym(Node p, int nt, Node *kids, const short *nts, const char *tpl)
{
  const char *etpl;
  for (; *tpl; tpl++)
    if (tpl[0]=='%' && tpl[1]=='{' && tpl[2]=='=')
      break;
  for (etpl = tpl; *etpl; etpl++)
    if (*etpl=='}')
      break;
  if (*tpl && *etpl) {
    const char *s;
    if (tpl[3]=='%' && tpl[4]>='0' && tpl[4]<='9' && tpl[5]=='}')
      return get_cnst_or_reg(kids[tpl[4]-'0'], nts[tpl[4]-'0'], 0);
    s = stringn(tpl+3,etpl-tpl-3);
    if (s == ireg[31]->x.name /* vAC */)
      return ireg[31];
    if (s == lreg[31]->x.name /* LAC */)
      return lreg[31];
    if (s == freg[31]->x.name /* FAC */)
      return freg[31];
    assert(0);
  }
  return 0;
}

static Symbol get_target_reg(Node p, int nt)
{
  int op;
  switch (nt) {
  case _lac_NT:
    return lreg[31];
  case _fac_NT:
    return freg[31];
  case _eac0_NT: case _ac0_NT: case _eac_NT: case _ac_NT:
    return ireg[31];
  case _reg_NT: case _regx_NT:
    return (p) ? p->syms[RX] : 0;
  case _rmw_NT:
    return (p && specific(p->op) == VREG+P) ? p->syms[0] : 0;
  case _asgn_NT:
    if (p && generic(p->op) == ASGN && p->kids[0] && (op = p->kids[0]->op)
        && isaddrop(op) && p->kids[0]->syms[0]) {
      if (generic(op) == ADDRG)
        return make_derived_symbol("*", p->kids[0]->syms[0]);
      else if (generic(op) == ADDRL || generic(op) == ADDRF)
        return make_derived_symbol("*%", p->kids[0]->syms[0]);
    }
  default:
    return 0;
  }
}


/* lcc callback: finalizer */
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

/* lcc callback: initializer */
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
    else if (!strcmp(argv[i],"-cpu=6")) {
      cpu = 6; /* TBD */
    } else if (!strcmp(argv[i],"-cpu=7"))
      cpu = 7; /* TBD */
    else if (!strncmp(argv[i],"-cpu=",5))
      warning("invalid cpu %s\n", argv[i]+5);
  /* Print header */
  print("#VCPUv%d\n\n",cpu);
  /* Prepare registers */
  for (i=0; i<24; i++)
    ireg[i] = mkreg("R%d", i, 1, IREG);
  for (i=0; i+1<24; i++)
    lreg[i] = mkreg("L%d", i, 3, IREG);
  for (i=0; i+2<24; i++)
    freg[i] = mkreg("F%d", i, 7, IREG);
  /* vAC/LAC/FAC */
  ireg[30] = mkreg("SP",  26, 1, IREG);
  ireg[31] = mkreg("vAC", 27, 1, IREG);
  freg[31] = mkreg("FAC", 28, 7, IREG);
  lreg[31] = mkreg("LAC", 29, 3, IREG);
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

/* Return register set for op */
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

/* Return register for argument passing or zero. */
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

/* lcc callback: provide explicit register targets */
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

/* lcc callback: mark caller-saved registers as clobbered. */
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

/* new lcc callback: preralloc is called before the normal register
   allocation pass and tries to eliminate temporaries when the
   instruction layout allows us to use an accumulator. This uses the
   same annotations as the emitter state machine. */

static void preralloc_scan(Node p, int nt, Symbol sym, int frag,
                           int *usecount, int *rclobbered)
{
  if ((p = reuse(p, nt))) {
    int rulenum = (*IR->x._rule)(p->x.state, nt);
    const short *nts = IR->x._nts[rulenum];
    const char *template = IR->x._templates[rulenum];
    const char *tpl = template;
    Node kids[10];
    Symbol t,s;
    (*IR->x._kids)(p, rulenum, kids);
    /* Scan template */
    if (*tpl == '#')
      return;
    while(--frag >= 0)
      while (*tpl && *tpl++ != '|')
        { }
    for(; *tpl && *tpl!='|' && !*rclobbered; tpl++)
      {
        if (tpl[0]=='%' && tpl[1]=='{' && tpl[2]=='!') {
          for(tpl=tpl+3; *tpl && *tpl!='|' && *tpl!='}'; tpl++)
            switch(*tpl) {
            case '4': if (cpu > 4) break;
            case '5': if (cpu > 5) break;
            case 'A': vac_clobbered = 1; break;
            case 'L': case 'F': xac_clobbered = vac_clobbered = 1; break;
            default: break;
            }
        } else if (tpl[0]=='%' && isdigit(tpl[1])) { /* %0 */
          Node k = kids[tpl[1]-'0'];
          int knt = nts[tpl[1]-'0'];
          k = reuse(k, knt);
          if (knt == _regx_NT && k->x.inst && k->syms[RX] == sym)
            *rclobbered = 1; /* a regx nonterminal kills preralloc */
          else if (k->x.inst != knt)
            preralloc_scan(k, knt, sym, 0, usecount, rclobbered);
          else if (k->syms[RX] == sym)
            *usecount -= 1;
          tpl += 1;
        } else if (tpl[0]=='%' && tpl[1]=='['        /* %[0b] */
                   && isdigit(tpl[2]) && islower(tpl[3])  && tpl[4]==']') {
          Node k = kids[tpl[2]-'0'];
          int knt = nts[tpl[2]-'0'];
          k = reuse(k, knt);
          if (k->x.inst != knt)
            preralloc_scan(k, knt, sym, tpl[3]-'a', usecount, rclobbered);
          tpl += 3;
        } else if (tpl[0]=='%'&&                    /* %{?...::} assumed != */
                   tpl[1]=='{' && tpl[2]=='?') {
          int s = 2;
          for (tpl=tpl+3; *tpl && *tpl!='}' && *tpl!='|'; tpl++)
            if (*tpl == ':' && !--s)
              break;
        } else if (tpl[0]=='%' && tpl[1]=='{') {    /* %{...} skipped */
          for (tpl=tpl+2; *tpl && *tpl!='}' && *tpl!='|'; tpl++) /**/;
        }
      }
    /* Process non-terminal clobber */
    t = get_target_reg(p, nt);
    s = get_source_sym(p, nt, kids, nts, template);
    if (s != sym) {
      if (t == ireg[31])
        vac_clobbered = 1;
      else if (t == lreg[31] || t == freg[31])
        vac_clobbered = xac_clobbered = 1;
    }
  }
}

static void preralloc(Node p)
{
  if (p->x.inst == _reg_NT)
    {
      Node q;
      int usecount = -1;
      Symbol sym = p->syms[RX];
      if (sym->temporary)
        for (q = sym->x.lastuse; q; q = q->x.prevuse)
          usecount += 1;
      if (usecount > 0)
        {
          /* Determine accumulator */
          Symbol r = ireg[31];
          int *rclobbered = &xac_clobbered;
          if (optype(p->op) == FLOAT)
            r = freg[31];
          else if (opsize(p->op) == 4)
            r = lreg[31];
          if (r == ireg[31])
            rclobbered = &vac_clobbered;
          /* Hack because moves to/from FAC are costly on cpu<7 */
          if (r == freg[31] && usecount > 1 && cpu < 7)
            return;
          /* Search for references to sym until accumulator clobbered */
          *rclobbered = 0;
          for (q=p->x.next; q && usecount>0 && !*rclobbered; q = q->x.next)
            preralloc_scan(q, q->x.inst, sym, 0, &usecount, rclobbered);
          /* Did we find all uses of sym? */
          if (! usecount) {
            /* Optimize temporary sym out of existence */
            r->x.lastuse = sym->x.lastuse;
            for (q = sym->x.lastuse; q; q = q->x.prevuse) {
              q->syms[RX] = r;
              q->x.registered = 1;
            }
          }
        }
    }
}

/* lcc-callback: before calling the lcc callback gen(), the pregen
   pass rearraanges some trees in the forest to improve the code
   generation. */

static Node pregen(Node forest)
{
  Node p;
  /* Reorganize to make rmw instruction more obvious */
  for(p=forest; p; p=p->link) {
    Node q = p->link;
    if (generic(p->op) == ASGN && p->kids[0] &&
        q && generic(q->op) == ASGN && q->kids[0] ) 
      {
        /* Change ASGN(v,...(u,...));ASGN(u,v)
           into ASGN(u,...(u,...));ASGN(v,u)   */
        Node v = p->kids[0];
        Node u = p->kids[1];
        while(u && !isaddrop(u->op))
          u = u->kids[0];
        if (generic(v->op) == ADDRL &&
            v->syms[0] && v->syms[0]->temporary &&
            sametree(u, q->kids[0]) )
          {
            Node w = q->kids[1];
            if (w && generic(w->op) == LOAD)
              w = w->kids[0];
            if (w && generic(w->op) == INDIR &&
                (w = w->kids[0]) && sametree(v,w) )
              {
                /* Match! */
                q->kids[0]->op = v->op;
                q->kids[0]->syms[0] = v->syms[0];
                q->kids[0]->syms[0]->u.t.cse = q->kids[1];
                v->op = w->op = u->op;
                v->syms[0] = w->syms[0] = u->syms[0];
              }
          }
      }
  }
  /* Continue with gen */
  return gen(forest);
}


/* Enhanced emitter than understands additional %{..} template
   constructs and contains a state machine that conservatively keeps
   track of equality across registers. */

static void emitfmt2(const char *template, int len,
                     Node p, int nt, Node *kids, short *nts)
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
  const char *fmt = template;
  static int alt_s;
  int s = alt_s;
  alt_s = 0;
  for (; len > 0 && *fmt; fmt++, len--)
    if (*fmt == '|' && s == 0)
      break;
    else if (*fmt == '|')
      s -= 1;
    else if (s > 0)
      continue;
    else if (*fmt != '%' || len <= 1)
      (void)putchar(*fmt);
    else if (--len && *++fmt == 'F')                          /* %F */
      print("%d", framesize);
    else if (*fmt >= 'a' && *fmt < 'a' + NELEMS(p->syms))     /* %a..%c */
      fputs(p->syms[*fmt - 'a']->x.name, stdout);
    else if (*fmt >= '0' && *fmt <= '9')                      /* %0..%9 */
      emitasm(kids[*fmt - '0'], nts[*fmt - '0']);
    else if (len>3 && fmt[0] == '[' && fmt[3] == ']'
             && fmt[1] >= '0' && fmt[1] <= '9'
             && fmt[2] >= 'a' && fmt[2] <= 'z') {      /* %[0a] */
      alt_s = fmt[2] - 'a';
      emitasm(kids[fmt[1] - '0'], nts[fmt[1] - '0']);
      fmt += 3;
      len -= 3;
      alt_s = 0;
    } else if (*fmt == '{') {
      int i;
      int level = 0;
      const char *s;
      for (i=0; i < len; i++)
        if (fmt[i] == '{')
          level += 1;
        else if (fmt[i] == '}' && !--level)
          break;
      assert(!level);
      emit3(fmt+1, i-1, p, nt, kids, nts);
      fmt += i;
      len -= i;
    } else
      (void)putchar(*fmt);

  /* This part of the state machine fires when on reaches the
     end of the template and updates the equality tracking information
     according to the nonterminal of the current rule and to the %{=...}
     annotations found in the template. */
  if (!fmt[len]) {
    Symbol t = get_target_reg(p, nt);
    Symbol s = get_source_sym(p, nt, kids, nts, template);
    if (nt == _asgn_NT) {
      vac_memval = lac_memval = fac_memval = 0;
      if (s == ireg[31])
        { vac_memval = t; }
      if (s == lreg[31])
        { lac_memval = t; fac_memval = 0; }
      if (s == freg[31])
        { lac_memval = 0; fac_memval = t; }
    } else if (t == ireg[31] /*vAC*/) {
      vac_clobbered = 1;
      vac_equiv = 0;
      vac_constval = vac_memval = 0;
      if (s && !s->x.regnode)
        vac_constval = s;
      else if (s && s->x.regnode && s->x.regnode->number < 24)
        vac_equiv = (1 << s->x.regnode->number);
    } else if (t == lreg[31] /*LAC*/) {
      vac_clobbered = xac_clobbered = 1;
      vac_equiv = lac_equiv = fac_equiv = 0;
      vac_constval = vac_memval = lac_memval = fac_memval = 0;
      if (s && s->x.regnode && s->x.regnode->number < 24)
        lac_equiv = (1 << s->x.regnode->number);
    } else if (t == freg[31] /*FAC*/) {
      vac_clobbered = xac_clobbered = 1;
      vac_equiv = lac_equiv = fac_equiv = 0;
      vac_constval = vac_memval = lac_memval = fac_memval = 0;
      if (s && s->x.regnode && s->x.regnode->number < 24)
        fac_equiv = (1 << s->x.regnode->number);
    } else if (t && t->x.regnode) {
      if (s == ireg[31] && t->x.regnode->number < 24)
        vac_equiv |= (1 << t->x.regnode->number);
      else if (s == lreg[31] && t->x.regnode->number < 24)
        lac_equiv |= (1 << t->x.regnode->number);
      else if (s == freg[31] && t->x.regnode->number < 24)
        fac_equiv |= (1 << t->x.regnode->number);
      else {
        vac_equiv &= ~t->x.regnode->mask;
        lac_equiv &= ~t->x.regnode->mask;
        fac_equiv &= ~t->x.regnode->mask;
      }
    }
  }
}

static void emitfmt1(const char *fmt, Node p, int nt, Node *kids, short *nts)
{
  emitfmt2(fmt, strlen(fmt), p, nt, kids, nts);
}

static void emit3(const char *fmt, int len, Node p, int nt, Node *kids, short *nts)
{
  int i = 0;
  /* Annotations %{!xxx} update the register equivalence
     information gleaned by the emitter state machine. */
  if (len > 0 && fmt[0] == '!')
    {
      for (int i=1; i<len; i++)
        switch(fmt[i]) {
        case '4': if (cpu <= 4) goto xvac; break;
        case '5': if (cpu <= 5) goto xvac; break;
        case 'A': goto xvac;
        case 'L': case 'F':
          xac_clobbered = 1;
          lac_equiv = fac_equiv = 0;
          lac_memval = fac_memval = 0;
        xvac:
          vac_clobbered = 1;
          vac_constval = vac_memval = 0;
          vac_equiv = 0;
        default:
          break;
        }
      return;
    }
  /* Annotations %{=xxx} are processed elsewhere */
  if (len > 0 && fmt[0] == '=')
    return;
  /* %{?[0-9a-c]==...:ifeq:ifne} */
  /* %{?[0-9a-c]=~xAC:ifeq:ifne} */
  if (len > 3 && fmt[0] == '?')
    {
      int starred = 0;
      if (fmt[1] == '*')
        {
          starred = 1;
          fmt += 1;
          len -= 1;
        }
      if (fmt[1] == '%')
        {
          fmt += 1;
          len -= 1;
        }
      if (fmt[2] == '=' && (fmt[3] == '=' || fmt[3] == '~'))
        {
          int ifeq, ifne, level = 0;
          for (ifeq=4; ifeq<len; ifeq++)
            if (fmt[ifeq] == '{')
              level++;
            else if (fmt[ifeq] == '}')
              level--;
            else if (fmt[ifeq] == ':' && !level)
              break;
          for (ifne=ifeq+1; ifne<len; ifne++)
            if (fmt[ifne] == '{')
              level++;
            else if (fmt[ifne] == '}')
              level--;
            else if (fmt[ifne] == ':' && !level)
              break;
          if (ifeq < len)
            {
              int eq = 0;
              Symbol sym = 0;
              const char *cmp = stringn(fmt+4,ifeq-4);
              if (fmt[1] >= 'a' && fmt[1] <= 'c' && !starred)
                sym = p->syms[fmt[1]-'a'];
              else if (fmt[1] >= '0' && fmt[1] <= '9')
                sym = get_cnst_or_reg(kids[fmt[1]-'0'], nts[fmt[1]-'0'], starred);
              if (sym && sym->x.name == cmp)
                eq = 1;
              else if (fmt[3] == '=')
                eq = 0; /* literal comparison */
              else if (sym && cmp == ireg[31]->x.name && !sym->x.regnode )
                eq = (samecnst(vac_constval,sym) || vac_memval == sym);
              else if (sym && cmp == lreg[31]->x.name && !sym->x.regnode )
                eq = (lac_memval == sym);
              else if (sym && cmp == freg[31]->x.name && !sym->x.regnode )
                eq = (fac_memval == sym);
              else if (sym && cmp == ireg[31]->x.name /* vAC */
                       && sym->x.regnode && sym->x.regnode->number < 24
                       && (vac_equiv & (1 << sym->x.regnode->number)) )
                eq = 1;
              else if (sym && cmp == lreg[31]->x.name /* LAC */
                       && sym->x.regnode && sym->x.regnode->number < 24
                       && (lac_equiv & (1 << sym->x.regnode->number)) )
                eq = 1;
              else if (sym && cmp == freg[31]->x.name /* FAC */
                       && sym->x.regnode && sym->x.regnode->number < 24
                       && (fac_equiv & (1 << sym->x.regnode->number)) )
                eq = 1;
              if (eq)
                emitfmt2(fmt+ifeq+1, ifne-ifeq-1, p, nt, kids, nts);
              else if (ifne < len)
                emitfmt2(fmt+ifne+1, len-ifne-1, p, nt, kids, nts);
              return;
            }
        }
    }
  /* %{mulC[%R]} -- multiplication by a small constant */
  else if (len >= 4 && !strncmp(fmt, "mul", 3) && fmt[3] >= '0' && fmt[3] <= '9')
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
      if (len > 4) {
        assert(fmt[4]=='%' && fmt[5]>='0' && fmt[5]<='9' && len==6);
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
        if (len > 4 && c < 0)
          print("LDI(0);SUBW(%s);", r);
        else if (len > 4)
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


/* placement constraints and other attributes */
struct constraints {
  char near_p, place_p, org_p, nohop_p;
  unsigned int amin, amax, aorg;
};

static int check_uintval(Attribute a, int n)
{
  Symbol c = a->args[n];
  if (c && c->scope == CONSTANTS && isint(c->type)) {
    unsigned long u = c->u.c.v.u;
    if (u != (u & 0xffff))
      warning("attribute `%s`: argument out of range\n", a->name);
    return 1;
  }
  return 0;
}

static unsigned int uintval(Symbol c)
{
  if (c && c->scope == CONSTANTS && isint(c->type))
    return (unsigned int) c->u.c.v.u;
  return 0;
}

static int check_attributes(Symbol p)
{
  Attribute a;
  char is_weak = 0;
  char has_org = 0;
  char has_place = 0;
  char is_extern = (p->sclass == EXTERN);
  if ((p->scope == GLOBAL) || is_extern) {
    for (a = p->attr; a; a = a->link) {
      char yes = 0;
      if (a->name == string("place") && !is_extern) {
        if (has_org)
          error("incompatible placement constraints (org & place)\n");
        a->okay = (check_uintval(a,0) &&  check_uintval(a,1));
        yes = has_place = 1;
      } else if (a->name == string("org")) {
        if (has_org)
          error("incompatible placement constraints (multiple org)\n");
        else if (has_place)
          error("incompatible placement constraints (org & place)\n");
        a->okay = (check_uintval(a,0) && !a->args[1]);
        yes = has_org = 1;
      } else if (a->name == string("nohop") && !is_extern) {
        a->okay = (!a->args[0] && !a->args[1]);
        yes = 1;
      } else if (a->name == string("weak") && is_extern) {
        is_weak = a->okay = (!a->args[0] && !a->args[1]);
        yes = 1;
      }      
      if (yes && !a->okay)
        error("illegal argument in `%s` attribute\n", a->name);
    }
  }
  return is_weak;
}

static void get_constraints(Symbol p, struct constraints *c)
{
  Attribute a;

  c->place_p = c->org_p = c->nohop_p = 0;
  c->amin = c->amax = c->aorg = 0;
  c->near_p = (fnqual(p->type) == NEAR);
  for (a = p->attr; a; a = a->link) {
    if (!a->okay) {
      continue;
    } else if (a->name == string("nohop")) {
      c->nohop_p = 1;
    } else if (a->name == string("org")) {
      c->aorg = uintval(a->args[0]);
      c->org_p = 1;
    } else if (a->name == string("place")) {
      unsigned long a0 = uintval(a->args[0]);
      unsigned long a1 = uintval(a->args[1]);
      if (c->place_p) {
        if (a0 < c->amin) a0 = c->amin;
        if (a1 > c->amax) a1 = c->amax;
      }
      c->place_p = 1;
      c->amin = a0;
      c->amax = a1;
    }
  }
  if (c->near_p) {
    if (!c->place_p)
      c->amin = 0;
    if (!c->place_p || c->amax > 0xff)
      c->amax = 0xff;
  }
  if ((c->near_p || c->place_p) && (c->amin > c->amax))
    warning("unsatisfyable placement constraints (place)\n");
}

static void print_constraints(Symbol p, struct constraints *c)
{
  if (c->nohop_p)
    lprint("('NOHOP', %s)", p->x.name);
  if (c->org_p)
    lprint("('ORG', %s, 0x%x)", p->x.name, c->aorg);
  if (c->near_p || c->place_p)
    lprint("('PLACE', %s, 0x%x, 0x%x)", p->x.name, c->amin, c->amax);
}


/* lcc callback: annotates arg nodes with offset and register info. */
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

/* lcc callback: place local variable. */
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

/* Utility for printing function prologues and epilogues. */
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

/* lcc callback: compile a function. */
static void function(Symbol f, Symbol caller[], Symbol callee[], int ncalls)
{
  /* Stack frame: 
   |
   |   SP+Framesize--> n bytes                   : arguments
   |                   maxoffset bytes           : local variables
   |                   0 to 2 bytes              : alignment pad
   |                   2 bytes                   : saved vLR
   |                   sizesave bytes            : saved registers
   |             SP--> maxargoffset bytes        : argument building area
   */

  int i, roffset, sizesave, ty;
  unsigned savemask;
  char frameless;
  struct constraints place;
  Symbol r;

  usedmask[0] = usedmask[1] = 0;
  freemask[0] = freemask[1] = ~(unsigned)0;
  offset = maxoffset = 0;
  maxargoffset = 0;
  assert(f->type && f->type->type);
  ty = ttob(f->type->type);
  if (ncalls) {
    tmask[IREG] = REGMASK_TEMPS;
    vmask[IREG] = REGMASK_SAVED;
  } else {
    tmask[IREG] = REGMASK_TEMPS;
    vmask[IREG] = REGMASK_MOREVARS;
  }
  /* placement constraints */
  get_constraints(f, &place);
  /* locate incoming arguments */
  offset = 0;
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
  framesize = maxargoffset + sizesave + maxoffset + 2;
  assert(framesize >= 2);
  if (framesize > 32768)
    error("%s() framesize (%d) too large for a gigatron\n",
          f->name, framesize);
  /* can we make a frameless leaf function */
  if (ncalls == 0 && framesize == 2 && (tmask[IREG] & ~usedmask[IREG])) {
    framesize = (cpu < 7) ? 0 : 2;  /* are SP and vSP the same? */
    frameless = 1;
  } else if (IR->longmetric.align == 4) {
    framesize = (framesize + 3) & ~0x3;
    frameless = 0;
  }
  /* prologue */
  xprint_init();
  segment(CODE);
  lprint("('%s', %s, code%d)", segname(), f->x.name, codenum);
  print_constraints(f, &place);
  print("# ======== %s\n", lhead.prev->s);
  print("def code%d():\n", codenum++);
  print("\tlabel(%s);\n", f->x.name);
  if (frameless) {
    print("\tPUSH();\n");
  } else {
    print("\t_PROLOGUE(%d,%d,0x%x); # save=", framesize, maxargoffset, savemask);
    printregmask(savemask);
    print("\n");
  }
  /* Emit actual code */
  vac_constval = 0;
  vac_equiv = lac_equiv = fac_equiv = 0;
  emitcode();
  /* Epilogue */
  if (frameless) {
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

/* lcc callback. Emit a constant. */
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
      xprint("\tbytes(%d);", x&0xff);
    else if (size == 2)
      xprint("\twords(%d);", x&0xffff);
    else if (size == 4)
      xprint("\twords(%d,%d);", x&0xffff, (x>>16)&0xffff);
    if (suffix == I)
      xprint(" # %D", (long)x);
    else if (suffix == U)
      xprint(" # %U", (unsigned long)x);
    xprint("\n");
  }
}

/* lcc callback - emit an address constant. */
static void defaddress(Symbol p)
{
  xprint("\twords(%s);\n", p->x.name);
}

/* lcc callback - emit a string constant. */
static void defstring(int n, char *str)
{
  int i;
  for (i=0; i<n; i++)
    xprint( ((i&7)==0) ? "\tbytes(%d" : ((i&7)==7) ? ",%d);\n" : ",%d", (int)str[i]&0xff );
  if (i&7)
    xprint(");\n");
}

/* lcc callback - mark imported symbol. */
static void import(Symbol p)
{
  if (p->ref > 0 && strncmp(p->x.name, "'__glink_weak_", 14) != 0) {
    Attribute a;
    for (a = p->attr; a; a = a->link)
      if (a && a->okay && a->name == string("org"))
        break;
    if (a)
      lprint("('IMPORT', %s, 'AT', 0x%x)", p->x.name, uintval(a->args[0]));
    else
      lprint("('IMPORT', %s)", p->x.name);
  }
}

/* lcc callback - mark exported symbol. */
static void export(Symbol p)
{
  int isnear = fnqual(p->type) == NEAR;
  int iscommon = (p->u.seg == BSS && !isnear && !p->attr);
  if (! iscommon)
    lprint("('EXPORT', %s)", p->x.name);
}

/* lcc callback: determine symbol names in assembly code. */
static void defsymbol(Symbol p)
{
  /* this is the time to check that attributes are meaningful */
  int is_weak = check_attributes(p);
  if (p->scope >= LOCAL && p->sclass == STATIC)
    p->x.name = stringf("'.%d'", genlabel(1));
  else if (p->generated)
    p->x.name = stringf("'.%s'", p->name);
  else if (p->sclass == EXTERN && is_weak)
    p->x.name = stringf("'__glink_weak_%s'", p->name);
  else if (p->scope == GLOBAL || p->sclass == EXTERN)
    p->x.name = stringf("'%s'", p->name);
  else
    p->x.name = p->name;
}

/* lcc callback: construct address+offset symbols. */
static void address(Symbol q, Symbol p, long n)
{
  if (p->scope == GLOBAL || p->sclass == STATIC || p->sclass == EXTERN) {
    Type ty0 = p->type;
    Type ty1 = q->type;
    q->x.name = stringf("v(%s)%s%D", p->x.name, n >= 0 ? "+" : "", n);
    if (ty0 && fnqual(ty0) == NEAR && n >= 0 && n < ty0->size)
      if (ty1 && !fnqual(ty1))
        q->type = qual(NEAR, ty1);
  } else {
    assert(n <= INT_MAX && n >= INT_MIN);
    q->x.offset = p->x.offset + n;
    q->x.name = stringd(q->x.offset);
  }
}

/* lcc callback: construct global variable. */
static void global(Symbol p)
{
  struct constraints place;
  unsigned int size, align;
  const char *s = segname();
  const char *n;

  get_constraints(p, &place);
  size = p->type->size;
  align = (place.near_p) ? 1 : p->type->align;
  if (p->u.seg == BSS && p->sclass != STATIC && !place.near_p && !p->attr)
    s = "COMMON";               /* no 'common' in the presence of placement attributes */
  if (p->u.seg == LIT)
    size = 0; /* unreliable in switch tables */
  lprint("('%s', %s, code%d, %d, %d)",
          s, p->x.name, codenum, size, align);
  n = lhead.prev->s;
  print_constraints(p, &place);
  xprint("# ======== %s\n", n);
  xprint("def code%d():\n", codenum++);
  if (align > 1)
    xprint("\talign(%d);\n", align);
  xprint("\tlabel(%s);\n", p->x.name);
  if (p->u.seg == BSS)
    xprint("\tspace(%d);\n", p->type->size);
}

/* lcc callback: define current segment. */
static void segment(int n)
{
  cseg = n;
}

/* lcc callback: emit assembly to skip space. */
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
        pregen,
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
          emitfmt1,
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
