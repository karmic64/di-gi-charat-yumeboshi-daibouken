

inc_old_samp .macro
	.binary "../" .. ROMNAME,$1c010+(\1*$40),(\2*$10)
	.endm
	
samp_base .function lab
	.endf (lab & $3fff) / $40
	
samp_len .function lab
	.endf size(lab) / $10



	.section bank_f_0
samp_old_01 #inc_old_samp $4f,$43

	.align $40,$55
samp_old_02 .fill $14*$10 ;low/high bongos
	* = samp_old_02
samp_old_20 #inc_old_samp $60,$28

	.align $40,$55
samp_old_08 #inc_old_samp $00,$38

	.align $40,$55
samp_old_10 #inc_old_samp $31,$48

	.align $40,$55
samp_old_40 #inc_old_samp $0e,$3c

	.align $40,$55
;samp_old_80 #inc_old_samp $1d,$50

;	.align $40,$55
samp_gema .binary "gema_loudester.dmc"

	.align $40,$55
samp_usada .binary "usadahurt_loudester.dmc"

	.align $40,$55
	.send
	
	
	.section bank_f_1
	.align $40
samp_puti .binary "nyu_loudester_13.dmc"
	.align $10,$55
	.send
	
	
	.section bank_f_2
	.align $40
samp_deji .binary "nyo_loudester_14.dmc"
	.align $10,$55
	.byte $55
	.send








;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


	.section bank_2_0


hurt_samp_hook
	sta $0608
	cmp #4
	bne _end
	
	ldy $8f
	lda _freq,y
	sta $4010
	lda _base,y
	sta $4012
	lda _len,y
	gcs $832f
	
_end
	jmp $831a
	
	
	
	
_freq
	.byte 14,15,13,15
_tbl = [samp_deji,samp_gema,samp_puti,samp_usada]
_base
	.for i = 0, i < 4, i=i+1
		.byte samp_base(_tbl[i])
	.next
_len
	.for i = 0, i < 4, i=i+1
		.byte samp_len(_tbl[i])
	.next
	
	
	
	.send
	
	
	
	* = rom_addr(2,$8317)
	jmp hurt_samp_hook
	
	
	
old_samps = [samp_old_01,samp_old_02,dummy,samp_old_08,samp_old_10,samp_old_20,samp_old_40,dummy]
	* = rom_addr(2,$8342)
	.for i = 0, i < 8, i=i+1
		.byte samp_base(old_samps[i])
	.next
	.for i = 0, i < 8, i=i+1
		.byte samp_len(old_samps[i])
	.next
	
	
	