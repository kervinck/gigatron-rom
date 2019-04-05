%{
#include "c.h"

#define NODEPTR_TYPE Node
#define OP_LABEL(p) ((p)->op)
#define LEFT_CHILD(p) ((p)->kids[0])
#define RIGHT_CHILD(p) ((p)->kids[1])
#define STATE_LABEL(p) ((p)->x.state)
static void address(Symbol, Symbol, long);
static void blkfetch(int, int, int, int);
static void blkloop(int, int, int, int, int, int[]);
static void blkstore(int, int, int, int);
static void defaddress(Symbol);
static void defconst(int, int, Value);
static void defstring(int, char *);
static void defsymbol(Symbol);
static void doarg(Node);
static void emit2(Node);
static void export(Symbol);
static void clobber(Node);
static void function(Symbol, Symbol [], Symbol [], int);
static void global(Symbol);
static void import(Symbol);
static void local(Symbol);
static void progbeg(int, char **);
static void progend(void);
static int rangep(Node, unsigned, unsigned);
static void segment(int);
static void space(int);
static void target(Node);

static void inst_spill(Node);
static void inst_copy(Node);
static void inst_ldloc(Node);
static void inst_ld(Node);
static void inst_st(Node);
static void inst_peek(Node);
static void inst_poke(Node);
static void inst_scon(Node);
static void inst_addr(Node);
static void inst_cnstw(Node);
static void inst_addi(Node p);
static void inst_subi(Node p);
static void inst_andi(Node p);
static void inst_ori(Node p);
static void inst_xori(Node p);
static void inst_addw(Node p);
static void inst_subw(Node p);
static void inst_andw(Node p);
static void inst_orw(Node p);
static void inst_xorw(Node p);
static void inst_lslw(Node p);
static void inst_lsh(Node p);
static void inst_rsh(Node p);
static void inst_mul(Node p);
static void inst_mod(Node p);
static void inst_div(Node p);
static void inst_neg(Node p);
static void inst_bcom(Node p);
static void inst_call(Node p);
static void inst_sys(Node p);

static Symbol regs[32];
static Symbol regw;
static struct {
	int known;
	unsigned value;
} vac;
%}

%start stmt
%term CNSTF2=2065
%term CNSTI1=1045
%term CNSTI2=2069
%term CNSTP2=2071
%term CNSTU1=1046
%term CNSTU2=2070

%term ARGB=41
%term ARGF2=2081
%term ARGI2=2085
%term ARGP2=2087
%term ARGU2=2086

%term ASGNB=57
%term ASGNF2=2097
%term ASGNI1=1077
%term ASGNI2=2101
%term ASGNP2=2103
%term ASGNU1=1078
%term ASGNU2=2102

%term INDIRB=73
%term INDIRF2=2113
%term INDIRI1=1093
%term INDIRI2=2117
%term INDIRP2=2119
%term INDIRU1=1094
%term INDIRU2=2118

%term CVFF2=2161
%term CVFI2=2165

%term CVIF2=2177
%term CVII1=1157
%term CVII2=2181
%term CVIU1=1158
%term CVIU2=2182

%term CVPU2=2198

%term CVUI1=1205
%term CVUI2=2229
%term CVUP2=2231
%term CVUU1=1206
%term CVUU2=2230

%term NEGF2=2241
%term NEGI2=2245

%term CALLB=217
%term CALLF2=2257
%term CALLI2=2261
%term CALLP2=2263
%term CALLU2=2262
%term CALLV=216

%term RETF2=2289
%term RETI2=2293
%term RETP2=2295
%term RETU2=2294
%term RETV=248

%term ADDRGP2=2311

%term ADDRFP2=2327

%term ADDRLP2=2343

%term ADDF2=2353
%term ADDI2=2357
%term ADDP2=2359
%term ADDU2=2358

%term SUBF2=2369
%term SUBI2=2373
%term SUBP2=2375
%term SUBU2=2374

%term LSHI2=2389
%term LSHU2=2390

