def scope():

    def code_write():
        nohop()
        label('_sim_write');
        LDWI('errno');STW('sysArgs0')
        _LDI(0xff02);STW('sysFn')
        SYS(36)
        RET()

    def code_read():
        nohop()
        label('_sim_read');
        LDWI('errno');STW('sysArgs0')
        _LDI(0xff03);STW('sysFn')
        SYS(36)
        RET()

    def code_flush():
        nohop()
        label('_sim_flush');
        LDWI('errno');STW('sysArgs0')
        _LDI(0xff05);STW('sysFn')
        SYS(36)
        RET()

    def code_lseek():
        nohop()
        label('_sim_lseek');
        LDWI('errno');STW('sysArgs0')
        _LDI(0xff04);STW('sysFn')
        SYS(36)
        RET()

    def code_errno():
        align(2)
        label('errno')
        space(2)
        
    def code_iovec():
        align(2)
        label('_sim_iovec')
        words('_sim_read')
        words('_sim_write')
        words('_sim_flush')
        words('_sim_lseek')
        
    module(name='_iovec.c',
           code=[ ('EXPORT', '_sim_iovec'),
                  ('CODE', '_sim_write', code_write),
                  ('CODE', '_sim_read', code_read),
                  ('CODE', '_sim_flush', code_flush),
                  ('CODE', '_sim_lseek', code_lseek),
                  ('COMMON', 'errno', code_errno, 2, 2),
                  ('DATA', '_sim_iovec', code_iovec, 12, 2) ] )

    def code_openf():
        label('_openf')
        _LDI('errno');STW('sysArgs0')
        _LDI('_sim_iovec');STW('sysArgs2')
        _LDI(0xff06);STW('sysFn')
        SYS(36)
        RET()

    module(name='_openf.c',
           code=[ ('EXPORT', '_openf'),
                  ('IMPORT', '_sim_iovec'),
                  ('CODE', '_openf', code_openf) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
