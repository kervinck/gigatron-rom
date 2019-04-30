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
static void canonicalize(Node*);
static void clobber(Node);
static void function(Symbol, Symbol [], Symbol [], int);
static Node gengt1(Node);
static void global(Symbol);
static void import(Symbol);
static int isinc(Node);
static void local(Symbol);
static void progbeg(int, char **);
static void progend(void);
static int rangep(Node, unsigned, unsigned);
static int saferegnum(Node);
static void segment(int);
static void space(int);
static void target(Node);

static void inst_inc(Node);
static void inst_strunc(Node);
static void inst_copy(Node);
static void inst_spill(Node);
static void inst_blkcopy(Node);
static void inst_ldloc(Node);
static void inst_stloc(Node);
static void inst_ld(Node);
static void inst_st(Node);
static void inst_peek(Node);
static void inst_poke(Node);
static void inst_scon(Node);
static void inst_addr(Node);
static void inst_cnstw(Node);
static void inst_addi(Node);
static void inst_subi(Node);
static void inst_andi(Node);
static void inst_ori(Node);
static void inst_xori(Node);
static void inst_addw(Node);
static void inst_subw(Node);
static void inst_andw(Node);
static void inst_orw(Node);
static void inst_xorw(Node);
static void inst_lslw(Node);
static void inst_lsh8(Node);
static void inst_rsh8(Node);
static void inst_lsh(Node);
static void inst_rsh(Node);
static void inst_mul(Node);
static void inst_mod(Node);
static void inst_div(Node);
static void inst_trunc(Node);
static void inst_sext(Node);
static void inst_neg(Node);
static void inst_bcom(Node);
static void inst_call(Node);
static void inst_sys(Node);
static void inst_lup(Node);
static void inst_jr(Node);
static void inst_label(Node);
static void inst_jeq(Node);
static void inst_jne(Node);
static void inst_jge(Node);
static void inst_jgt(Node);
static void inst_jle(Node);
static void inst_jlt(Node);
static void inst_arg(Node);
static void inst_ret(Node);

struct inst {
	int opcode;
	Symbol operand;
	struct inst *prev, *next;
};
typedef struct inst *Inst;

static Symbol wregs[32], bregs[32], vregs[32];
static Symbol wregw, bregw, vregw;
static unsigned argsize, saversize, argdepth;
static Inst* insts;

static int infunction = 0;
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
%term LOADP2=2279

%term VREGP=711

%term SYSI2=2661
%term LUPI2=2677

%%
trunc: CVII1(reg)  "%a" 1
trunc: CVIU1(reg)  "%a" 1
trunc: CVUI1(reg)  "%a" 1
trunc: CVUU1(reg)  "%a" 1

con0: CNSTI2 "0" range(a, 0, 0)
con0: CNSTU2 "0" range(a, 0, 0)

con1: CNSTI2 "1" range(a, 1, 1)
con1: CNSTU2 "1" range(a, 1, 1)

lshcon: CNSTI2 "%a" range(a, 0, 4)
lshcon: CNSTU2 "%a" range(a, 0, 4)

con8: CNSTI2 "8" range(a, 8, 8)
con8: CNSTU2 "8" range(a, 8, 8)

scon: CNSTI2 "%a" range(a, 0, 255)
scon: CNSTU2 "%a" range(a, 0, 255)
scon: CNSTP2 "%a" rangep(a, 0, 255)

reg: INDIRI1(VREGP)       "# read register\n"
reg: INDIRU1(VREGP)       "# read register\n"

reg: INDIRI2(VREGP)       "# read register\n"
reg: INDIRU2(VREGP)       "# read register\n"
reg: INDIRP2(VREGP)       "# read register\n"

stmt: ASGNI1(VREGP, CVII1(ADDI2(LOADI1(INDIRI1(VREGP)), con1))) `inst_inc` isinc(a)
stmt: ASGNU1(VREGP, CVUU1(ADDI2(LOADU1(INDIRU1(VREGP)), con1))) `inst_inc` isinc(a)
stmt: ASGNI1(scon, CVII1(ADDI2(INDIRI1(scon), con1))) `inst_inc` isinc(a)
stmt: ASGNU1(scon, CVUU1(ADDU2(INDIRU1(scon), con1))) `inst_inc` isinc(a)

stmt: ASGNI1(VREGP, trunc) `inst_strunc`
stmt: ASGNU1(VREGP, trunc) `inst_strunc`

stmt: ASGNI1(VREGP, reg)  "# write register\n"
stmt: ASGNU1(VREGP, reg)  "# write register\n"