%term MODI2=2405
%term MODU2=2406

%term RSHI2=2421
%term RSHU2=2422

%term BANDI2=2437
%term BANDU2=2438

%term BCOMI2=2453
%term BCOMU2=2454

%term BORI2=2469
%term BORU2=2470

%term BXORI2=2485
%term BXORU2=2486

%term DIVF2=2497
%term DIVI2=2501
%term DIVU2=2502

%term MULF2=2513
%term MULI2=2517
%term MULU2=2518

%term EQF2=2529
%term EQI2=2533
%term EQU2=2534

%term GEF2=2545
%term GEI2=2549
%term GEU2=2550

%term GTF2=2561
%term GTI2=2565
%term GTU2=2566

%term LEF2=2577
%term LEI2=2581
%term LEU2=2582

%term LTF2=2593
%term LTI2=2597
%term LTU2=2598

%term NEF2=2609
%term NEI2=2613
%term NEU2=2614

%term JUMPV=584

%term LABELV=600

%term LOADB=233
%term LOADI1=1253
%term LOADI2=2277
%term LOADU1=1254
%term LOADU2=2278

%term VREGP=711

%term SYSI2=2661

%%
reg: INDIRI1(VREGP)       "# read register\n"
reg: INDIRU1(VREGP)       "# read register\n"

reg: INDIRI2(VREGP)       "# read register\n"
reg: INDIRU2(VREGP)       "# read register\n"
reg: INDIRP2(VREGP)       "# read register\n"

stmt: ASGNI1(VREGP, reg)  "# write register\n"
stmt: ASGNU1(VREGP, reg)  "# write register\n"

stmt: ASGNI2(VREGP, reg)  "# write register\n"
stmt: ASGNU2(VREGP, reg)  "# write register\n"
stmt: ASGNP2(VREGP, reg)  "# write register\n"

stmt: ASGNI2(ADDRLP2, LOADI2(INDIRI2(VREGP))) `inst_spill` 1
stmt: ASGNU2(ADDRLP2, LOADU2(INDIRU2(VREGP))) `inst_spill` 1

stmt: reg "# "

con1: CNSTI2 "1" range(a, 1, 1)
con1: CNSTU2 "1" range(a, 1, 1)

scon: CNSTI2 "%a" range(a, 0, 127)
scon: CNSTU2 "%a" range(a, 0, 255)
scon: CNSTP2 "%a" rangep(a, 0, 255)

reg: LOADI1(reg)  `inst_copy` 1
reg: LOADI2(reg)  `inst_copy` 1
reg: LOADU1(reg)  `inst_copy` 1
reg: LOADU2(reg)  `inst_copy` 1

reg: INDIRI1(ADDRLP2) `inst_ldloc` 1
reg: INDIRI2(ADDRLP2) `inst_ldloc` 1
reg: INDIRU1(ADDRLP2) `inst_ldloc` 1
reg: INDIRU2(ADDRLP2) `inst_ldloc` 1
reg: INDIRP2(ADDRLP2) `inst_ldloc` 1
reg: INDIRF2(ADDRLP2) `inst_ldloc` 1

reg: INDIRI1(ADDRFP2) `inst_ldloc` 1
reg: INDIRI2(ADDRFP2) `inst_ldloc` 1
reg: INDIRU1(ADDRFP2) `inst_ldloc` 1
reg: INDIRU2(ADDRFP2) `inst_ldloc` 1
reg: INDIRP2(ADDRFP2) `inst_ldloc` 1
reg: INDIRF2(ADDRFP2) `inst_ldloc` 1

reg: INDIRI1(scon) `inst_ld` 1
reg: INDIRU1(scon) `inst_ld` 1
reg: INDIRI2(scon) `inst_ld` 1
reg: INDIRU2(scon) `inst_ld` 1
reg: INDIRP2(scon) `inst_ld` 1
reg: INDIRF2(scon) `inst_ld` 1
stmt: ASGNI1(scon, reg) `inst_st` 1
stmt: ASGNU1(scon, reg) `inst_st` 1
stmt: ASGNI2(scon, reg) `inst_st` 1
stmt: ASGNU2(scon, reg) `inst_st` 1
stmt: ASGNP2(scon, reg) `inst_st` 1
stmt: ASGNF2(scon, reg) `inst_st` 1

