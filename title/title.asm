	* = rom_addr(0,$961f)
	.logical $961f
title_map_rle
	.binary "name-out.rle"
	.cerror * > $9882, "too long"
	.here
	
	* = rom_addr(0,$9882)
	.byte 12,56,15,28
	.byte 12,37,15,42
	.byte 12,32,27,?
	.byte 12,32,?,?
	.byte 12,$38,?,?
	.byte 12,?,?,?
	.byte 12,?,?,?
	.byte 12,?,?,?
	
	
	
	* = rom_addr(0,$9a87)
	jmp title_init_hook
	* = rom_addr(0,$9b09)
	jmp title_clear_hook
	
	
	
	
	
	.section bank_0_0
	
title_init_hook
	lda #$20
	sta $2006
	ldy #$00
	sty $2006
	
	
	lda #<title_map_rle
	ldx #>title_map_rle
	jsr rle2ppu
	
	
	lda #$3f
	sta $0200
	lda #$ca
	sta $0201
	lda #0
	sta $0202
	lda #$73
	sta $0203
	
	
	
	lda #0
	sta $11
	lda #$98
	sta $ff
	sta $2000
	jmp $9a95
	
	
	
	
	
title_clear_hook
	sta $0305
	
	lda $14
	cmp #$21
	bne +
	lda $15
	cmp #$06
	bne +
	
	lda #$f8
	sta $0200
+	
	
	jmp $9b0c
	
	
	
	.send
	
	
	
	
	;;;; chr replace
	
	
	* = $2a010
	.binary "tiles-out"
	; paw tile
	.byte %00011000
	.byte %11011000
	.byte %11000110
	.byte %00110110
	.byte %01111000
	.byte %01111000
	.byte %00110000
	.byte %00000000
	.fill 8,0
	