stmt: ASGNI2(VREGP, reg)  "# write register\n"
stmt: ASGNU2(VREGP, reg)  "# write register\n"
stmt: ASGNP2(VREGP, reg)  "# write register\n"

stmt: ASGNI1(ADDRLP2, LOADI1(INDIRI1(VREGP))) `inst_spill` 1
stmt: ASGNI2(ADDRLP2, LOADI2(INDIRI2(VREGP))) `inst_spill` 1
stmt: ASGNU1(ADDRLP2, LOADU1(INDIRU1(VREGP))) `inst_spill` 1
stmt: ASGNU2(ADDRLP2, LOADU2(INDIRU2(VREGP))) `inst_spill` 1
stmt: ASGNP2(ADDRLP2, LOADP2(INDIRP2(VREGP))) `inst_spill` 1

stmt: ASGNB(reg, INDIRB(reg)) `inst_blkcopy` 1

stmt: reg "# "

reg: LOADI1(reg)  `inst_copy` 1
reg: LOADI2(reg)  `inst_copy` 1
reg: LOADU1(reg)  `inst_copy` 1
reg: LOADU2(reg)  `inst_copy` 1
reg: LOADP2(reg)  `inst_copy` 1

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

stmt: ASGNI1(ADDRLP2, reg) `inst_stloc` 1
stmt: ASGNI2(ADDRLP2, reg) `inst_stloc` 1
stmt: ASGNU1(ADDRLP2, reg) `inst_stloc` 1
stmt: ASGNU2(ADDRLP2, reg) `inst_stloc` 1
stmt: ASGNP2(ADDRLP2, reg) `inst_stloc` 1
stmt: ASGNF2(ADDRLP2, reg) `inst_stloc` 1

stmt: ASGNI1(ADDRFP2, reg) `inst_stloc` 1
stmt: ASGNI2(ADDRFP2, reg) `inst_stloc` 1
stmt: ASGNU1(ADDRFP2, reg) `inst_stloc` 1
stmt: ASGNU2(ADDRFP2, reg) `inst_stloc` 1
stmt: ASGNP2(ADDRFP2, reg) `inst_stloc` 1
stmt: ASGNF2(ADDRFP2, reg) `inst_stloc` 1

reg: INDIRI1(scon) `inst_ld` 1
reg: INDIRU1(scon) `inst_ld` 1
reg: INDIRI2(scon) `inst_ld` 1
reg: INDIRU2(scon) `inst_ld` 1
reg: INDIRP2(scon) `inst_ld` 1
reg: INDIRF2(scon) `inst_ld` 1
stmt: ASGNI1(scon, trunc) `inst_st` 1
stmt: ASGNI1(scon, reg) `inst_st` 1
stmt: ASGNU1(scon, trunc) `inst_st` 1
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

reg: LSHI2(reg, lshcon)  `inst_lslw` 1
reg: LSHU2(reg, lshcon)  `inst_lslw` 1

reg: LSHI2(reg, con8)  `inst_lsh8` 1
reg: LSHU2(reg, con8)  `inst_lsh8` 1

reg: RSHI2(reg, con8)  `inst_rsh8` 1
reg: RSHU2(reg, con8)  `inst_rsh8` 1

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

reg: CVII1(reg)  `inst_trunc` 1
reg: CVII2(reg)  `inst_sext` 1
reg: CVIU1(reg)  `inst_trunc` 1
reg: CVIU2(reg)  `inst_sext` 1

reg: CVPU2(reg)  "# " 1

reg: CVUI1(reg)  `inst_trunc` 1
reg: CVUI2(reg)  "# " 1
reg: CVUP2(reg)  "# " 1
reg: CVUU1(reg)  `inst_trunc` 1
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

stmt: JUMPV(reg) `inst_jr` 1
stmt: LABELV     `inst_label`

stmt: EQI2(reg, con0) `inst_jeq` 1
stmt: EQU2(reg, con0) `inst_jeq` 1
stmt: NEI2(reg, con0) `inst_jne` 1
stmt: NEU2(reg, con0) `inst_jne` 1
stmt: GEI2(reg, con0) `inst_jge` 1
stmt: GEU2(reg, con0) `inst_jge` 1
stmt: GTI2(reg, con0) `inst_jgt` 1
stmt: GTU2(reg, con0) `inst_jgt` 1
stmt: LEI2(reg, con0) `inst_jle` 1
stmt: LEU2(reg, con0) `inst_jle` 1
stmt: LTI2(reg, con0) `inst_jlt` 1
stmt: LTU2(reg, con0) `inst_jlt` 1