reg: INDIRI1(reg) `inst_peek` 1
reg: INDIRU1(reg) `inst_peek` 1
reg: INDIRI2(reg) `inst_peek` 1
reg: INDIRU2(reg) `inst_peek` 1
reg: INDIRP2(reg) `inst_peek` 1
reg: INDIRF2(reg) `inst_peek` 1
stmt: ASGNI1(reg, reg) `inst_poke` 1
stmt: ASGNU1(reg, reg) `inst_poke` 1
stmt: ASGNI2(reg, reg) `inst_poke` 1
stmt: ASGNU2(reg, reg) `inst_poke` 1
stmt: ASGNP2(reg, reg) `inst_poke` 1
stmt: ASGNF2(reg, reg) `inst_poke` 1

reg: scon    `inst_scon` 1
reg: CNSTI1  `inst_cnstw` 1
reg: CNSTU1  `inst_scon` 1
reg: CNSTI2  `inst_cnstw` 1
reg: CNSTU2  `inst_cnstw` 1
reg: CNSTP2  `inst_cnstw` 1
reg: CNSTF2  `inst_cnstw` 1
reg: ADDRLP2 `inst_addr` 1
reg: ADDRFP2 `inst_addr` 1
reg: ADDRGP2 `inst_addr` 1

reg: ADDI2(reg, scon)   `inst_addi` 1
reg: ADDU2(reg, scon)   `inst_addi` 1
reg: ADDP2(reg, scon)   `inst_addi` 1
reg: SUBI2(reg, scon)   `inst_subi` 1
reg: SUBU2(reg, scon)   `inst_subi` 1
reg: SUBP2(reg, scon)   `inst_subi` 1
reg: BANDI2(reg, scon)  `inst_andi` 1
reg: BANDU2(reg, scon)  `inst_andi` 1
reg: BORI2(reg, scon)   `inst_ori` 1
reg: BORU2(reg, scon)   `inst_ori` 1
reg: BXORI2(reg, scon)  `inst_xori` 1
reg: BXORU2(reg, scon)  `inst_xori` 1

reg: ADDI2(reg, reg)   `inst_addw` 1
reg: ADDU2(reg, reg)   `inst_addw` 1
reg: ADDP2(reg, reg)   `inst_addw` 1
reg: SUBI2(reg, reg)   `inst_subw` 1
reg: SUBU2(reg, reg)   `inst_subw` 1
reg: SUBP2(reg, reg)   `inst_subw` 1
reg: BANDI2(reg, reg)  `inst_andw` 1
reg: BANDU2(reg, reg)  `inst_andw` 1
reg: BORI2(reg, reg)   `inst_orw` 1
reg: BORU2(reg, reg)   `inst_orw` 1
reg: BXORI2(reg, reg)  `inst_xorw` 1
reg: BXORU2(reg, reg)  `inst_xorw` 1

reg: LSHI2(reg, con1)  `inst_lslw` 1
reg: LSHU2(reg, con1)  `inst_lslw` 1

reg: LSHI2(reg, reg)  `inst_lsh` 1
reg: LSHU2(reg, reg)  `inst_lsh` 1
reg: RSHI2(reg, reg)  `inst_rsh` 1
reg: RSHU2(reg, reg)  `inst_rsh` 1

reg: MULI2(reg, reg)   `inst_mul` 1
reg: MULU2(reg, reg)   `inst_mul` 1
reg: MODI2(reg, reg)   `inst_mod` 1
reg: MODU2(reg, reg)   `inst_mod` 1
reg: DIVI2(reg, reg)   `inst_div` 1
reg: DIVU2(reg, reg)   `inst_div` 1

