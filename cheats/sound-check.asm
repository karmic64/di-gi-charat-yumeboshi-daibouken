

	.section bank_0_0
	
sound_check
	.enc "gametext"
	.virtual 0
_cursor .byte ?
_sounds .fill 6
_attribuf .fill 4
	.endv
	
	
	lda #$80
	sta $0603
	
	
	lda #0
	sta _cursor
	ldx #size(_sounds)-1
-	sta _sounds,x
	dex
	bpl -
	
	
	
	jsr $ec8a
	
	jsr $fe57
	jsr $ff73
	
	
	lda #<_init_stripe
	sta $f0
	lda #>_init_stripe
	sta $f1
	jsr $ece3
	
	
	
	lda #<_nmi
	sta nmi_ptr
	lda #>_nmi
	sta nmi_ptr+1
	
	gne _first
	
	
	
_nmi
	
	ldx #$ff
	txs
	
	lda #$10
	sta $ff
	sta $2000
	
	
	lda #$21
	sta $2006
	lda #$4a
	sta $2006
	clc
-	inx
	lda _sounds,x
	adc #'0'
	sta $2007
	lda #' '
	sta $2007
	cpx #size(_sounds)-1
	bcc -
	
	
	lda #$23
	sta $2006
	lda #$d2
	sta $2006
	.for i = 0, i < size(_attribuf), i=i+1
		lda _attribuf+i
		sta $2007
	.next
	
	
	
	lda #$23
	sta $2006
	lda #$ca
	sta $2006
	lda $0100
	lsr
	lda #$00
	bcc +
	lda #$55
+	.rept 4
		sta $2007
	.next
	
	
	
	
	
	
	ldx #0
	stx $2005
	stx $2005
	ldy #$03
-	dex
	bne -
	dey
	bne -
	
	
	lda #$0b
	sta $2001
	jsr $ec7c
	lda #$0a
	sta $2001
	
	
	
_first
	
	jsr $f661
	
	lda $f5
	ldx _cursor
	
	lsr
	bcc _nor
	inx
	cpx #size(_sounds)
	bcc _nor
	ldx #0
_nor
	
	lsr
	bcc _nol
	dex
	bpl _nol
	ldx #size(_sounds)-1
_nol
	stx _cursor
	
	
	ldy _sounds,x
	lsr
	bcc _nod
	dey
	bpl _nod
	ldy #7
_nod
	
	lsr
	bcc _nou
	iny
	cpy #8
	bcc _nou
	ldy #0
_nou
	sty _sounds,x
	
	
	bit _sound_tbl+1
	bne _issel
	bit _sound_tbl+2
	bne _isb
	bit _sound_tbl+3
	beq _noa
	lda _sound_tbl,y
	sta $0600,x
	gne _noa
_issel
	lda $0100
	eor #1
	sta $0100
	gne _noa
_isb
	ldx #($620-$5ec)-1
	lda #0
-	sta $05ec,x
	dex
	bpl -
	lda #$80
	sta $0603
_noa
	
	
	
	
	
	
	lda #0
	.for i = 0, i < size(_attribuf), i=i+1
		sta _attribuf+i
	.next
	ldx _cursor
	inx
	txa
	lsr
	tax
	lda #$30
	bcc +
	lda #$c0
+	
	sta _attribuf,x
	
	
	
	
	
	lda $2002
	lda #$90
	sta $ff
	sta $2000
	
	jmp *
	
	
	
_sound_tbl
	.byte 1,2,4,8,$10,$20,$40,$80
	
	
	
_init_stripe
	.text $3f,$00, $02,$0f,$30
	.text $3f,$05, $01,$10
	.text $3f,$0d, $01,$2a
	.text $20,$ea, len("SOUND  CHECK"),"SOUND  CHECK"
	.byte 0
	
	
	
	
	.send
	
	