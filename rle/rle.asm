

	.section bank_f_3

rle2ppu
	sta 0
	stx 1
	jmp _start
	
	
_loop
	tya
	sec
	adc 0
	sta 0
	bcc _start
	inc 1
_start
	ldy #0
	lax (0),y
	beq _exit
	bmi _run
-	iny
	lda (0),y
	sta $2007
	dex
	bne -
	geq _loop
	
_run
	dex
	dex
	iny
	lda (0),y
-	sta $2007
	inx
	bne -
	geq _loop
	
	
_exit
	rts


	.send