reg: CVII1(reg)  "# andi $ff # fixme: sign-extend\n" 1
reg: CVII2(reg)  "# " 1
reg: CVIU1(reg)  "# andi $ff\n" 1
reg: CVIU2(reg)  "# " 1

reg: CVPU2(reg)  "# " 1

reg: CVUI1(reg)  "# andi $ff # fixme: sign-extend\n" 1
reg: CVUI2(reg)  "# " 1
reg: CVUP2(reg)  "# " 1
reg: CVUU1(reg)  "# andi $ff\n" 1
reg: CVUU2(reg)  "# " 1

reg: NEGI2(reg)  `inst_neg` 1

reg: BCOMI2(reg) `inst_bcom` 1
reg: BCOMU2(reg) `inst_bcom` 1

reg: CVFF2(reg)  "# " 1
reg: CVFI2(reg)  "# call cvfi\n" 1
reg: CVIF2(reg)  "# call cvif\n" 1
reg: NEGF2(reg)  "# call negf\n" 1

reg: ADDF2(reg, reg)     "# stw ha\nldw %1\ncall addf\n" 1
reg: SUBF2(reg, reg)     "# stw ha\nldw %1\ncall subf\n" 1
reg: MULF2(reg, reg)     "# stw ha\nldw %1\ncall mulf\n" 1
reg: DIVF2(reg, reg)     "# stw ha\nldw %1\ncall divf\n" 1

stmt: JUMPV(reg) "j\n" 1
stmt: LABELV     "%a:\n"
stmt: EQI2(reg, reg) "# jcc eq %1 %a # pseudo\n" 1
stmt: EQU2(reg, reg) "# jcc eq %1 %a # pseudo\n" 1
stmt: NEI2(reg, reg) "# jcc ne %1 %a # pseudo\n" 1
stmt: NEU2(reg, reg) "# jcc ne %1 %a # pseudo\n" 1
stmt: GEI2(reg, reg) "# jcc ge %1 %a # pseudo\n" 1
stmt: GEU2(reg, reg) "# jcc ge %1 %a # pseudo\n" 1
stmt: GTI2(reg, reg) "# jcc gt %1 %a # pseudo\n" 1
stmt: GTU2(reg, reg) "# jcc gt %1 %a # pseudo\n" 1
stmt: LEI2(reg, reg) "# jcc le %1 %a # pseudo\n" 1
stmt: LEU2(reg, reg) "# jcc le %1 %a # pseudo\n" 1
stmt: LTI2(reg, reg) "# jcc lt %1 %a # pseudo\n" 1
stmt: LTU2(reg, reg) "# jcc lt %1 %a # pseudo\n" 1

stmt: EQF2(reg, reg) "# jcc eq %1 %a # pseudo\n" 1
stmt: NEF2(reg, reg) "# jcc ne %1 %a # pseudo\n" 1
stmt: GEF2(reg, reg) "# stw ha\nldw %1\ncall gef\njcc ne zero %a # pseudo\n" 1
stmt: GTF2(reg, reg) "# stw ha\nldw %1\ncall gtf\njcc ne zero %a # pseudo\n" 1
stmt: LEF2(reg, reg) "# stw ha\nldw %1\ncall lef\njcc ne zero %a # pseudo\n" 1
stmt: LTF2(reg, reg) "# stw ha\nldw %1\ncall ltf\njcc ne zero %a # pseudo\n" 1

stmt: ARGF2(reg) "# push %0\n"  1
stmt: ARGI2(reg) "# push %0\n"  1
stmt: ARGU2(reg) "# push %0\n"  1
stmt: ARGP2(reg) "# push %0\n"  1

reg: CALLF2(reg)  `inst_call` 1
reg: CALLI2(reg)  `inst_call` 1
reg: CALLP2(reg)  `inst_call` 1
reg: CALLU2(reg)  `inst_call` 1
reg: CALLV(reg)   `inst_call` 1

reg: SYSI2 `inst_sys` 1

