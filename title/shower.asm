.text "NES",$1a
.byte 1,0
* = $10

tile_data = binary("tiles-out")
name_data = binary("name-out")

BG_COL = 12

	.logical $c000
	
reset
	lda #$00
	sta $2000
	sta $2001
	
	lda $2002
-	lda $2002
	bpl -
-	lda $2002
	bpl -
	
	
	lda #$3f
	sta $2006
	ldy #$00
	sty $2006
	
	lda #BG_COL
	sta $2007
	ldx #2
-	lda pal0,x
	sta $2007
	dex
	bpl -
	lda $2007
	ldx #2
-	lda pal1,x
	sta $2007
	dex
	bpl -
	lda $2007
	ldx #2
-	lda pal2,x
	sta $2007
	dex
	bpl -
	
	
	lda #$20
	sta $2006
	sty $2006
	
	lda #<names
	sta 0
	lda #>names
	sta 1
	ldx #4
-	lda (0),y
	sta $2007
	iny
	bne -
	inc 1
	dex
	bne -
	
	
	stx $2006
	stx $2006
	
	lda #<tiles
	sta 0
	lda #>tiles
	sta 1
	ldx #$10
-	lda (0),y
	sta $2007
	iny
	bne -
	inc 1
	dex
	bne -
	
	stx $2005
	stx $2005
	
	lda #$0a
	sta $2001
	
	
	jmp *
	
	
	
	
pal0
	.byte [55,13,28][::-1]
pal1
	.byte [37,13,42][::-1]
pal2
	.byte [32,27,255][::-1]
	
	
tiles
	.text tile_data
names
	.text name_data
	
	
	
	* = $fffc
	.word reset,0
	.here
	