stmt: EQF2(reg, reg) "# jcc eq %1 %a # pseudo\n" 1
stmt: NEF2(reg, reg) "# jcc ne %1 %a # pseudo\n" 1
stmt: GEF2(reg, reg) "# stw ha\nldw %1\ncall gef\njcc ne zero %a # pseudo\n" 1
stmt: GTF2(reg, reg) "# stw ha\nldw %1\ncall gtf\njcc ne zero %a # pseudo\n" 1
stmt: LEF2(reg, reg) "# stw ha\nldw %1\ncall lef\njcc ne zero %a # pseudo\n" 1
stmt: LTF2(reg, reg) "# stw ha\nldw %1\ncall ltf\njcc ne zero %a # pseudo\n" 1

stmt: ARGF2(reg) `inst_arg`  1
stmt: ARGI2(reg) `inst_arg`  1
stmt: ARGU2(reg) `inst_arg`  1
stmt: ARGP2(reg) `inst_arg`  1

reg: CALLF2(reg)  `inst_call` 1
reg: CALLI2(reg)  `inst_call` 1
reg: CALLP2(reg)  `inst_call` 1
reg: CALLU2(reg)  `inst_call` 1
reg: CALLV(reg)   `inst_call` 1

reg: SYSI2 `inst_sys` 1

reg: LUPI2(reg) `inst_lup` 1