stmt: RETF2(reg)  "# ret\n" 1
stmt: RETI2(reg)  "# ret\n" 1
stmt: RETP2(reg)  "# ret\n" 1
stmt: RETU2(reg)  "# ret\n" 1
stmt: RETV(reg)   "# ret\n" 1
%%
static void progbeg(int argc, char* argv[]) {
	int i;

	{
		union {
			char c;
			int i;
		} u;
		u.i = 0;
		u.c = 1;
		swap = ((int)(u.i == 1)) != IR->little_endian;
	}
	parseflags(argc, argv);

	regs[0] = mkreg("vAC", 0, 1, IREG);
	for (i = 1; i < 16; i++) {
		regs[i] = mkreg("r%d", i, 1, IREG);
	}

	regw = mkwildcard(regs);

	tmask[IREG] = 0x00ff; tmask[FREG] = 0;
	vmask[IREG] = 0x7f00; vmask[FREG] = 0;
}
static Symbol rmap(int opk) {
	switch (optype(opk)) {
	case I:
	case U:
	case P:
	case B:
	case F:
		return regw;
	default:
		return 0;
	}
}
static void segment(int n) {
	// Nothing to do for the moment.
}
static void progend(void) {
	// Nothing to do for the moment.
}
static int rangep(Node p, unsigned lo, unsigned hi) {
	if (specific(p->op) != CNST+P) {
		return LBURG_MAX;
	}

	Symbol s = p->syms[0];
	unsigned u = (unsigned)s->u.c.v.p;
	return u >= lo && u <= hi ? 0 : LBURG_MAX;
}
static void target(Node p) {
	int kop;
	switch (generic(p->op)) {
	case CNST:
	case ADDRF:
	case ADDRG:
	case ADDRL:
		// Constants must be loaded into vAC.
		setreg(p, regs[0]);
		break;
	case INDIR:
		// If this is a ldloc, we also require vAC, so just assign the result to vAC.
		kop = generic(p->kids[0]->op);
		if (kop == ADDRF || kop == ADDRL) {
			setreg(p, regs[0]);
			break;
		}
		// fallthrough
	case RET:
		// Standard unary operators require their operand in vAC.
		rtarget(p, 0, regs[0]);
		break;
	case NEG:
	case BCOM:
		// These operators require their operand in a register besides vAC, and produce a result in vAC. We can't
		// represent the former--we'll just spull to ha if the operand is in vAC--but we can handle the latter.
		setreg(p, regs[0]);
		break;
	case ADD:
	case BAND:
	case BOR:
	case BXOR:
		// Commutative binops that can take a small constant operand.
		if (range(p->kids[1], 0, optype(p->op) == U ? 255 : 127) == 0) {
			rtarget(p->kids[0], 0, regs[0]);
			break;
		}
		// fallthrough
	case EQ:
	case GE:
	case GT:
	case LE:
	case LT:
	case NE:
		// Commutative operators and comparisons. In these cases, we put the RHS in vAC to help avoid spills.
		rtarget(p, 1, regs[0]);
		break;
	case LSH:
		// LSH reg, 1 is a unary operator. LSH reg, reg is a helper call.
		if (generic(p->kids[1]->op) == CNST && p->kids[1]->syms[0]->u.c.v.u == 1) {
			rtarget(p, 0, regs[0]);
			break;
		}
		// fallthrough
	case RSH:
	case MUL:
	case MOD:
	case DIV:
		// Helper calls. We can pick the targets to help avoid spills. We'll put the LHS in r0 and the RHS in vAC.
		rtarget(p, 0, regs[1]);
		rtarget(p, 1, regs[0]);
		break;
	case SUB:
		// SUB can take a small constant operand.
		if (range(p->kids[1], 0, optype(p->op) == U ? 255 : 127) == 0) {
			rtarget(p->kids[0], 0, regs[0]);
			break;
		}

		// TODO: attempt to swap the operands and turn this into an RSUB. For now, target the RHS in vAC, let the
		// LHS live anywhere, then spill the RHS to ha and load the LHS into vAC.
		rtarget(p, 1, regs[0]);
		break;
	case ASGN:
		// Assignments to VREG nodes do not need targeting.
		if (generic(p->kids[0]->op) != VREG) {
			rtarget(p, 1, regs[0]);
		}
		break;
	case CALL:
	case SYS:
		// Set the result register to vAC.
		setreg(p, regs[0]);
		break;
	}
}
static void clobber(Node p) {
	if (generic(p->op) == CALL) {
		spill(0x00ff, IREG, p);
	}
}
static void emitjcc(Node p, char* kind) {
	if (range(p->kids[0], 0, 0) != 0) {
		print("sub r%d\n", getregnum(p->kids[0]));
	}
	print("jcc %s %s\n", kind, p->syms[0]->x.name);
}

