	
	* = rom_addr(6,$8067)
	.logical $8067
end_name_rle
	.binary "name-out.rle"
	.cerror * > $821f, "too far"
	.here
	
	
	* = rom_addr(6,$8363)
	jsr upload_end_name
	
	
	
	.section bank_6_0
upload_end_name
	asl $0100
	
	lda #$20
	sta $2006
	lda #$00
	sta $2006
	
	lda #<end_name_rle
	ldx #>end_name_rle
	jsr rle2ppu
	
	
	
	lsr $0100
	rts
	
	
	
	
	.send
	
	
	
	
	* = rom_addr(6,$82b0)
	.byte 0  ;disable mario's eye attributes
	
	
	
	
	
	;;;;;;;;; chr replace
	
	* = $34010
	.binary "tiles-out"