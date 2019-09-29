frogup_draw		STW		scratch
			LDWI	frogup_bak
			STW		scratch2
			LD		scratch
			ADDI	4
			ST		scratch
			LDW		scratch
			DEEK
			DOKE	scratch2
			INC		scratch2
			INC		scratch2
			LDW		cst_0404
			DOKE	scratch
			INC		scratch
			INC		scratch
			LDW		scratch
			PEEK
			POKE	scratch2
			INC		scratch2
			LDI		$04
			POKE	scratch
			LDW		scratch
			ADDI	253
			STW		scratch
			DEEK
			DOKE	scratch2
			INC		scratch2
			INC		scratch2
			LDW		cst_1004
			DOKE	scratch
			INC		scratch
			INC		scratch
			LDW		scratch
			DEEK
			DOKE	scratch2
			INC		scratch2
			INC		scratch2
			LDW		cst_1004
			DOKE	scratch
			INC		scratch
			INC		scratch
			LDW		scratch
			PEEK
			POKE	scratch2
			INC		scratch2
			LDI		$04
			POKE	scratch
			LDW		scratch
			ADDI	252
			STW		scratch
			DEEK
			DOKE	scratch2
			INC		scratch2
			INC		scratch2
			LDW		cst_0504
			DOKE	scratch
			INC		scratch
			INC		scratch
			LDW		scratch
			DEEK
			DOKE	scratch2
			INC		scratch2
			INC		scratch2
			LDW		cst_0505
			DOKE	scratch
			INC		scratch
			INC		scratch
			LDW		scratch
			PEEK
			POKE	scratch2
			INC		scratch2
			LDI		$04
			POKE	scratch
			LDW		scratch
			ADDI	252
			STW		scratch
			DEEK
			DOKE	scratch2
			INC		scratch2
			INC		scratch2
			LDW		cst_0504
			DOKE	scratch
			INC		scratch
			INC		scratch
			LDW		scratch
			DEEK
			DOKE	scratch2
			INC		scratch2
			INC		scratch2
			LDW		cst_0505
			DOKE	scratch
			INC		scratch
			INC		scratch
			LDW		scratch
			PEEK
			POKE	scratch2
			INC		scratch2
			LDI		$04
			POKE	scratch
			LDW		scratch
			ADDI	250
			STW		scratch
			DEEK
			DOKE	scratch2
			INC		scratch2
			INC		scratch2
			LDW		cst_0404
			DOKE	scratch
			INC		scratch
			INC		scratch
			LDW		scratch
			DEEK
			DOKE	scratch2
			INC		scratch2
			INC		scratch2
			LDW		cst_0404
			DOKE	scratch
			INC		scratch
			INC		scratch
			INC		scratch
			LDW		scratch
			PEEK
			POKE	scratch2
			INC		scratch2
			LDI		$04
			POKE	scratch
			LDW		scratch
			ADDI	250
			STW		scratch
			PEEK
			POKE	scratch2
			INC		scratch2
			LDI		$04
			POKE	scratch
			RET		; bytes = 255
frogup_draw0 			INC		scratch
			INC		scratch
			LDW		scratch
			PEEK
			POKE	scratch2
			INC		scratch2
			LDI		$04
			POKE	scratch
			LDW		scratch
			ADDI	254
			STW		scratch
			PEEK
			POKE	scratch2
			INC		scratch2
			LDI		$04
			POKE	scratch
			RET		; 26 pixels in 17 copyops - 1 pages + 30 bytes

frogup_restore	STW		scratch
			LDWI	frogup_bak
			STW		scratch2
			LD		scratch
			ADDI	4
			ST		scratch
			LDW		scratch2
			DEEK
			DOKE	scratch
			INC		scratch2
			INC		scratch2
			INC		scratch
			INC		scratch
			LDW		scratch2
			PEEK
			POKE	scratch
			INC		scratch2
			LDW		scratch
			ADDI	253
			STW		scratch
			LDW		scratch2
			DEEK
			DOKE	scratch
			INC		scratch2
			INC		scratch2
			INC		scratch
			INC		scratch
			LDW		scratch2
			DEEK
			DOKE	scratch
			INC		scratch2
			INC		scratch2
			INC		scratch
			INC		scratch
			LDW		scratch2
			PEEK
			POKE	scratch
			INC		scratch2
			LDW		scratch
			ADDI	252
			STW		scratch
			LDW		scratch2
			DEEK
			DOKE	scratch
			INC		scratch2
			INC		scratch2
			INC		scratch
			INC		scratch
			LDW		scratch2
			DEEK
			DOKE	scratch
			INC		scratch2
			INC		scratch2
			INC		scratch
			INC		scratch
			LDW		scratch2
			PEEK
			POKE	scratch
			INC		scratch2
			LDW		scratch
			ADDI	252
			STW		scratch
			LDW		scratch2
			DEEK
			DOKE	scratch
			INC		scratch2
			INC		scratch2
			INC		scratch
			INC		scratch
			LDW		scratch2
			DEEK
			DOKE	scratch
			INC		scratch2
			INC		scratch2
			INC		scratch
			INC		scratch
			LDW		scratch2
			PEEK
			POKE	scratch
			INC		scratch2
			LDW		scratch
			ADDI	250
			STW		scratch
			LDW		scratch2
			DEEK
			DOKE	scratch
			INC		scratch2
			INC		scratch2
			INC		scratch
			INC		scratch
			LDW		scratch2
			DEEK
			DOKE	scratch
			INC		scratch2
			INC		scratch2
			INC		scratch
			INC		scratch
			INC		scratch
			LDW		scratch2
			PEEK
			POKE	scratch
			INC		scratch2
			LDW		scratch
			ADDI	250
			STW		scratch
			LDW		scratch2
			PEEK
			POKE	scratch
			INC		scratch2
			INC		scratch
			INC		scratch
			LDW		scratch2
			PEEK
			POKE	scratch
			INC		scratch2
			LDW		scratch
			ADDI	254
			STW		scratch
			LDW		scratch2
			PEEK
			POKE	scratch
			INC		scratch2
			RET		; 0 pages + 228 bytes