static void ensurereg(Node p) {
	int r = getregnum(p);
	if (r != 0) {
		print("stw r%d\n", r);
	}
}

static void inst_spill(Node p) {
	Node local = p->kids[0];
	Node vregp = p->kids[1]->kids[0]->kids[0];
	assert(getregnum(vregp) == 0);
	print("stw ha\nldwi %d\ncall stloc\n", local->syms[0]->x.offset);
}

static void inst_copy(Node p) {
	assert(getregnum(p) == 0 || getregnum(p->kids[0]) == 0);
	if (getregnum(p) == 0) {
		print("ldw r%d\n", getregnum(p->kids[0]));
	} else if (getregnum(p->kids[0]) == 0) {
		print("stw r%d\n", getregnum(p));
	}
}

static void inst_ldloc(Node p) {
	assert(getregnum(p) == 0);

	int offs;
	if (generic(p->kids[0]->op) == ADDRF) {
		offs = p->kids[0]->syms[0]->x.offset + offset;
	} else {
		assert(generic(p->kids[0]->op) == ADDRL);
		offs = p->kids[0]->syms[0]->x.offset;
	}

	if (offs < 256) {
		print("ldi %d\n", offs);
	} else {
		print("ldiw %d\n", offs);
	}
	print("call ldloc\n");
}

static void inst_st(Node p) {
	char* op = "st";
	if (opsize(p->op) == 2) {
		op = "stw";
	}
	print("%s $%x\n", op, p->kids[0]->syms[0]->u.c.v.u);
}

static void inst_ld(Node p) {
	char* op = "ld";
	if (opsize(p->op) == 2) {
		op = "ldw";
	}
	print("%s $%x\n", op, p->kids[0]->syms[0]->u.c.v.u);
	ensurereg(p);
}

static void inst_peek(Node p) {
	char* op = "peek";
	if (opsize(p->op) == 2) {
		op = "deek";
	}
	print("%s\n", op);
	ensurereg(p);
}

static void inst_poke(Node p) {
	char* op = "poke";
	if (opsize(p->op) == 2) {
		op = "doke";
	}
	print("%s r%d\n", op, getregnum(p->kids[0]));
}

static void inst_scon(Node p) {
	assert(getregnum(p) == 0);
	print("ldi $%x\n", p->syms[0]->u.c.v.u);
}

static void inst_cnstw(Node p) {
	assert(getregnum(p) == 0);
	print("ldwi $%x\n", p->syms[0]->u.c.v.u);
}

static void inst_addr(Node p) {
	assert(getregnum(p) == 0);
	print("ldwi %s\n", p->syms[0]->x.name);
}

static void inst_addi(Node p) {
	print("addi $%x\n", p->kids[1]->syms[0]->u.c.v.u);
	ensurereg(p);
}

static void inst_subi(Node p) {
	print("subi $%x\n", p->kids[1]->syms[0]->u.c.v.u);
	ensurereg(p);
}

static void inst_andi(Node p) {
	print("andi $%x\n", p->kids[1]->syms[0]->u.c.v.u);
	ensurereg(p);
}

static void inst_ori(Node p) {
	print("ori $%x\n", p->kids[1]->syms[0]->u.c.v.u);
	ensurereg(p);
}

