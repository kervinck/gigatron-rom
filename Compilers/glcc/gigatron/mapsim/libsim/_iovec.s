def scope():

    def code_write():
        nohop()
        label('_sim_write');
        _LDI('errno');STW('sysArgs0')
        _LDI(0xff02);STW('sysFn')
        SYS(36)
        RET()

    def code_read():
        nohop()
        label('_sim_read');
        _LDI('errno');STW('sysArgs0')
        _LDI(0xff03);STW('sysFn')
        SYS(36)
        RET()

    def code_lseek():
        nohop()
        label('_sim_lseek');
        _LDI('errno');STW('sysArgs0')
        _LDI(0xff04);STW('sysFn')
        SYS(36)
        RET()

    def code_close():
        nohop()
        label('_sim_close');
        _LDI('errno');STW('sysArgs0')
        _LDI(0xff05);STW('sysFn')
        SYS(36)
        RET()
        
    def code_errno():
        align(2)
        label('errno')
        space(2)
        
    def code_svec():
        align(2)
        label('_sim_svec')
        words('_default_flsbuf')
        words('_sim_write')
        words('_default_filbuf')
        words('_sim_read')
        words('_sim_lseek')
        words('_sim_close')
        
    module(name='_iovec.c',
           code=[ ('EXPORT', '_sim_svec'),
                  ('IMPORT', '_default_filbuf'),
                  ('IMPORT', '_default_flsbuf'),
                  ('CODE', '_sim_write', code_write),
                  ('CODE', '_sim_read', code_read),
                  ('CODE', '_sim_lseek', code_lseek),
                  ('CODE', '_sim_close', code_close),
                  ('COMMON', 'errno', code_errno, 2, 2),
                  ('DATA', '_sim_svec', code_svec, 12, 2) ] )

    def code_openf():
        label('_openf')
        _LDI('errno');STW('sysArgs0')
        _LDI('_sim_svec');STW('sysArgs2')
        _LDI(0xff06);STW('sysFn')
        SYS(36)
        RET()

    module(name='_openf.c',
           code=[ ('EXPORT', '_openf'),
                  ('IMPORT', '_sim_svec'),
                  ('CODE', '_openf', code_openf) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