stmt: RETF2(reg)  `inst_ret` 1
stmt: RETI2(reg)  `inst_ret` 1
stmt: RETP2(reg)  `inst_ret` 1
stmt: RETU2(reg)  `inst_ret` 1
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

	wregs[0] = mkreg("vAC", 0, 1, IREG);
	bregs[0] = mkreg("vACb", 0, 1, IREG);
	vregs[0] = NULL;
	for (i = 1; i < 16; i++) {
		vregs[i] = wregs[i] = mkreg("r%d", i, 1, IREG);
		bregs[i] = mkreg("r%db", i, 1, IREG);
	}

	wregw = mkwildcard(wregs);
	bregw = mkwildcard(bregs);
	vregw = mkwildcard(vregs);

	tmask[IREG] = 0x00ff; tmask[FREG] = 0;
	vmask[IREG] = 0x7f00; vmask[FREG] = 0;
}
static Symbol rmap(int opk) {
	switch (optype(opk)) {
	case I:
	case U:
		return opsize(opk) == 1 ? bregw : wregw;
	case P:
	case B:
	case F:
		return wregw;
	default:
		return 0;
	}
}
static Symbol vac(int opk) {
	switch (optype(opk)) {
	case I:
	case U:
		return opsize(opk) == 1 ? bregs[0] : wregs[0];
	default:
		return wregs[0];
	}
}
static void segment(int n) {
	// Nothing to do for the moment.
}
static void progend(void) {
	// Nothing to do for the moment.
}
static Symbol constant_zero(int type) {
	Value v;
	switch (type) {
	case I:
		v.i = 0;
		return constant(inttype, v);
	case U:
		v.u = 0;
		return constant(unsignedtype, v);
	}
	assert(0);
	return NULL;
}
static Symbol constant_negate(Symbol c) {
	Value v = c->u.c.v;
	v.i = -v.i;
	return constant(c->type, v);
}
static void canonicalize(Node* pp) {
	// Canonicalization:
	// - Ensure that constants are always on the RHS (except for SUB)
	// - Turn subtraction of a large constant into addition of the negated constant
	// - Replace conditionals with conditionals vs. 0

	Node p = *pp;
	if (p == NULL) {
		return;
	}
	debug(fprintf(stderr, "(canonicalizing %p (%p, %p))\n", p, p->kids[0], p->kids[1]));
	canonicalize(&p->kids[0]);
	canonicalize(&p->kids[1]);
	switch (generic(p->op)) {
	case CVP:
		*pp = p->kids[0];
		break;
	case CVU:
		if (opsize(p->op) == 2) {
			*pp = p->kids[0];
		}
		break;
	case CVI:
		if (opsize(p->op) == p->syms[0]->u.c.v.i) {
			*pp = p->kids[0];
		}
		break;
	case ADD: case BAND: case BOR: case BXOR:
		if (generic(p->kids[0]->op) == CNST) {
			Node k = p->kids[0];
			p->kids[0] = p->kids[1];
			p->kids[1] = k;
		}
		break;
	case SUB:
		if (generic(p->kids[1]->op) == CNST && range(p->kids[1], 0, 255) != 0) {
			p->kids[1]->syms[0] = constant_negate(p->kids[1]->syms[0]);
			p->op = ADD + optype(p->op) + sizeop(opsize(p->op));
		}
		break;
	case EQ: case NE: case GE: case GT: case LE: case LT:
		if (generic(p->kids[0]->op) == CNST) {
			Node k = p->kids[0];
			p->kids[0] = p->kids[1];
			p->kids[1] = k;
		}
		if (range(p->kids[1], 0, 0) != 0) {
			// TODO: check for opposite signs if this is a signed compare. This is relatively complicated, as it
			// involves spilling to a temp and inserting a new tree.
			int opst = optype(p->op) + sizeop(opsize(p->op));
			p->kids[0] = newnode(SUB + opst, p->kids[0], p->kids[1], NULL);
			p->kids[1] = newnode(CNST + opst, NULL, NULL, constant_zero(optype(p->op)));
		}
		break;
	}
}
static Node gengt1(Node forest) {
	for (Node* p = &forest; *p != NULL; p = &(*p)->link) {
		debug(fprintf(stderr, "pre-canon: "));
		debug(dumptree(*p));
		debug(fprint(stderr, "\n"));

		canonicalize(p);

		debug(fprintf(stderr, "post-canon: "));
		debug(dumptree(*p));
		debug(fprint(stderr, "\n"));
	}

	return gen(forest);
}
static int rangep(Node p, unsigned lo, unsigned hi) {
	if (specific(p->op) != CNST+P) {
		return LBURG_MAX;
	}

	Symbol s = p->syms[0];
	unsigned u = (unsigned)s->u.c.v.p;
	return u >= lo && u <= hi ? 0 : LBURG_MAX;
}
static int saferegnum(Node p) {
	if (p->syms[RX]->x.regnode == NULL) {
		assert(p->syms[RX]->sclass == REGISTER);
		return -1;
	}
	return getregnum(p);
}
static void target(Node p) {
	int kop;
	switch (generic(p->op)) {
	case CNST:
	case ADDRF:
	case ADDRG:
	case ADDRL:
		// Constants must be loaded into vAC.
		setreg(p, wregs[0]);
		break;
	case INDIR:
		// If this is a ldloc, we also require vAC, so just assign the result to vAC.
		kop = generic(p->kids[0]->op);
		if (kop == ADDRF || kop == ADDRL) {
			setreg(p, wregs[0]);
		} else {
			rtarget(p, 0, wregs[0]);
		}
		break;
	case CVI:
	case CVU:
	case CVF:
	case RET:
	case ARG:
	case LUP:
		// Standard unary operators require their operand in vAC.
		rtarget(p, 0, vac(p->kids[0]->op));
		break;
	case NEG:
	case BCOM:
		// These operators require their operand in a register besides vAC, and produce a result in vAC.
		rtarget(p, 0, vregw);
		setreg(p, wregs[0]);
		break;
	case ADD:
	case BAND:
	case BOR:
	case BXOR:
		// Commutative binops that can take a small constant operand. Otherwise, put the RHS in vAC to help avoid
		// spills.
		if (range(p->kids[1], 0, 255) == 0) {
			rtarget(p, 0, wregs[0]);
		} else {
			rtarget(p, 0, vregw);
			rtarget(p, 1, wregs[0]);
		}
		break;
	case EQ:
	case GE:
	case GT:
	case LE:
	case LT:
	case NE:
		// Comparisons. All of these have an RHS of 0; we need the LHS in vAC.
		rtarget(p, 0, wregs[0]);
		break;
	case LSH:
	case RSH:
		// LSH reg, [1,8] are unary operators. LSH reg, reg is a helper call.
		if (generic(p->kids[1]->op) == CNST) {
			unsigned amt = p->kids[1]->syms[0]->u.c.v.u;
			if ((generic(p->op) == LSH && amt <= 4) || amt == 8) {
				rtarget(p, 0, wregs[0]);
				break;
			}
		}
		// fallthrough
	case MUL:
	case MOD:
	case DIV:
		// Helper calls. We can pick the targets to help avoid spills. We'll put the LHS in r0 and the RHS in vAC.
		// Note that we first target the LHS to vAC in order to ensure that a copy-through-vAC is inserted.
		rtarget(p, 0, wregs[0]);
		rtarget(p, 0, wregs[1]);
		rtarget(p, 1, wregs[0]);
		break;
	case SUB:
		// TODO: attempt to swap the operands and turn this into an RSUB. For now, do nothing:
		// - pattern matching during emit will allow us to handle constants, etc.
		// - at emit time:
		//     - if the LHS is not in vAC, load it into vAC
		// - because we do not have any operand killing vAC, we need the SUB itself to kill vAC.
		if (generic(p->kids[1]->op) != CNST) {
			rtarget(p, 1, vregw);
		}
		setreg(p, wregs[0]);
		break;
	case ASGN:
		// Handle block copies first. These become helper calls.
		if (specific(p->op) == ASGN+B) {
			rtarget(p, 0, wregs[0]);
			rtarget(p, 0, wregs[1]);
			rtarget(p, 1, wregs[0]);
			break;
		}

		// Assignments to VREG nodes may need targeting if there is no intermediate load through vAC.
		if (p->kids[0]->op == VREG+P) {
			if (generic(p->kids[1]->op) == LOAD && saferegnum(p->kids[1]) != 0) {
				rtarget(p->kids[1], 0, vac(p->kids[1]->kids[0]->op));
			}
		} else {
			// stores to locals are special
			if (generic(p->kids[0]->op) != ADDRL && generic(p->kids[0]->op) != ADDRF) {
				rtarget(p, 0, vregw);
			}
			rtarget(p, 1, vac(p->kids[0]->op));
		}
		break;
	case CALL:
	case SYS:
		// Set the result register to vAC.
		setreg(p, wregs[0]);
		break;
	}
}
static void clobber(Node p) {
	// Nothing to do.
	if (generic(p->op) == CALL) {
		debug(fprintf(stderr, "live registers at callsite: %x\n", ~freemask[IREG]));
	}
}
static void ensurereg(Node p) {
	int r = getregnum(p);
	if (r != 0) {
		print("asm.stw('r%d')\n", r);
	}
}
static int isinc(Node p) {
	// Two cases: reg and zp increment
	// 
	// TODO: the patterns that match this need to be more comprehensive.
	if (p->kids[0]->op == VREG+P) {
		Symbol dest = p->kids[0]->syms[0];
		Symbol src = p->kids[1]->kids[0]->kids[0]->kids[0]->syms[RX];
		return src != NULL && src == dest ? 0 : LBURG_MAX;
	}

	Symbol src = p->kids[0]->syms[0];
	Symbol dest = p->kids[1]->kids[0]->kids[0]->syms[0];
	return src->u.c.v.u == dest->u.c.v.u ? 0 : LBURG_MAX;
}

