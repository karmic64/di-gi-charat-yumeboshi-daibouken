
	.section bank_1_0


triclyde_head_hook
	lda $03
	pha
	eor #3 ^ 1
	sta $03
	jsr $9cf2
	pla
	sta $03
	rts
	
	
	.send
	
	
	
	;;;;;;;;;;;;;;;;;;;;;;
	* = rom_addr(1,$a817)
	jsr triclyde_head_hook
	* = rom_addr(1,$a84e)
	jsr triclyde_head_hook
	
	
	
	;;;;;;;;;;;;;;;;;;;;;;
	;triclyde's shots
	* = rom_addr(FIXED_BANK,$f50b)
	.byte $13
	
	
	