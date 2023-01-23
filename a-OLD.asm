dummy
            
ROMNAME = "Super Mario Bros. 2 (U) (PRG0) [!].nes"
           	
            .binary ROMNAME


OLD_BLOCKS = $2a
            
            * = $8010
            .logical $8000
            
            
            ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
            
            * = $875f
            
            
            
block_hook
            cpy #OLD_BLOCKS+1
            bcc +
            
            lda block_tempo - OLD_BLOCKS-1,y
            sta $0612
            lda block_lo - OLD_BLOCKS-1,y
            sta $bb
            lda block_hi - OLD_BLOCKS-1,y
            sta $bc
            ;sq2 index is always $00
            lda block_tri - OLD_BLOCKS-1,y
            sta $0615
            lda block_sq1 - OLD_BLOCKS-1,y
            sta $0614
            lda block_perc - OLD_BLOCKS-1,y
            jmp $83fa
            
+           lda $8fff,y
            jmp $83da
            
            
hurt_samp_hook
	sta $0608
	cmp #4
	bne _end
	
	ldy $8f
	lda #13
	sta $4010
	lda _base,y
	sta $4012
	lda _len,y
	jmp $832f
	
	
_end
	jmp  $831a
	
_tbl = [samp_deji,samp_gema,samp_puti,samp_usada]
_base
	.for i = 0, i < 4, i=i+1
		.byte samp_base(_tbl[i])
	.next