frogdown_draw		STW		scratch
			LDWI	frogdown_bak
			STW		scratch2
			LDWI	0x0203
			ADDW	scratch
			STW		scratch
			DEEK
			DOKE	scratch2
			INC		scratch2
			INC		scratch2
			LDW		cst_0404
			DOKE	scratch
			INC		scratch
			INC		scratch
			LDW		scratch
			PEEK
			POKE	scratch2
			INC		scratch2
			LDI		$04
			POKE	scratch
			LDW		scratch
			ADDI	253
			STW		scratch
			DEEK
			DOKE	scratch2
			INC		scratch2
			INC		scratch2
			LDW		cst_1004
			DOKE	scratch
			INC		scratch
			INC		scratch
			LDW		scratch
			DEEK
			DOKE	scratch2
			INC		scratch2
			INC		scratch2
			LDW		cst_1004
			DOKE	scratch
			INC		scratch
			INC		scratch
			LDW		scratch
			PEEK
			POKE	scratch2
			INC		scratch2
			LDI		$04
			POKE	scratch
			LDW		scratch
			ADDI	252
			STW		scratch
			DEEK
			DOKE	scratch2
			INC		scratch2
			INC		scratch2
			LDW		cst_0404
			DOKE	scratch
			INC		scratch
			INC		scratch
			LDW		scratch
			DEEK
			DOKE	scratch2
			INC		scratch2
			INC		scratch2
			LDW		cst_0405
			DOKE	scratch
			INC		scratch
			INC		scratch
			LDW		scratch
			PEEK
			POKE	scratch2
			INC		scratch2
			LDI		$04
			POKE	scratch
			LDW		scratch
			ADDI	251
			STW		scratch
			DEEK
			DOKE	scratch2
			INC		scratch2
			INC		scratch2
			LDW		cst_0404
			DOKE	scratch
			INC		scratch
			INC		scratch
			LDW		scratch
			DEEK
			DOKE	scratch2
			INC		scratch2
			INC		scratch2
			LDW		cst_0505
			DOKE	scratch
			INC		scratch
			INC		scratch
			LDW		scratch
			DEEK
			DOKE	scratch2
			INC		scratch2
			INC		scratch2
			LDW		cst_0405
			DOKE	scratch
			LDW		scratch
			ADDI	251
			STW		scratch
			DEEK
			DOKE	scratch2
			INC		scratch2
			INC		scratch2
			LDW		cst_0404
			DOKE	scratch
			INC		scratch
			INC		scratch
			INC		scratch
			LDW		scratch
			PEEK
			POKE	scratch2
			INC		scratch2
			LDI		$04
			POKE	scratch
			INC		scratch
			INC		scratch
			INC		scratch
			LDW		scratch
			DEEK
			DOKE	scratch2
			INC		scratch2
			INC		scratch2
			LDW		cst_0404
			DOKE	scratch
			RET		; bytes = 243
frogdown_draw0 			RET		; 24 pixels in 14 copyops - 1 pages + 0 bytes

frogdown_restore	STW		scratch
			LDWI	frogdown_bak
			STW		scratch2
			LDWI	515
			ADDW	scratch
			STW		scratch
			LDW		scratch2
			DEEK
			DOKE	scratch
			INC		scratch2
			INC		scratch2
			INC		scratch
			INC		scratch
			LDW		scratch2
			PEEK
			POKE	scratch
			INC		scratch2
			LDW		scratch
			ADDI	253
			STW		scratch
			LDW		scratch2
			DEEK
			DOKE	scratch
			INC		scratch2
			INC		scratch2
			INC		scratch
			INC		scratch
			LDW		scratch2
			DEEK
			DOKE	scratch
			INC		scratch2
			INC		scratch2
			INC		scratch
			INC		scratch
			LDW		scratch2
			PEEK
			POKE	scratch
			INC		scratch2
			LDW		scratch
			ADDI	252
			STW		scratch
			LDW		scratch2
			DEEK
			DOKE	scratch
			INC		scratch2
			INC		scratch2
			INC		scratch
			INC		scratch
			LDW		scratch2
			DEEK
			DOKE	scratch
			INC		scratch2
			INC		scratch2
			INC		scratch
			INC		scratch
			LDW		scratch2
			PEEK
			POKE	scratch
			INC		scratch2
			LDW		scratch
			ADDI	251
			STW		scratch
			LDW		scratch2
			DEEK
			DOKE	scratch
			INC		scratch2
			INC		scratch2
			INC		scratch
			INC		scratch
			LDW		scratch2
			DEEK
			DOKE	scratch
			INC		scratch2
			INC		scratch2
			INC		scratch
			INC		scratch
			LDW		scratch2
			DEEK
			DOKE	scratch
			INC		scratch2
			INC		scratch2
			LDW		scratch
			ADDI	251
			STW		scratch
			LDW		scratch2
			DEEK
			DOKE	scratch
			INC		scratch2
			INC		scratch2
			INC		scratch
			INC		scratch
			INC		scratch
			LDW		scratch2
			PEEK
			POKE	scratch
			INC		scratch2
			INC		scratch
			INC		scratch
			INC		scratch
			LDW		scratch2
			DEEK
			DOKE	scratch
			INC		scratch2
			INC		scratch2
			RET		; 0 pages + 195 bytes


