                ; convert equal to into a boolean
convertEqOp     BEQ     convertEq_1
                LDI     0
                RET
convertEq_1     LDI     1
                RET

                
                ; convert not equal to into a boolean
convertNeOp     BNE     convertNe_1
                LDI     0
                RET
convertNe_1     LDI     1
                RET

                
                ; convert less than or equal to into a boolean
convertLeOp     BLE     convertLe_1
                LDI     0
                RET
convertLe_1     LDI     1
                RET


                ; convert greater than or equal to into a boolean
convertGeOp     BGE     convertGe_1
                LDI     0
                RET
convertGe_1     LDI     1
                RET

                
                ; convert less than into a boolean
convertLtOp     BLT     convertLt_1
                LDI     0
                RET
convertLt_1     LDI     1
                RET

                
                ; convert greater than into boolean
convertGtOp     BGT     convertGt_1
                LDI     0
                RET
convertGt_1     LDI     1
                RET
