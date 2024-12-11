def scope():

    # The tree rebalancing function is frameless
    # but contains a lot of redundant code that
    # can be compressed in low level calls.
    # Compiling this right would require
    # interprocedural register allocation,
    # something well outside LCC's capabilities.
    #
    # struct avlnode_s {
    #   int height;
    #	avlnode_t *left, *right; }
    #
    #
    # void _avl_rebal(register avlnode_t ***sp)           R8
    # {
    # 	register avlnode_t **pelt, *elt, **r, *rtmp;      R9 R10 R11 R12
    # 	register int lh, rh, tmp;                         R13 R14 R15

    #     #define GETH do { \
    # 		lh = rh = 0;\
    # 		if (elt->left)\
    # 			lh = elt->left->height;\
    # 		if (elt->right)\
    # 			rh = elt->right->height;\
    # 	} while(0)

    def code_geth():
        label('.geth')
        PUSH()
        LDI(0);STW(R13);STW(R14)
        LDI(2);ADDW(R10);DEEK();_BEQ('.geth1')
        DEEK();STW(R13)
        label('.geth1')
        LDI(4);ADDW(R10);DEEK();_BEQ('.geth2')
        DEEK();STW(R14)
        label('.geth2')
        tryhop(2);POP();RET()

    # #define CALC do {\
    # 		GETH;\
    # 		tmp = 0;\
    # 		if (lh > tmp)\
    # 			tmp = lh;\
    # 		if (rh > tmp)\
    # 			tmp = rh;\
    # 		elt->height = tmp + 1;\
    # 	} while(0)

    def code_calc():
        label('.calc')
        PUSH()
        _MOVIW(0,R15);
        _CALLJ('.geth')
        LDW(R13);SUBW(R15);_BLE('.calc1')
        _MOVW(R13,R15)
        label('.calc1')
        LDW(R14);SUBW(R15);_BLE('.calc2')
        _MOVW(R14,R15)
        label('.calc2')
        LDI(1);ADDW(R15);DOKE(R10)
        tryhop(2);POP();RET()

    # #define LROT do {\
    # 		rtmp = (*r)->right;\
    # 		(*r)->right = rtmp->left;\
    # 		rtmp->left = (*r);\
    # 		elt = *r; CALC; \
    # 		elt = rtmp; CALC; \
    # 		*r = rtmp;\
    # 	} while(0)
    # #define RROT do {\
    # 		rtmp = (*r)->left;\
    # 		(*r)->left = rtmp->right;\
    # 		rtmp->right = (*r);\
    # 		elt = *r; CALC; \
    # 		elt = rtmp; CALC; \
    # 		*r = rtmp;\
    # 	} while(0)

    def code_rotate():
        label('.lrot')    # r = pelt; LROT
        PUSH()
        LDW(R9)
        label('.l1')
        STW(R11);DEEK();ADDI(4);STW(R22)
        DEEK();STW(R12);LDI(2)
        _BRA('.end')
        label('.lrotl')   # r = &(*pelt)->left; LROT
        PUSH()
        LDW(R9);DEEK();ADDI(2);_BRA('.l1')
        label('.rrotr')   # r = (*pelt)->right; RROT
        PUSH()
        LDW(R9);DEEK();ADDI(4);_BRA('.r1')
        label('.rrot')    # r = pelt; RROT
        PUSH()
        LDW(R9)
        label('.r1')
        STW(R11);DEEK();ADDI(2);STW(R22)
        DEEK();STW(R12);LDI(4)
        label('.end')
        ADDW(R12);STW(R21);DEEK();DOKE(R22)
        LDW(R11);DEEK();DOKE(R21)
        STW(R10);_CALLJ('.calc')
        _MOVW(R12,R10);_CALLJ('.calc')
        LDW(R12);DOKE(R11)
        tryhop(2);POP();RET()

    def code_rebal():
        label('__avl_rebal')
        PUSH()
        label('.loop')
        # while (pelt = *sp++) {
        # 	if (! (elt = *pelt))
        # 		continue;
        LDW(R8);DEEK();STW(R9)
        LDI(2);ADDW(R8);STW(R8)
        LDW(R9);_BNE('.rebal0')
        tryhop(2);POP();RET()
        label('.rebal0')
        DEEK();STW(R10)
        _BEQ('.loop')
        #	CALC;
        #	if (rh - lh == -2) {
        _CALLJ('.calc')
        LDW(R14);SUBW(R13)
        ADDI(2);_BNE('.rebal2')
        # 		elt = elt->left;
        # 		GETH;
        # 		if (rh - lh > 0)
        # 			{ r = &((*pelt)->left); LROT; }
        # 		r = pelt; RROT;
        LDI(2);ADDW(R10);DEEK();STW(R10)
        _CALLJ('.geth')
        LDW(R14);SUBW(R13);_BLE('.rebal1')
        _CALLJ('.lrotl')
        label('.rebal1')
        _CALLJ('.rrot')
        _BRA('.loop')
        label('.rebal2')
        #	} else if (rh - lh == +2) {
        XORI(4);_BNE('.loop')
        # 		elt = elt->right;
        # 		GETH;
        # 		if (rh - lh < 0)
        # 			{ r = &((*pelt)->right); RROT; }
        # 		r = pelt; LROT;
        LDI(4);ADDW(R10);DEEK();STW(R10)
        _CALLJ('.geth')
        LDW(R14);SUBW(R13);_BGE('.rebal3')
        _CALLJ('.rrotr')
        label('.rebal3')
        _CALLJ('.lrot')
        _BRA('.loop')

    module(name='_avl_bal.s',
           code=[('EXPORT', '__avl_rebal'),
                 ('CODE', '.geth', code_geth),
                 ('CODE', '.calc', code_calc),
                 ('CODE', '.rotate', code_rotate),
                 ('CODE', '__avl_rebal', code_rebal) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