static void inst_inc(Node p) {
	// Two cases: reg and zp increment
	if (p->kids[0]->op == VREG+P) {
		print("asm.inc('r%d')\n", p->kids[0]->syms[0]->x.regnode->number);
	} else {
		print("asm.inc(%D)\n", p->kids[0]->syms[0]->u.c.v.i);
	}
}

static void inst_strunc(Node p) {
	unsigned to = p->kids[0]->syms[0]->x.regnode->number;
	unsigned from = getregnum(p->kids[1]->kids[0]);
	assert(from == 0 || to != 0);
	print("asm.st('r%d')\n", to);
}

static void inst_copy(Node p) {
	unsigned to = getregnum(p), from = getregnum(p->kids[0]);
	assert(to == 0 || from == 0 || to == from);

	if (to == from) {
		// ignore this case
	} else if (to == 0) {
		print("asm.ldw('r%d')\n", from);
	} else if (from == 0) {
		print("asm.stw('r%d')\n", to);
	}
}

static void inst_spill(Node p) {
	// inst_spill is essentially the same as inst_stloc, but covers an entire spill tree to avoid reentry in the
	// spiller when spilling vAC.
	Node vregp = p->kids[1]->kids[0]->kids[0];
	assert(getregnum(vregp) == 0);
	setreg(p->kids[1], wregs[0]);
	inst_stloc(p);
}

static void inst_blkcopy(Node p) {
	print("asm.stw('ha')\n");
	print("asm.ldi(%d)\n", p->syms[0]->u.c.v.u);
	print("asm.call('blkcopy')\n");
}

static int localoffset(Node addr) {
	if (generic(addr->op) == ADDRF) {
		// Args end at sp + 2 + framesize + saversize + argsize.
		return 2 + argdepth + framesize + saversize + argsize - addr->syms[0]->x.offset;
	}

	// Locals end at sp + 2 + framesize.
	assert(generic(addr->op) == ADDRL);
	return 2 + argdepth + framesize - addr->syms[0]->x.offset;
}

