def scope():

    # -----------------------------------------------
    # Long constants

    def code_lzero():
        label('_lzero')
        bytes(0,0,0,0) # 0L

    module(name='_lzero.s',
           code=[ ('EXPORT', '_lzero'),
                  ('DATA', '_lzero', code_lzero, 4, 2) ] )

    def code_lone():
        label('_lone')
        bytes(1,0,0,0) # 1L

    module(name='_lone.s',
           code=[ ('EXPORT', '_lone'),
                  ('DATA', '_lone', code_lone, 4, 2) ] )

    def code_ltwo():
        label('_ltwo')
        bytes(2,0,0,0) # 2.0F

    module(name='_ltwo.s',
           code=[ ('EXPORT', '_ltwo'),
                  ('DATA', '_ltwo', code_ltwo, 4, 2) ] )

    def code_lminus():
        label('_lminus')
        bytes(255,255,255,255) # -1L

    module(name='_lminus.s',
           code=[ ('EXPORT', '_lminus'),
                  ('DATA', '_lminus', code_lminus, 4, 2) ] )

    # -----------------------------------------------
    # Floating point constants

    def code_fzero():
        label('_fzero')
        bytes(0,0,0,0,0) # 0.0F

    module(name='_fzero.s',
           code=[ ('EXPORT', '_fzero'),
                  ('DATA', '_fzero', code_fzero, 5, 1) ] )

    def code_fone():
        label('_fone')
        bytes(129,0,0,0,0) # 1.0F

    module(name='_fone.s',
           code=[ ('EXPORT', '_fone'),
                  ('DATA', '_fone', code_fone, 5, 1) ] )

    def code_fhalf():
        label('_fhalf')
        bytes(128,0,0,0,0) # 0.5F

    module(name='_fhalf.s',
           code=[ ('EXPORT', '_fhalf'),
                  ('DATA', '_fhalf', code_fhalf, 5, 1) ] )

    def code_ftwo():
        label('_ftwo')
        bytes(130,0,0,0,0) # 2.0F

    module(name='_ftwo.s',
           code=[ ('EXPORT', '_ftwo'),
                  ('DATA', '_ftwo', code_ftwo, 5, 1) ] )

    def code_fminus():
        label('_fminus')
        bytes(129,128,0,0,0) # -1.0F

    module(name='_fminus.s',
           code=[ ('EXPORT', '_fminus'),
                  ('DATA', '_fminus', code_fminus, 5, 1) ] )

    def code_fpi():
        label('_pi')
        bytes(130,73,15,218,162); # 0.785398 * 4
        label('_pi_over_2')
        bytes(129,73,15,218,162); # 0.785398 * 2
        label('_pi_over_4')
        bytes(128,73,15,218,162); # 0.785398
        
    module(name='_fpi.s',
           code=[ ('EXPORT', '_pi'),
                  ('EXPORT', '_pi_over_2'),
                  ('EXPORT', '_pi_over_4'),
                  ('DATA', '_pi', code_fpi, 15, 1) ] )
    
scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