static void inst_xori(Node p) {
	print("xori $%x\n", p->kids[1]->syms[0]->u.c.v.u);
	ensurereg(p);
}

static void inst_addw(Node p) {
	print("addw r%d\n", getregnum(p->kids[0]));
	ensurereg(p);
}

static void inst_subw(Node p) {
	print("subw r%d\n", getregnum(p->kids[0]));
	ensurereg(p);
}

static void inst_andw(Node p) {
	print("andw r%d\n", getregnum(p->kids[0]));
	ensurereg(p);
}

static void inst_orw(Node p) {
	print("orw r%d\n", getregnum(p->kids[0]));
	ensurereg(p);
}

static void inst_xorw(Node p) {
	print("xorw r%d\n", getregnum(p->kids[0]));
	ensurereg(p);
}

static void inst_lslw(Node p) {
	print("lslw\n");
	ensurereg(p);
}

static void inst_lsh(Node p) {
	print("call lsh\n");
	ensurereg(p);
}

static void inst_rsh(Node p) {
	print("call rsh\n");
	ensurereg(p);
}

static void inst_mul(Node p) {
	print("call mul\n");
	ensurereg(p);
}

static void inst_mod(Node p) {
	print("call mod\n");
	ensurereg(p);
}

static void inst_div(Node p) {
	print("call div\n");
	ensurereg(p);
}

static void inst_neg(Node p) {
	assert(getregnum(p) == 0);
	if (getregnum(p->kids[0]) == 0) {
		print("st ha\n");
		print("ldi 0\n");
		print("sub ha\n");
	} else {
		print("ldi 0\n");
		print("sub r%d\n", getregnum(p->kids[0]));
	}
}

static void inst_bcom(Node p) {
	assert(getregnum(p) == 0);
	if (getregnum(p->kids[0]) == 0) {
		print("st ha\n");
		print("ldwi $ffff\n");
		print("xorw ha\n");
	} else {
		print("ldwi $ffff\n");
		print("xorw r%d\n", getregnum(p->kids[0]));
	}
}

static void inst_call(Node p) {
	if (getregnum(p->kids[0]) == 0) {
		print("call vAC\n");
	} else {
		print("call r%d\n", getregnum(p->kids[0]));
	}
}

static void inst_sys(Node p) {
	print("sys %d\n", p->syms[0]->u.c.v.i);
}