_len
	.for i = 0, i < 4, i=i+1
		.byte samp_len(_tbl[i])
	.next



			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;the first note is $02 (C#1), and the step is $02
make_octave .segment
C\1     = \2+0
Cs\1    = \2+2
D\1     = \2+4
Ds\1    = \2+6
E\1     = \2+8
F\1     = \2+10
Fs\1    = \2+12
G\1     = \2+14
Gs\1    = \2+16
A\1     = \2+18
As\1    = \2+20
B\1     = \2+22
.endm
#make_octave 1, 0
#make_octave 2, 24
#make_octave 3, 48
#make_octave 4, 72
#make_octave 5, 96

;perc can be either noise or dpcm, depending if the song id mask ($0600) is either $04 (underground) or $10 (starman)

            ;tempo index, ptr
blocktbl = [$b3, star0, star0_sq1, star0_tri, star0_perc,     $b3, star1, star1_sq1, star1_tri, star1_perc,     ]
block_tempo .byte blocktbl[::5]
block_lo    .byte <blocktbl[1::5]
block_hi    .byte >blocktbl[1::5]
block_sq1   .byte blocktbl[2::5]
block_tri   .byte blocktbl[3::5]
block_perc  .byte blocktbl[4::5]

;music data format:
;$00 - sq2 end block, sq1 sweep-down effect enable, tri end, perc loop block
;$80-$ff - upper nybble, instrument - lower nybble, duration $8f00+($0612)+x
;$01-$7f - note (on sqx, $7e - cut), noi type << 1, dmc sample mask >> 1

star0   .logical 0
        .byte $85,Fs4
        .byte $81,$7e
        .byte $85,Ds4
        .byte $81,$7e
        .byte $82,Fs4
        .byte $80,$7e
        .byte $85,Gs4
        .byte $81,$7e
        .byte $87,As4
        .byte $82,B4
        .byte $80,$7e
        .byte $82,As4
        .byte $80,$7e
        .byte $82,B4
        .byte $80,$7e
        .byte $85,As4
        .byte $81,$7e
        .byte $87,Fs4
        
        .byte $83,$7e
        .byte $c2,Ds4
        .byte $c0,$7e
        .byte $c2,F4
        .byte $c0,$7e
        .byte $c2,Fs4
        .byte $c0,$7e
        .byte $c2,Gs4
        .byte $c0,$7e
        .byte $c6,F4
        .byte $c0,$7e
        .byte $c7,Ds4
        .byte $c2,Cs4
        .byte $c0,$7e
        .byte $c2,Cs4
        .byte $c0,$7e
        .byte $c2,Ds4
        .byte $c0,$7e
        .byte $cb,Ds4
        .byte $c3,$7e
        
        .byte $00
        
        
star0_sq1
        .byte $85,Ds4
        .byte $81,$7e
        .byte $85,B3
        .byte $81,$7e
        .byte $82,Ds4
        .byte $80,$7e
        .byte $85,F4
        .byte $81,$7e
        .byte $87,Fs4
        .byte $82,Fs4
        .byte $80,$7e
        .byte $82,Fs4
        .byte $80,$7e
        .byte $82,Fs4
        .byte $80,$7e
        .byte $85,Fs4
        .byte $81,$7e
        .byte $87,Ds4
        
        .byte $83,$7e
        .byte $c2,Ds3
        .byte $c0,$7e
        .byte $c2,F3
        .byte $c0,$7e
        .byte $c2,Fs3
        .byte $c0,$7e
        .byte $c2,Gs3
        .byte $c0,$7e
        .byte $c6,F3
        .byte $c0,$7e
        .byte $c7,Ds3
        .byte $c2,Cs3
        .byte $c0,$7e
        .byte $c2,Cs3
        .byte $c0,$7e
        .byte $c2,Ds3
        .byte $c0,$7e
        .byte $cb,Ds3
        .byte $c3,$7e
        
        .byte $00
        
star0_tri
        .byte $83
        .byte B2,B3,B2,B3
        .byte Cs3,Cs4,Cs3,Cs4
        .byte As2,As3,As2,As3
        .byte Ds3,Ds4,Ds3,Ds4
        
        .byte B2,B3,B2,B3
        .byte Cs3,Cs4,Cs3,Cs4
        .byte Ds4,Gs4,Ds4,Gs4
        .byte Ds4,G4,Ds4,G4
        
star0_perc
        .byte $81,$01
        .byte $83,$10
        .byte $81,$10
        .byte $10
        .byte $10
        .byte $01
        .byte $01
        .byte 0
        
        .here
        
        
        

star1   .logical 0
        .byte $85,Fs4
        .byte $81,$7e
        .byte $85,Ds4
        .byte $81,$7e
        .byte $82,Fs4
        .byte $80,$7e
        .byte $85,Gs4
        .byte $81,$7e
        .byte $87,As4
        .byte $82,B4
        .byte $80,$7e
        .byte $82,As4
        .byte $80,$7e
        .byte $82,B4
        .byte $80,$7e
        .byte $85,Cs5
        .byte $81,$7e
        .byte $87,Fs4
        
        .byte $83,$7e
        .byte $c2,Ds4
        .byte $c0,$7e
        .byte $c2,F4
        .byte $c0,$7e
        .byte $c2,Fs4
        .byte $c0,$7e
        .byte $c2,B4
        .byte $c0,$7e
        .byte $c6,As4
        .byte $c0,$7e
        .byte $c7,Gs4
        .byte $c2,Fs4
        .byte $c0,$7e
        .byte $cd,Fs4
        .byte $c9,$7e
        
        .byte $00
        
        
star1_sq1
        .byte $85,Ds4
        .byte $81,$7e
        .byte $85,B3
        .byte $81,$7e
        .byte $82,Ds4
        .byte $80,$7e
        .byte $85,F4
        .byte $81,$7e
        .byte $87,Fs4
        .byte $82,Fs4
        .byte $80,$7e
        .byte $82,Fs4
        .byte $80,$7e
        .byte $82,Fs4
        .byte $80,$7e
        .byte $85,Fs4
        .byte $81,$7e
        .byte $87,Ds4
        
        .byte $83,$7e
        .byte $c2,Ds3
        .byte $c0,$7e
        .byte $c2,F3
        .byte $c0,$7e
        .byte $c2,Fs3
        .byte $c0,$7e
        .byte $c2,B3
        .byte $c0,$7e
        .byte $c6,As3
        .byte $c0,$7e
        .byte $c7,Gs3
        .byte $c2,Fs3
        .byte $c0,$7e
        .byte $cd,Fs3
        .byte $c9,$7e
        
        .byte $00
        
star1_tri
        .byte $83
        .byte B2,B3,B2,B3
        .byte Cs3,Cs4,Cs3,Cs4
        .byte As2,As3,As2,As3
        .byte Ds3,Ds4,Ds3,Ds4
        
        .byte B2,B3,B2,B3
        .byte Cs3,Cs4,Cs3,Cs4
        .byte Fs4,Fs4,Fs4,Fs4
        .byte Fs4,Fs4,Fs4,Fs4
        
star1_perc
        .byte $81,$01
        .byte $83,$10
        .byte $81,$10
        .byte $10
        .byte $10
        .byte $01
        .byte $01
        .byte 0
        
        .here
            
            
            
            
            
            ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
            
            * = $9103+4 ;start block
            .byte OLD_BLOCKS
            * = $910c+4 ;last block
            .byte OLD_BLOCKS+1
            * = $9115+4 ;loop to
            .byte OLD_BLOCKS
            
            
            * = $83d7
            jmp block_hook
            
            
            * = $8317
            jmp hurt_samp_hook
            
            
            
            * = $8fb3
            .for i = 0, i < $10, i=i+1
                .byte 3*(i+1)
            .next
            
            
            
            
            * = $8361
            lda #$a5
            cmp $0160
            beq +
            sta $0160
            ldx #0
            jsr mus_init
            
            
+ jmp mus_play
            
            
            


old_samps = [samp_old_01,samp_old_02,dummy,samp_old_08,samp_old_10,samp_old_20,samp_old_40,samp_old_80]
	* = $8342
	.for i = 0, i < 8, i=i+1
		.byte samp_base(old_samps[i])
	.next
	.for i = 0, i < 8, i=i+1
		.byte samp_len(old_samps[i])
	.next




	* = $a400
	.include "mus-player/a.asm"
            .here
            




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



inc_old_samp .macro
	.binary ROMNAME,$1c010+(\1*$40),(\2*$10)
	.endm
	
samp_base .function lab
	.endf (lab & $3fff) / $40
	
samp_len .function lab
	.endf size(lab) / $10


	* = $1c010
	.logical $c000
samp_old_01 #inc_old_samp $4f,$43
	.align $40,$55
samp_old_02 #inc_old_samp $60,$14
	.align $40,$55
samp_old_08 #inc_old_samp $00,$38
	.align $40,$55
samp_old_10 #inc_old_samp $31,$48
	.align $40,$55
samp_old_20 #inc_old_samp $60,$28
	.align $40,$55
samp_old_40 #inc_old_samp $0e,$3c
	.align $40,$55
samp_old_80 #inc_old_samp $1d,$50
	.align $40,$55
	
samp_puti .binary "dmc/nyu_loud_13.dmc"
	.align $40,$55
samp_deji .binary "dmc/nyo_loud_13.dmc",0,$180
	.cerror * > $dc00, "too far"
	
	
	* = $ed80
samp_usada .binary "dmc/usadahurt_loud_13.dmc"
	.align $10,$55
	.cerror * > $f000, "too far"
	
	
	* = $fb40
samp_gema .binary "dmc/gema_loud_13.dmc"
	.align $10,$55
	.cerror * > $fe00, "too far"
	
	
	
	
	* = $ffe0
	.text "SEKAI DE ICHIBAN"
	.here
	




