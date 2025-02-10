

def scope():

    if args.cpu < 7:
        error("gt_threads require cpu>=7")

    maxslice = 4

    # ----------------------------------------
    # Scheduler

    # -- vars

    def code_vars0():
        align(2)
        # current thread
        # hi=vIrqCtx lo=0xaa
        label('_gt_ct')
        words(0)
        # thread associated with next active timeout
        label('_gt_nt')
        words(0)

    def code_vars1():
        align(4)
        # tick count at next irq
        label('_gt_clk')
        words(0,0)

    # -- clock manipulation

    def code_clock():
        nohop()
        # calculate current tick count
        label('gt_clock')
        LD('frameCount');MOVQB(0xff,vACH)
        label('_gt_adjclock')
        STW(LAC);LDSB(vACH);LDSB(vACH);STW(LAC+2)
        _LDI('_gt_clk');ADDL();
        RET()

    # -- switch to thread T0

    def code_switch():
        nohop()
        label('_gt_switch')
        # t0->wchan = 0
        LDW(T0);XORI(0xaa^0xac);DOKEI(0)
        # glcc registers
        # if ct != t0, save regs, set ct, set virqctx
        LDW('_gt_ct');XORW(T0);_BEQ('.s1')
        LDW('_gt_ct')
        XORI(0xaa^0xb0);STW(T2);MOVQW(R0,T3);COPYN(24*2)
        LDW(T0);STW('_gt_ct')
        XORI(0xaa^0xb0);STW(T3);MOVQW(R0,T2);COPYN(24*2)
        _LDI('vIrqCtx_v7');POKEA(T0+1)
        label('.s1')
        # if (nt==0 || (slice = nt->wclock[0] - clock) >= maxslice)
        #   slice = maxslice
        LDW('_gt_nt');_BEQ('.s2')
        XORI(0xaa^0xae);DEEK();LDLAC();_LDI('_gt_clk');SUBL()
        LDW(LAC+2);_BNE('.s2');LDW(LAC);CMPIU(maxslice);_BLE('.s3')
        label('.s2')
        MOVQW(maxslice,LAC)
        label('.s3')
        # clk += slice
        # vIrqCtx = t>>8
        # restore vcpu context with frameCount adjustment
        LDI(0);STW(LAC+2);SUBW(LAC);ST(T1)
        _LDI('_gt_clk');ADDL();STLAC()
        LDW(T0+1);VRESTORE()

    # -- update nt if thread T0 is next to time out

    def code_chknt():
        nohop()
        label('_gt_chknt')
        LDW(T0);XORI(0xaa^0xac);DEEK();_BEQ('.uret')
        LDW(T0);XORI(0xaa^0xae);DEEK();_BEQ('.uret')
        LDLAC()
        LDW('_gt_nt');_BEQ('.unew')
        XORI(0xaa^0xae);DEEK();SUBL();LDW(LAC+2);_BGE('.uret')
        label('.unew')
        LDW(T0);STW('_gt_nt')
        label('.uret')
        RET()

    # -- interrupt code and scheduler

    def code_irq():
        nohop()
        # .pick:
        #   do {
        #     t0 = t0->next
        #     if (!t0->wchan || !t0->wchan[0]) goto switch
        #   while(t0 != current)
        label('.loop')
        LDW(T0);DEEK();STW(T0)
        XORI(0xaa^0xac);DEEK();JEQ('_gt_switch')
        PEEK();JEQ('_gt_switch')
        LDW(T0);XORW(T1);JNE('.loop')
        #
        # .idle:
        #   clk += frameCount; frameCount=0
        label('.idle')
        LDI('frameCount');EXBA(0)
        CALLI('_gt_adjclock');STLAC()
        #
        # _gt_irq:
        #   t0 = current
        #   if (nt == 0 || clk - nt->wclock[0] < 0) goto loop
        label('_gt_irq')
        LDW('_gt_ct');STW(T0);STW(T1);DEEK();_BNE('.irq1')
        LDWI('_gt_mainthread');STW(T0);STW(T1)  # happens when ct is killed
        label('.irq1')
        LDW('_gt_nt');_BEQ('.loop')
        _LDI('_gt_clk');LDLAC()
        LDW('_gt_nt');XORI(0xaa^0xae);DEEK();SUBL()
        LDW(LAC+2);_BLT('.loop')
        #
        # .timeout
        #   nt->wclk = 0
        #   recompute nt
        #   goto switch
        label('.timeout')
        LDW('_gt_nt');STW(T0);XORI(0xaa^0xae);DOKEQ(0)
        CALLI('_gt_fixnt')
        CALLI('_gt_switch')

    # -- yield

    def code_yield():
        nohop()
        # gt_join
        #   wclk=0 wchan=r8
        label('gt_join')
        LDW('_gt_ct');XORI(0xaa^0xae);DOKEQ(0);XORI(0xae^0xac);DOKEA(R8)
        # gt_yield
        #   call sched with vac=chknt
        label('gt_yield')
        LDWI('_gt_chknt');STW(T3)
        # gt_sched
        #   t0 = current
        #   vsave, fix saved pc, update clock
        #   call t3, and jump into irq routine
        label('_gt_sched')
        LDW('_gt_ct');STW(T0);LD(vACH);VSAVE()
        LDW(T0);XORI(0xe2^0xaa);DOKEI(v('.ret')-2)
        LDI('frameCount');EXBA(0);ADDHI(0xff)
        CALLI('_gt_adjclock');STLAC()
        CALL(T3)
        CALLI('_gt_irq')
        label('.ret')
        RET()


    # --- exit / kill

    def code_kill():
        nohop()
        # gt_exit
        #  kill ct
        label('gt_exit')
        MOVW('_gt_ct', R8)
        # gt_kill
        #  t3 = .kill2
        #  goto sched if R8 != mainthread else return -1
        label('gt_kill')
        MOVIW('.kill2',T3)
        LDWI('_gt_mainthread');XORW(R8);JNE('_gt_sched')
        _LDI(-1);RET()
        # .kill2
        #  search thread, goto kill3 with prev in t0
        #  else plant vAC=-1 in saved context
        label('.kill2')
        LDW(T0);DEEK();XORW(R8);_BEQ('.kill3')
        XORW(R8);STW(T0);XORW('_gt_ct');_BNE('.kill2')
        LDW('_gt_ct');XORI(0xaa^0xe0);DOKEI(-1);RET()
        label('.kill3')
        # remove thread r8 from circular list
        # set r8->next = 0
        # plant vAC=0 in saved context
        # check nt, update if needed
        LDW(R8);DEEK();DOKE(T0)
        LDI(0);DOKE(R8)
        LDW('_gt_ct');STW(T0);XORI(0xaa^0xe0);DOKEQ(0)
        LDW('_gt_nt');XORW(R8);JNE('.ret')
        # recompute next thread to time out
        # assume t0 is a running thread
        label('_gt_fixnt')
        MOVW(vLR,T2)
        MOVQW(0,'_gt_nt')
        MOVW(T0,T1)
        label('.f1')
        CALLI('_gt_chknt')
        LDW(T0);DEEK();STW(T0)
        XORW(T1);_BNE('.f1')
        CALL(T2) # return!

    # -- main thread

    def code_mainthread():
        label('_gt_mainthread')
        space(0x100-0xaa)

    # -- init/fini

    def code_init():
        label('_gt_init')
        LDWI('_gt_mainthread')
        STW('_gt_ct')
        DOKEA(vAC)
        XORI(0xaa^0xff)
        POKEQ(1)
        LDWI('vIrqCtx_v7')
        POKEA(v('_gt_ct')+1)
        LDWI('vIRQ_v5')
        DOKEI('_gt_irq')
        LDWI(v('_gt_clk')+1)
        POKEQ(1)
        LDI(0)
        ST('frameCount')
        JEQ('gt_yield')

    def code_initrec():
        align(2);
        label('__glink_magic_init')
        words('_gt_init',0)

    def code_fini():
        label('_gt_fini')
        LDWI('vIRQ_v5')
        DOKEQ(0)
        LDWI('vIrqCtx_v7')
        POKEQ(0)
        RET()

    def code_finirec():
        align(2);
        label('__glink_magic_fini')
        words('_gt_fini',0)

    module(name='gt_sched.s',
           code=[('EXPORT', '_gt_irq'),
                 ('EXPORT', '_gt_sched'),
                 ('EXPORT', '_gt_chknt'),
                 ('EXPORT', 'gt_clock'),
                 ('EXPORT', 'gt_join'),
                 ('EXPORT', 'gt_yield'),
                 ('EXPORT', 'gt_kill'),
                 ('EXPORT', 'gt_exit'),
                 # vars
                 ('EXPORT', '_gt_ct'),
                 ('EXPORT', '_gt_clk'),
                 ('DATA', '_gt_ct', code_vars0, 4, 2),
                 ('PLACE', '_gt_ct', 0x00, 0xff),
                 ('DATA', '_gt_clk', code_vars1, 10, 4),
                 # code
                 ('CODE', 'gt_clock', code_clock),
                 ('CODE', '_gt_switch', code_switch),
                 ('CODE', '_gt_chknt', code_chknt),
                 ('CODE', '_gt_irq', code_irq),
                 ('CODE', 'gt_yield', code_yield),
                 ('CODE', 'gt_kill', code_kill),
                 ('BSS', '_gt_mainthread', code_mainthread, 0x100-0xaa, 2),
                 ('OFFSET', '_gt_mainthread', 0xaa),
                 ('CODE', '_gt_init', code_init),
                 ('CODE', '_gt_fini', code_fini),
                 ('DATA', '__glink_magic_init', code_initrec, 4, 2),
                 ('DATA', '__glink_magic_fini', code_finirec, 4, 2) ] )



    # ----------------------------------------
    # Return current thread or thread info

    def code_current():
        nohop()
        label('gt_current')
        LDW('_gt_ct');RET()

    module(name='gt_current.s',
           code=[('EXPORT', 'gt_current'),
                 ('IMPORT', '_gt_ct'),
                 ('CODE', 'gt_current', code_current) ] )


    # -----------------------------------------
    # Start

    def code_start():
        # gt_thread_p gt_start(gt_thread_p tp, void *stk, int stksz,
	#                      void(*f)(void*), void *arg)
        nohop()
        label('gt_start')
        # checks
        LD(R8);XORI(0xaa);_BNE('.s0')
        LDW(R8);DEEK();BNE('.s0')
        # prepare context
        LD(R11);SUBI(2);ST(R11)           # initial pc
        LDW(R9);ADDW(R10);SUBI(2);STW(R9) # initial sp
        ANDI(0xfc);ST(R9)                 # long-align sp
        LDW(R8);XORI(0xaa^0xac);DOKEQ(0)  # clear wchan
        XORI(0xac^0xae);DOKEQ(0)          # clear wclk
        XORI(0xae^0xc0);DOKEA(R12)        # t.r8 := arg
        XORI(0xc0^0xe2);DOKEA(R11)        # t.pc
        XORI(0xe2^0xe4);DOKEI('gt_exit')  # t.vlr
        XORI(0xe4^0xe6);DOKEA(R9)         # t.sp
        XORI(0xe6^0xfd);POKEQ(2)          # vCpuSelect := 2
        XORI(0xfd^0xff);POKEQ(1)          # vIrqMask
        # goto sched
        LDWI('.s1');STW(T3)
        JNE('_gt_sched')
        # .s1
        label('.s1')
        # install return value in current thread context
        LDW(T0);XORI(0xaa^0xe0);DOKEA(R8)
        # install thread in circular list
        LDW(T0);DEEK();DOKE(R8)
        LDW(R8);DOKE(T0)
        # return
        label('.s0')
        LDI(0);RET()

    module(name='gt_start.s',
           code=[('EXPORT', 'gt_start'),
                 ('IMPORT', '_gt_sched'),
                 ('CODE', 'gt_start', code_start) ] )


    # -----------------------------------------
    # Wait functions

    def code_wait():
        nohop()
        label('gt_wait')
        MOVL(L8,LAC)
        label('_gt_wait')
        LDW(SP);STLAC()
        LDW('_gt_ct');XORI(0xaa^0xae);DOKEA(SP);XORI(0xae^0xac);DOKEQ(0x80)
        JNE('gt_yield')

    module(name='gt_wait.s',
           code=[('EXPORT','gt_wait'),
                 ('EXPORT','_gt_wait'),
                 ('IMPORT','gt_yield'),
                 ('IMPORT','_gt_ct'),
                 ('CODE','gt_wait', code_wait) ] )

    def code_sleep():
        nohop()
        label('gt_sleep')
        PUSH();CALLI('gt_clock');POP()
        LDI(L8);ADDL();JNE('_gt_wait')

    module(name='gt_sleep.s',
           code=[('EXPORT','gt_sleep'),
                 ('IMPORT','_gt_wait'),
                 ('IMPORT','_gt_clock'),
                 ('CODE','gt_sleep', code_sleep) ] )


    # -----------------------------------------
    # Mutex functions

    def code_mutex_lock():
        nohop()
        label('.m1')
        PUSH();CALLI('gt_join');POP()  # same code as join!
        label('gt_mutex_lock')
        LDW(R8);EXBA(1);_BNE('.m1')
        RET()

    module(name='gt_mutex_lock.s',
           code=[('EXPORT', 'gt_mutex_lock'),
                 ('IMPORT', 'gt_join'),
                 ('CODE', 'gt_mutex_lock', code_mutex_lock) ] )

    def code_mutex_trylock():
        nohop()
        label('gt_mutex_trylock')
        LDW(R8);EXBA(1);_BNE('.m1')
        LDWI(-1)
        label('.m1')
        RET()

    module(name='gt_mutex_trylock.s',
           code=[('EXPORT', 'gt_mutex_trylock'),
                 ('CODE', 'gt_mutex_trylock', code_mutex_trylock) ] )


    def code_mutex_unlock():
        nohop()
        label('gt_mutex_unlock')
        label('gt_cond_signal')
        LDI(0);POKE(R8);RET()

    module(name='gt_mutex_unlock',
           code=[('EXPORT','gt_mutex_unlock'),
                 ('EXPORT','gt_cond_signal'),
                 ('CODE','gt_mutex_unlock', code_mutex_unlock) ] )


    # -----------------------------------------
    # Condvar functions

    def code_cond_wait():
        nohop()
        label('gt_cond_wait')
        MOVQW(0,R10)
        label('_gt_cond_wait')
        PUSH()
        MOVW(R8,R16);MOVW(R9,R8)
        # atomically unlock mutex and wait
        MOVIW('.cw0',T3);CALLI('_gt_sched')
        # relock mutex
        CALLI('gt_mutex_lock')
        # best effort to prevent other threads from waking up
        LDI(1);POKE(R16)
        tryhop(2);POP();RET()
        # atomic sequence
        label('.cw0')
        LDW('_gt_ct')
        XORI(0xaa^0xae);DOKEA(R10) # set wclk
        XORI(0xae^0xac);DOKEA(R16) # set wchan
        LDI(0);POKE(R8)            # unlock mutex
        LDI(1);POKE(R16)           # arm condvar
        JNE('_gt_chknt')

    module(name='gt_cond_wait.s',
           code=[('EXPORT', 'gt_cond_wait'),
                 ('EXPORT', '_gt_cond_wait'),
                 ('IMPORT', 'gt_mutex_lock'),
                 ('IMPORT', '_gt_ct'),
                 ('IMPORT', '_gt_sched'),
                 ('IMPORT', '_gt_chknt'),
                 ('CODE', 'gt_cond_wait', code_cond_wait) ] )

    def code_cond_timedwait():
        nohop()
        label('gt_cond_timedwait')
        _SP(4);DOKEA(L10);STW(R10);ADDI(2);DOKEA(L10+2)
        JNE('_gt_cond_wait')

    module(name='gt_cond_timedwait.s',
           code=[('EXPORT', 'gt_cond_timedwait'),
                 ('IMPORT', '_gt_cond_wait'),
                 ('CODE', 'gt_cond_timedwait', code_cond_timedwait) ] )


    # ----------------------------a------------
    # Replacement for libc functions clock
    # and _clock using _gt_clk instead of
    # their own irq handler.

    def code_clock():
        nohop()
        label('clock')
        label('_clock')
        PUSH()
        CALLI('gt_clock')
        LDW(LAC);POP();RET()

    module(name='clock.s',
           code=[('EXPORT', 'clock'),
                 ('EXPORT', '_clock'),
                 ('IMPORT', 'gt_clock'),
                 ('CODE', 'clock', code_clock) ] )



# create all the modules
scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:


