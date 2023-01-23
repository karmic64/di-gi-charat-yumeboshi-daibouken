	
	
	* = rom_addr(FIXED_BANK,$e409)
	jmp title_cheat_hook
	
	
	.section bank_0_0
title_cheat_hook

	jsr $9a3d
	
	jsr $eaa3
	
	
	lda $f7
	and #$0f
	cmp #$09
	beq j_world_select
	cmp #$0a
	beq j_skip_to_ending
	cmp #$06
	beq j_sound_check
	jmp $e40c
	
	
j_world_select
	jmp world_select
	
j_sound_check
	jmp sound_check
	
j_skip_to_ending
	lda #$06
	sta $0635
	lda #$13
	sta $0531
	lda #$05
	sta $0532
	jsr $fe16
	
	jmp skip_to_ending
	
	
	.send
	
	
	
	
	.section bank_f_4
	
skip_to_ending
	
	lda #$05
	jsr $ff85
	jsr $8451
	
	lda #$04
	jsr $ff85
	jsr $f754
	
	lda #$03
	jsr $ff85
	jsr $93a4
	
	
	jmp $e956
	
	.send
	
	
	
	
	.include "world-select.asm"
	.include "sound-check.asm"
	