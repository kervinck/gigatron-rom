" Vim syntax file
" Language: Gigatron vCPU Language
" Maintainer: Chris Lord
" Latest Revision: 6th May 2018

if exists("b:current_syntax")
  finish
endif

" Keywords
syn match gclComparators '\(if[<>]=\?0\|if<>0\|if=0\)\(loop\)\?'
syn keyword gclKeywords else do loop def ret push pop call
syn keyword gclVersion gcl0x gcl1

" Regions
syn region gclComment start="{" end="}" contains=gclComment,@Spell

" Constants
syn match gclNumber '\<\d\+'
syn match gclNumber '\$[0-9a-fA-F]\+'
syn match gclData '\<\d\+#'
syn match gclData '\$[0-9a-fA-F]\+#'
syn match gclSegment '^\d\+:'
syn match gclSegment '^\$[0-9a-fA-F]\+:'
syn match gclOperator '+'
syn match gclOperator '-'
syn match gclOperator '='
syn match gclOperator '<'
syn match gclOperator '>'
syn match gclOperator '!'
syn match gclOperator '%'
syn match gclOperator '&'
syn match gclOperator '\^'
syn match gclOperator '|'
syn match gclSysFn '\\memSize'
syn match gclSysFn '\\bootCount'
syn match gclSysFn '\\bootCheck'
syn match gclSysFn '\\entropy'
syn match gclSysFn '\\videoY'
syn match gclSysFn '\\frameCount'
syn match gclSysFn '\\serialRaw'
syn match gclSysFn '\\buttonState'
syn match gclSysFn '\\xout'
syn match gclSysFn '\\xoutMask'
syn match gclSysFn '\\vPC'
syn match gclSysFn '\\vAC'
syn match gclSysFn '\\vLR'
syn match gclSysFn '\\vSP'
syn match gclSysFn '\\romType'
syn match gclSysFn '\\sysFn'
syn match gclSysFn '\\sysArgs[0-8]'
syn match gclSysFn '\\soundTimer'
syn match gclSysFn '\\ledTimer'
syn match gclSysFn '\\ledState'
syn match gclSysFn '\\ledTempo'
syn match gclSysFn '\\vars'
syn match gclSysFn '\\SYS_Reset_36'
syn match gclSysFn '\\SYS_Exec_88'
syn match gclSysFn '\\SYS_Out_22'
syn match gclSysFn '\\SYS_In_24'
syn match gclSysFn '\\SYS_NextByteIn'
syn match gclSysFn '\\SYS_Random_34'
syn match gclSysFn '\\SYS_LSRW7_30'
syn match gclSysFn '\\SYS_LSRW8_24'
syn match gclSysFn '\\SYS_LSLW8_24'
syn match gclSysFn '\\SYS_Draw4_30'
syn match gclSysFn '\\SYS_VDrawBits_134'
syn match gclSysFn '\\SYS_LSRW1_48'
syn match gclSysFn '\\SYS_LSRW2_52'
syn match gclSysFn '\\SYS_LSRW3_52'
syn match gclSysFn '\\SYS_LSRW4_50'
syn match gclSysFn '\\SYS_LSRW5_50'
syn match gclSysFn '\\SYS_LSRW6_48'
syn match gclSysFn '\\SYS_LSLW4_46'
syn match gclSysFn '\\SYS_Read3_40'
syn match gclSysFn '\\SYS_Unpack_56'
syn match gclSysFn '\\SYS_PayloadCopy_34'

let b:current_syntax="gcl"

hi def link gclKeywords Statement
hi def link gclVersion Identifier
hi def link gclSysFn Identifier
hi def link gclComparators Statement
hi def link gclComment Comment
hi def link gclNumber Constant
hi def link gclData Constant
hi def link gclSegment Macro
hi def link gclOperator Operator
