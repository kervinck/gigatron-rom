                ; convert equal to conditional operator into a logical 1, (boolean conversion)
convertEqOp     BEQ     convertEq_1
                LDI     0
                RET
convertEq_1     LDI     1
                RET

                
                ; convert not equal to conditional operator into a logical 1, (boolean conversion)
convertNeOp     BNE     convertNe_1
                LDI     0
                RET
convertNe_1     LDI     1
                RET

                
                ; convert less than or equal to conditional operator into a logical 1, (boolean conversion)
convertLeOp     BLE     convertLe_1
                LDI     0
                RET
convertLe_1     LDI     1
                RET


                ; convert greater than or equal to conditional operator into a logical 1, (boolean conversion)
convertGeOp     BGE     convertGe_1
                LDI     0
                RET
convertGe_1     LDI     1
                RET

                
                ; convert less than conditional operator into a logical 1, (boolean conversion)
convertLtOp     BLT     convertLt_1
                LDI     0
                RET
convertLt_1     LDI     1
                RET

                
                ; convert greater than conditional operator into a logical 1, (boolean conversion)
convertGtOp     BGT     convertGt_1
                LDI     0
                RET
convertGt_1     LDI     1
                RET