static void inst_ldloc(Node p) {
	assert(getregnum(p) == 0);

	int offs = localoffset(p->kids[0]);
	assert(offs >= 0);

	if (offs < 256) {
		print("asm.ldi(%d)\n", offs);
	} else {
		print("asm.ldwi(%d)\n", offs);
	}
	print("asm.call('ldloc%s')\n", opsize(p->op) == 1 ? "b" : "");
}

static void inst_stloc(Node p) {
	assert(getregnum(p->kids[1]) == 0);

	int offs = localoffset(p->kids[0]);
	assert(offs >= 0);

	print("asm.stw('ha')\n");
	if (offs < 256) {
		print("asm.ldi(%d)\n", offs);
	} else {
		print("asm.ldwi(%d)\n", offs);
	}
	print("asm.call('stloc%s')\n", opsize(p->op) == 1 ? "b" : "");
}

static void inst_st(Node p) {
	char* op = "st";
	if (opsize(p->op) == 2) {
		op = "stw";
	}
	print("asm.%s(%D)\n", op, p->kids[0]->syms[0]->u.c.v.i);
}

static void inst_ld(Node p) {
	char* op = "ld";
	if (opsize(p->op) == 2) {
		op = "ldw";
	}
	print("asm.%s(%D)\n", op, p->kids[0]->syms[0]->u.c.v.i);
	ensurereg(p);
}

static void inst_peek(Node p) {
	char* op = "peek";
	if (opsize(p->op) == 2) {
		op = "deek";
	}
	print("asm.%s()\n", op);
	ensurereg(p);
}

static void inst_poke(Node p) {
	char* op = "poke";
	if (opsize(p->op) == 2) {
		op = "doke";
	}
	print("asm.%s('r%d')\n", op, getregnum(p->kids[0]));
}

static void inst_scon(Node p) {
	assert(getregnum(p) == 0);
	print("asm.ldi(%D)\n", p->syms[0]->u.c.v.i);
}

static void inst_cnstw(Node p) {
	assert(getregnum(p) == 0);
	print("asm.ldwi(%D)\n", p->syms[0]->u.c.v.i);
}

static void inst_addr(Node p) {
	assert(getregnum(p) == 0);

	int offs;
	switch (generic(p->op)) {
	case ADDRF:
	case ADDRL:
		offs = localoffset(p);
		if (offs < 256) {
			print("asm.ldw('sp')\n");
			if (offs > 0) {
				print("asm.addi(%d)\n", offs);
			}
		} else {
			print("asm.ldwi(%d)\n", offs);
			print("asm.addw('sp')\n");
		}
		break;
	case ADDRG:
		print("asm.ldwi('%s')\n", p->syms[0]->x.name);
		break;
	default:
		assert(0);
	}
}

static void inst_addi(Node p) {
	print("asm.addi(%D)\n", p->kids[1]->syms[0]->u.c.v.i);
	ensurereg(p);
}

static void inst_subi(Node p) {
	assert(getregnum(p) == 0);

	if (getregnum(p->kids[0]) != 0) {
		print("asm.ldw('r%d')\n", getregnum(p->kids[0]));
	}
	print("asm.subi(%D)\n", p->kids[1]->syms[0]->u.c.v.i);
}

static void inst_andi(Node p) {
	print("asm.andi(%D)\n", p->kids[1]->syms[0]->u.c.v.i);
	ensurereg(p);
}

static void inst_ori(Node p) {
	print("asm.ori(%D)\n", p->kids[1]->syms[0]->u.c.v.i);
	ensurereg(p);
}

static void inst_xori(Node p) {
	print("asm.xori(%D)\n", p->kids[1]->syms[0]->u.c.v.i);
	ensurereg(p);
}

static void inst_addw(Node p) {
	print("asm.addw('r%d')\n", getregnum(p->kids[0]));
	ensurereg(p);
}

static void inst_subw(Node p) {
	assert(getregnum(p) == 0);

	char* fmt = "asm.subw('r%d')\n";
	if (getregnum(p->kids[1]) == 0) {
		fmt = "asm.subw('ht')\n";
		print("asm.stw('ht')\n");
	}
	if (getregnum(p->kids[0]) != 0) {
		print("asm.ldw('r%d')\n", getregnum(p->kids[0]));
	}
	print(fmt, getregnum(p->kids[1]));
}

static void inst_andw(Node p) {
	print("asm.andw('r%d')\n", getregnum(p->kids[0]));
	ensurereg(p);
}

static void inst_orw(Node p) {
	print("asm.orw('r%d')\n", getregnum(p->kids[0]));
	ensurereg(p);
}

