
def scope():
  if not 'USE_LIBC_PRINTF' in args.opts:
    def code0():
        nohop()
        label('printf')
        LDWI(0xff01);STW('sysFn');SYS(34)
        RET();

    module(name='printf.s',
           code=[ ('EXPORT', 'printf'),
                  ('CODE', 'printf', code0) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
