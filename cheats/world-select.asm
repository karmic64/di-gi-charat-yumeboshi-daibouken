
	.section bank_0_0
	
world_select
	.enc "gametext"
.virtual 0
_cur_lvl .byte ?
_max_lvl .byte ?
_cursor .byte ?
.endv


	lda #0
	sta _cur_lvl
	sta _cursor
	
	
	jsr $ec8a
	jsr $fe57
	
	lda #<_init_stripe
	sta $f0
	lda #>_init_stripe
	sta $f1
	jsr $ece3
	
	ldx #size(_main_stripe)-1
-	lda _main_stripe,x
	sta $0302,x
	dex
	bpl -
	
	;;;;
	
_mainloop
	
	
	lda $f5
	and #$0f
	beq +
	lda #4
	sta $0602
+	
	
	
	lda $f5
	and #3
	beq +
	inc _cursor
+	
	
	
	lda _cursor
	lsr
	lda $f5
	bcs _level_control
	
_world_control
	ldx $0635
	
	bit _bit_up
	beq +
	inx
	cpx #7
	bcc +
	ldx #0
+	
	bit _bit_down
	beq +
	dex
	bpl +
	ldx #6
+	
	stx $0635
	
	
	
	lda $e013,x
	clc
	sbc $e012,x
	sta _max_lvl
	
	cmp _cur_lvl
	bcs _after_control
	sta _cur_lvl
	
	bcc _after_control
	
	
_level_control
	
	ldy _cur_lvl
	
	bit _bit_up
	beq +
	iny
	cpy _max_lvl
	beq +
	bcc +
	ldy #0
+	
	
	
+	bit _bit_down
	beq +
	dey
	bpl +
	ldy _max_lvl
+	
	
_set_level
	sty _cur_lvl
	
	
	
_after_control
	
	;;;;
	
	clc
	
	lda $0635
	adc #'1'
	sta $0304
	
	lda _cur_lvl
	adc #'1'
	sta $0308
	
	lda _cursor
	lsr
	lda #$10
	bcc +
	lda #0
+	sta $030c
	lda #$40
	bcs +
	lda #0
+	sta $030d
	
	lda #$21
	sta $0301
	
	
	jsr $ea2b
	jsr $eaa7
	
	lda $f5
	and #$10
	bne _exit
	jmp _mainloop
	
	
	
	;;;;
	
_exit
	jsr $eaa3
	
	inc $06f3
	lda #2
	sta $05c5
	lda #3
	sta $04ed
	
	ldx $0635
	lda $e012,x
	clc
	adc _cur_lvl
	tay
	
	jmp $e41f
	
	
	
	
_bit_up .byte 8
_bit_down .byte 4
	
	
	
_init_stripe
	.text $3f,$00, $02,$0f,$30
	.text $3f,$05, $01,$2a
	.text $20,$ea, len("WORLD SELECT"),"WORLD SELECT"
	.text $21,$4f, $02,$f4,$f4
	.byte 0

_main_stripe .block
	;.byte $21
	.byte     $4c, $01,0
	.byte $21,$53, $01,0
	.byte $23,$d3, $02,0,0
	.byte 0
	.bend




	.send