static void inst_xorw(Node p) {
	print("asm.xorw('r%d')\n", getregnum(p->kids[0]));
	ensurereg(p);
}

static void inst_lslw(Node p) {
	for (unsigned i = 0; i < p->kids[1]->syms[0]->u.c.v.u; i++) {
		print("asm.lslw()\n");
	}
	ensurereg(p);
}

static void inst_lsh8(Node p) {
	print("asm.st('vACH')\n");
	print("asm.ori(0xff)\n");
	print("asm.xori(0xff)\n");
	ensurereg(p);
}

static void inst_rsh8(Node p) {
	print("asm.ld('vACH')\n");
	ensurereg(p);
}

static void inst_lsh(Node p) {
	print("asm.call('lsh')\n");
	ensurereg(p);
}

static void inst_rsh(Node p) {
	print("asm.call('rsh')\n");
	ensurereg(p);
}

static void inst_mul(Node p) {
	print("asm.call('mul')\n");
	ensurereg(p);
}

static void inst_mod(Node p) {
	print("asm.call('mod')\n");
	ensurereg(p);
}

static void inst_div(Node p) {
	print("asm.call('div')\n");
	ensurereg(p);
}

static void inst_trunc(Node p) {
	print("asm.andi(0xff)\n");
	ensurereg(p);
}

static void inst_sext(Node p) {
	print("asm.xori(0x80)\n");
	print("asm.subi(0x80)\n");
	ensurereg(p);
}

static void inst_neg(Node p) {
	assert(getregnum(p) == 0);
	if (getregnum(p->kids[0]) == 0) {
		print("asm.stw('ha')\n");
		print("asm.ldi(0)\n");
		print("asm.subw('ha')\n");
	} else {
		print("asm.ldi(0)\n");
		print("asm.subw('r%d')\n", getregnum(p->kids[0]));
	}
}

static void inst_bcom(Node p) {
	assert(getregnum(p) == 0);
	if (getregnum(p->kids[0]) == 0) {
		print("asm.stw('ha')\n");
		print("asm.ldwi(0xffff)\n");
		print("asm.xorw('ha')\n");
	} else {
		print("asm.ldwi(0xffff)\n");
		print("asm.xorw('r%d')\n", getregnum(p->kids[0]));
	}
}

static void inst_call(Node p) {
	if (getregnum(p->kids[0]) == 0) {
		print("asm.call('vAC')\n");
	} else {
		print("asm.call('r%d')\n", getregnum(p->kids[0]));
	}
	argdepth = 0;
}

static void inst_sys(Node p) {
	print("asm.sys(%D)\n", p->syms[0]->u.c.v.i);
}

static void inst_lup(Node p) {
	print("asm.lup(%D)\n", p->syms[0]->u.c.v.i);
}

static void inst_jr(Node p) {
	print("asm.jr()\n");
}

static void inst_label(Node p) {
	print("asm.label('%s')\n", p->syms[0]->x.name);
}

static void inst_jeq(Node p) {
	print("asm.jeq('%s')\n", p->syms[0]->x.name);
}

static void inst_jne(Node p) {
	print("asm.jne('%s')\n", p->syms[0]->x.name);
}

static void inst_jge(Node p) {
	print("asm.jge('%s')\n", p->syms[0]->x.name);
}

static void inst_jgt(Node p) {
	print("asm.jgt('%s')\n", p->syms[0]->x.name);
}

static void inst_jle(Node p) {
	print("asm.jle('%s')\n", p->syms[0]->x.name);
}

static void inst_jlt(Node p) {
	print("asm.jlt('%s')\n", p->syms[0]->x.name);
}

static void inst_arg(Node p) {
	print("asm.call('pusha')\n");
	argdepth += 2;
}

static void inst_ret(Node p) {
	print("asm.stw('rv')\n");
}

