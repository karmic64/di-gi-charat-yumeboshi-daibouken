
	* = rom_addr(2,$803e)
	jsr sfx_0
	* = rom_addr(2,$8059)
	jsr sfx_0_set
	* = rom_addr(2,$8068)
	jsr sfx_0
	
	* = rom_addr(2,$80d0)
	jsr sfx_0
	
	* = rom_addr(2,$80fd)
	jsr sfx_0
	
	* = rom_addr(2,$8114)
	jsr sfx_0
	
	* = rom_addr(2,$8186)
	jsr sfx_4_freq



	.section bank_2_0

sfx_0_set
	sty $4001
	stx $4000
	rts
	
	
sfx_4_freq
	ldx #4
	bne sfx_sq_freq
	
sfx_0
	stx $4000
	sty $4001
	ldx #0
sfx_sq_freq

	lsr
	tay
	lda m_freq_tbl_lo,y
	sta $4002,x
	;sta $05f9,x
	lda m_freq_tbl_hi,y
	sta $4003,x
	rts
	
	
	
	.send
	
	
	