static void emit2(Node p) {
	long i;
	unsigned long u;
	switch (specific(p->op)) {
	case CVF+F:
	case CVF+I:

	case CVI+F:
	case CVI+I:
	case CVI+U:

	case CVP+U:

	case CVU+I:
	case CVU+P:
	case CVU+U:

	case NEG+F:
	case NEG+I:
		break;

	case ARG+F:
	case ARG+I:
	case ARG+U:
	case ARG+P:
		print("; arg...\n");
		break;

	case ADDRF+P:

	case ADDRL+P:
		break;

	case EQ+F:
	case EQ+I:
	case EQ+U:
		emitjcc(p, "eq");
		break;

	case GE+F:
	case GE+I:
	case GE+U:
		emitjcc(p, "lt");
		break;

	case GT+F:
	case GT+I:
	case GT+U:
		emitjcc(p, "le");
		break;

	case LE+F:
	case LE+I:
	case LE+U:
		emitjcc(p, "gt");
		break;

	case LT+F:
	case LT+I:
	case LT+U:
		emitjcc(p, "ge");
		break;

	case NE+F:
	case NE+I:
	case NE+U:
		emitjcc(p, "ne");
		break;

	case JUMP+V:

	case LABEL+V:
		break;
	}
}
static void doarg(Node p) {
	// Nothing to do
}
static void blkfetch(int k, int off, int reg, int tmp) {}
static void blkstore(int k, int off, int reg, int tmp) {}
static void blkloop(int dreg, int doff, int sreg, int soff, int size, int tmps[]) {}
static void local(Symbol p) {
	if (isfloat(p->type)) {
		p->sclass = AUTO;
	}
	if (askregvar(p, (*IR->x.rmap)(ttob(p->type))) == 0) {
		assert(p->sclass == AUTO);
		offset += p->type->size;
		p->x.offset = -offset;
		p->x.name = stringd(-offset);
	}
}
static void function(Symbol f, Symbol caller[], Symbol callee[], int n) {
	vac.known = 0;

	print("%s:\n", f->x.name);

	// TODO: prolog?

	usedmask[0] = usedmask[1] = 0;
	freemask[0] = freemask[1] = ~(unsigned)0;

	// Home incoming args
	offset = 0;
	for (int i = 0; callee[i]; i++) {
		Symbol p = callee[i], q = caller[i];
		assert(q);
		p->x.offset = q->x.offset = offset;
		p->x.name = q->x.name = stringf("%d", p->x.offset);
		p->sclass = q->sclass = AUTO;
		offset += q->type->size;
	}
	offset = maxoffset = 0;

	gencode(caller, callee);
	framesize = maxoffset;
	print("push\n");
	if (framesize > 0) {
		print("%s %d\n", framesize < 256 ? "ldi" : "ldwi", -framesize);
		print("addw sp\n");
		print("stw sp\n");
	}

	emitcode();

	if (framesize > 0) {
		print("%s %d\n", framesize < 256 ? "ldi" : "ldwi", framesize);
		print("addw sp\n");
		print("stw sp\n");
	}

	print("pop\n");
	print("ret\n");
}
static void defsymbol(Symbol p) {
	if (p->scope >= LOCAL && p->sclass == STATIC) {
		p->x.name = stringf("L%d", genlabel(1));
	} else if (p->generated) {
		p->x.name = stringf("L%s", p->name);
	} else if (p->scope == GLOBAL || p->sclass == EXTERN) {
		p->x.name = stringf("_%s", p->name);
	} else {
		p->x.name = p->name;
	}
}
static void address(Symbol q, Symbol p, long n) {
	if (p->scope == GLOBAL || p->sclass == STATIC || p->sclass == EXTERN) {
		q->x.name = stringf("%s%s%D", p->x.name, n >= 0 ? "+" : "", n);
	} else {
		assert(n <= INT_MAX && n >= INT_MIN);
		q->x.offset = p->x.offset + n;
		q->x.name = stringd(q->x.offset);
	}
}
static void defconst(int suffix, int size, Value v) {
	// TODO: need to do something here...
	//assert(0);
}
static void defaddress(Symbol n) {
	// TODO: need to do something here...
	assert(0);	
}
static void defstring(int n, char* str) {
	// TODO: need to do something here...
	//assert(0);
	print("%s\n", str);
}
static void export(Symbol p) {
	// Nothing to do
}
static void import(Symbol p) {
	// Nothing to do
}
static void global(Symbol p) {
	print("%s:\n", p->x.name);
	// Nothing to do
}
static void space(int n) {
	// TODO: need to do something here...
}
Interface gt1IR = {
	1, 1, 0,  /* char */
	2, 1, 0,  /* short */
	2, 1, 0,  /* int */
	2, 1, 0,  /* long */
	2, 1, 0,  /* long long */
	2, 1, 1,  /* float */
	2, 1, 1,  /* double */
	2, 1, 1,  /* long double */
	2, 1, 0,  /* T * */
	0, 1, 0,  /* struct */
	1,        /* little_endian */
	0,        /* mulops_calls */
	0,        /* wants_callb */
	1,        /* wants_argb */
	0,        /* left_to_right */
	0,        /* wants_dag */
	0,        /* unsigned_char */
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
	0, 0, 0, 0, 0, 0, 0,
	{1, rmap,
		blkfetch, blkstore, blkloop,
		_label,
		_rule,
		_nts,
		_kids,
		_string,
		_actions,
		_templates,
		_isinstruction,
		_ntname,
		emit2,
		doarg,
		target,
		clobber,
	}
};
static char rcsid[] = "$Id$";