static void emit2(Node p) {
	// Nothing to do
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
		debug(fprint(stderr, "local %s of size %d @ offset %d\n", p->name, p->type->size, offset));
		p->x.offset = offset;
		p->x.name = stringd(offset);
	}
}
static void function(Symbol f, Symbol caller[], Symbol callee[], int n) {
	// Frame:
	//
	// +-------------------------------+
	// | Incoming arguments            |
	// +-------------------------------+ <- original sp
	// | Saved registers               | <- saversize bytes
	// +-------------------------------+
	// | Locals                        | <- framesize bytes
	// +-------------------------------+ <- sp
	//
	// - Args end at sp + framesize + saversize
	// - Locals end at sp

	debug(fprint(stderr, "(function %s)\n", f->x.name));
	print("asm.defun('%s')\n", f->x.name);
	infunction = 1;

	usedmask[0] = usedmask[1] = 0;
	freemask[0] = freemask[1] = ~(unsigned)0;

	// Home incoming args
	offset = 0;
	for (int i = 0; callee[i]; i++) {
		Symbol p = callee[i], q = caller[i];
		assert(q);
		offset += roundup(q->type->size, 2);
		debug(fprint(stderr, "parameter %s of size %d @ offset %d\n", p->name, p->type->size, offset));
		p->x.offset = q->x.offset = offset;
		p->x.name = q->x.name = stringf("%d", p->x.offset);
		p->sclass = q->sclass = AUTO;
	}
	argsize = offset;
	offset = maxoffset = 0;

	gencode(caller, callee);

	framesize = maxoffset;
	print("asm.push()\n");

	// Save any registers that are used by this function.
	debug(fprint(stderr, "(used registers: %x)\n", usedmask[IREG]));
	saversize = 0;
	unsigned entermask = 0, leavemask = 0;
	for (unsigned e = 1 << 1,l = 1 << 15, n = 1; n < 16; e <<= 1, l >>= 1, n++) {
		if (usedmask[IREG] & e) {
			entermask |= e, leavemask |= l;
			saversize += 2;
		}
	}
	if (entermask != 0) {
		print("asm.ldwi(0x%x)\n", entermask);
		print("asm.call('enter')\n");
	}

	if (framesize > 0) {
		if (framesize < 256) {
			print("asm.ldw('sp')\n");
			print("asm.subi(%d)\n", framesize);
		} else {
			print("asm.ldwi(%d)\n", -framesize);
			print("asm.addw('sp')\n");
		}
		print("asm.stw('sp')\n");
	}

	debug(fprintf(stderr, "framesize: %d, saversize: %d, argsize: %d\n", framesize, saversize, argsize));
	emitcode();

	if (framesize > 0) {
		print("asm.%s(%d)\n", framesize < 256 ? "ldi" : "ldwi", framesize);
		print("asm.addw('sp')\n");
		print("asm.stw('sp')\n");
	}
	if (leavemask != 0) {
		print("asm.ldwi(0x%x)\n", leavemask);
		print("asm.call('leave')\n");
	}
	if (argsize > 0) {
		print("asm.%s(%d)\n", argsize < 256 ? "ldi" : "ldwi", argsize);
		print("asm.addw('sp')\n");
		print("asm.stw('sp')\n");
	}

	if (f->type->type != voidtype) {
		print("asm.ldw('rv')\n");
	}

	print("asm.pop()\n");
	print("asm.ret()\n");

	infunction = 0;
}
static void defsymbol(Symbol p) {
	if (p->scope >= LOCAL && p->sclass == STATIC) {
		p->x.name = stringf(".S%d", genlabel(1));
	} else if (p->generated) {
		p->x.name = stringf(".L%s", p->name);
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
		q->x.offset = p->x.offset - n;
		q->x.name = stringd(q->x.offset);
	}
}
static void defconst(int suffix, int size, Value v) {
	switch (suffix) {
	case I:
	case U:
	case P:
	case F:
		print("asm.d%c(%D)\n", size == 1 ? 'b' : 'w', v.i);
		break;
	default:
		assert(0);
	}
}
static void defaddress(Symbol p) {
	print("asm.d%c('%s')\n", p->scope == LABELS ? 'c' : 'w', p->x.name);
}
static void defstring(int n, char* str) {
	print("asm.dx([");
	for (int x = 0; x < n; x++) {
		print("%s0x%b", x == 0 ? "" : ", ", str[x]);
	}
	print("]);\n");
}
static void export(Symbol p) {
	// Nothing to do
}
static void import(Symbol p) {
	// Nothing to do
}
static void global(Symbol p) {
	print("asm.%s('%s')\n", infunction ? "glob" : "defun", p->x.name);
}
static void space(int n) {
	print("asm.dx([0] * %d)\n", n);
}
Interface gt1IR = {
        //size align outofline
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
	0,        /* wants_argb */
	1,        /* left_to_right */
	0,        /* wants_dag */
	1,        /* unsigned_char */
	NULL /*address*/, // 2019-04-30 (marcelk) TODO: Enable when asm.py can evaluate symbol offsets
	blockbeg,
	blockend,
	defaddress,
	defconst,
	defstring,
	defsymbol,
	emit,
	export,
	function,
	gengt1,
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
