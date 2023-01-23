.cpu "6502i"



dummy
	
ROMNAME = "Super Mario Bros. 2 (U) (PRG0) [!].nes"
;ROMNAME = "scr-builds/2022-07-31_ORIGINAL_SOUND_Di_Gi_Charat_-_Yumeboshi_Daibouken-endfix.nes"
           	
	.binary ROMNAME

.enc "gametext"
.cdef "09",$d0
.cdef "AZ",$da
.cdef "--",$f4
.cdef "??",$f5
.cdef "..",$f6
.cdef ",,",$f7
.cdef "cc",$f8
.cdef "  ",$ff


HEXTBL = "0123456789ABCDEF"
MAKE16STR .function v
	.endf HEXTBL[(v>>12) & $f] .. HEXTBL[(v>>8) & $f] .. HEXTBL[(v>>4) & $f] .. HEXTBL[(v>>0) & $f]
MAKE8STR .function v
	.endf HEXTBL[(v>>4) & $f] .. HEXTBL[(v>>0) & $f]


;;;;;;;;;;;;;;;;;;;;; ROM LAYOUT ;;;;;;;;;;;;;;;;;;;;;;;;;;

FIXED_BANK = 7

rom_addr .function bank, addr
	.endf (bank*$4000)+$10 + (addr & $3fff)



* = rom_addr(0,0)
.logical $8000
* = $9c58
.dsection bank_0_0
.cerror * > $a200, "too far"
* = $ae5a
.dsection bank_0_1
.cerror * > $b900, "too far"
* = $bb52
.dsection bank_0_2
.cerror * > $c000, "too far"
.here


* = rom_addr(1,0)
.logical $8000
* = $9ebd
.dsection bank_1_0
.cerror * > $a030, "too far"
* = $b39b
.dsection bank_1_1
.cerror * > $b4e0, "too far"
* = $beb0
.dsection bank_1_2
.cerror * > $c000, "too far"
.here


* = rom_addr(2,0)
.logical $8000
* = $835a
.dsection bank_2_0
.cerror * > $c000, "too far"
.here


* = rom_addr(6,0)
.logical $8000
* = $943e
.dsection bank_6_0
.cerror * > $c000, "too far"
.here


* = rom_addr(FIXED_BANK,0)
.logical $c000
* = $c000 ;this space is originally filled with dmc samples but we reorganize them
.dsection bank_f_0
.cerror * > $dc00, "too far"
* = $ed4d
.dsection bank_f_1
.cerror * > $f000, "too far"
* = $fb36
.dsection bank_f_2
.cerror * > $fe00, "too far"
* = $fe97
.dsection bank_f_3
.cerror * > $ff50, "too far"
* = $ffa4
.dsection bank_f_4
.cerror * > $ffe0, "too far"

* = $ffe0
.enc "none"
.text "SEKAI DE ICHIBAN"
* = $fffa
.word nmi_op
.here






;;;;;;;;;;;;;;;;;;;;;;;;; INCLUDES ;;;;;;;;;;;;;;;;;;;;;;

.include "rle/rle.asm"

.include "dmc/dmc.asm"
.include "music/music.asm"

.include "title/title.asm"
;.include "cheats/cheats.asm"

.include "end/end.asm"

.include "triclyde-head.asm"





;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

 * = rom_addr(0,$ac5a)  ;change ending wart to mario palette
 lda #$00
















;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	
	
	.virtual $150
nmi_op .byte ?
nmi_ptr .word ?
	.endv
	
	
	
	

	.section bank_f_4
reset_hook
	
	ldx #0
	txa
-	sta $0100,x
	inx
	bne -
	
	lda #$4c
	sta nmi_op
	lda #<$eb8e
	sta nmi_ptr
	lda #>$eb8e
	sta nmi_ptr+1
	
	jmp $e3f9
	
	.send
	
	
	* = rom_addr(FIXED_BANK,$ff70)
	jmp reset_hook






