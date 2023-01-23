
	* = rom_addr(2,$8008)
	lsr
	jmp music_pause_hook
	

	.section bank_2_0
	
music_pause_hook
	bcs _is_paused
	
_not_paused
	lax $4015
	ora #$f
	sta $4015
	txa
	
	ldx #$c0
	lsr
	bcs +
	stx m_t_pitch_hi+0
+	
	lsr
	bcs +
	stx m_t_pitch_hi+4
+	
	
	jmp $8014
	
	
	
_is_paused
	jmp $800c
	
	
	
	.send








	* = rom_addr(2,$8020)
	jsr music_hook


m_zp = $bb


M_TRACKS = 5
M_MAX_TRACK_INDEX = (M_TRACKS-1)*4

	.virtual $1901
m_v_tempo
	.byte ?
m_v_tempo_accum
	.byte ?
m_v_tempo_flag
	.byte ?
	
	
m_track_vars
m_t_dur_cnt
	.byte ?
m_t_dur_save
	.byte ?
m_t_seq_id
	.byte ?
m_t_seq_index
	.byte ?
	.fill (M_TRACKS-1)*4
	
m_t_ptn_index
	.byte ?
m_t_instr_id
	.byte ?
m_t_env_index
	.byte ?
m_t_pitch_env_index
	.byte ?
	.fill (M_TRACKS-1)*4
	
m_t_volume
	.byte ?
m_t_note
	.byte ?
m_t_detune
	.byte ?
m_t_pitch_hi
	.byte ?
	.fill (M_TRACKS-1)*4
	
	
m_track_vars_end
	.endv
	
	
	
	
	.section bank_2_0
	
	
mus_stop
	ldx #m_track_vars_end-m_track_vars-1
	lda #0
-	sta m_track_vars,x
	dex
	bpl -
	
	rts
	
	
	
	
	;.align $10,$ff
	.enc "none"
	.text "music arranged by 125scratch...coded by karmic"
	
	
	
	
	
	
	
	
	
music_hook
	
	lda $0603
	bmi _stop
	beq _check_0600
_init_3
	sta $0606
	cmp #$40
	bne +
	lda #1
+	
	ldx $0609
	stx $05f3
	ldx #0
	stx $0609
	ldx #8
	gne _init_song
	
	
_stop
	jsr mus_stop
_set_stop
	sta $0606
	sta $0609
	jmp mus_play
	
	
_check_0600
	lda $0600
	beq _check_end
	cmp $0609
	beq mus_play
_init_0
	sta $0609
	ldx #0
	stx $0606
	geq _init_song
	
-
	inx
_init_song
	lsr
	bcc -
	
	lda m_song_tempo,x
	sta m_v_tempo
	lda m_song_tracks,x
	sta m_zp
	ldy m_song_seq,x
	
	jsr mus_stop
	stx m_v_tempo_accum
	
	ldx #M_MAX_TRACK_INDEX
	
- lsr m_zp
	bcc +
	
	tya
	iny
	sta m_t_seq_id,x
	
	lda #2
	sta m_t_ptn_index,x
	
	lda #$f0
	sta m_t_volume,x
	
	sta m_t_pitch_hi,x
	
	lda #$ff
	sta m_t_note,x
	
+	dex
	dex
	dex
	dex
	bpl -
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
	
_check_end
	lda m_t_ptn_index+0
	ora m_t_ptn_index+4
	ora m_t_ptn_index+8
	ora m_t_ptn_index+$c
	ora m_t_ptn_index+$10
	bne mus_play
	
	lda $0606
	and #$21
	beq _set_stop
	lda $05f3
	beq _set_stop
	ldx #0
	geq _init_0
	
	
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
mus_play
	lda m_v_tempo
	sec
	adc m_v_tempo_accum
	sta m_v_tempo_accum
	ror m_v_tempo_flag
	
	ldx #M_MAX_TRACK_INDEX
	gne _trackmain
	
_gotrkenv
	jmp _trkenv
	
_end
	rts
	
_nexttrack
	dex
	bmi _end
	dex
	dex
	dex
	
_trackmain
	lda m_t_ptn_index,x
	beq _gotrkenv
	lda m_v_tempo_flag
	bpl _gotrkenv
	dec m_t_dur_cnt,x
	bpl _gotrkenv
	
	
	ldy m_t_seq_id,x
	lda m_seq_tbl_lo,y
	sta m_zp+2
	lda m_seq_tbl_hi,y
	sta m_zp+3
	
	ldy m_t_seq_index,x
	lda (m_zp+2),y
	bmi ++
	cmp #$7f
	bne +
	lda #0
	sta m_t_ptn_index,x
	beq _gotrkenv
	
+	asl
	sta m_t_seq_index,x
	tay
	
	lda (m_zp+2),y
+	sta m_zp+1
	iny
	lda (m_zp+2),y
	sta m_zp
	
	
	ldy m_t_ptn_index,x
	lda (m_zp),y
	iny
	cmp #$fe
	bne +
	lda (m_zp),y
	iny
	sta m_v_tempo
	lda (m_zp),y
	iny
+	
	cmp #$fd
	bne +
	lda (m_zp),y
	iny
	sta m_t_detune,x
	lda (m_zp),y
	iny
+	
	cmp #$d0
	bcc +
	sbc #$d0
	sta m_t_instr_id,x
	lda (m_zp),y
	iny
+	
	cmp #$c0
	bcc +
	asl
	asl
	asl
	asl
	sta m_t_volume,x
	lda (m_zp),y
	iny
+	
	sta m_zp+2
	cmp #$80
	bcc _normaldur
	cmp #$a0
	bcs _recalldur
	
	lda (m_zp),y
	iny
	sta m_t_dur_save,x
	gne _setdur
	
_recalldur
	lda m_t_dur_save,x
	gpl _setdur
	
_normaldur
	sty m_zp+3
	
	and #$60
	asl
	asl
	rol
	rol
	ldy #0
	ora (m_zp),y
	tay
	lda m_duration_tbl,y
	
	ldy m_zp+3
_setdur
	sta m_t_dur_cnt,x
	
	
	lda m_zp+2
	and #$1f
	cmp #$1d
	bcc _normalnote
	sbc #$1e
	bmi _setnote ;$1d = cut
	bne _blanknote
	
_absnote
	lda (m_zp),y
	iny
	gne _setnote
	
_normalnote
	sty m_zp+3
	
	ldy #1
	;clc
	adc (m_zp),y
	
	ldy m_zp+3
_setnote
	sta m_t_note,x
	lda #0
	sta m_t_env_index,x
	sta m_t_pitch_env_index,x
	
	
_blanknote
	lda (m_zp),y
	cmp #$ff
	bne +
	ldy #2
	inc m_t_seq_index,x
	inc m_t_seq_index,x
+
	tya
	sta m_t_ptn_index,x
	
	
	
	
	
	
	
_trkenv
	
	
	cpx #$10
	bcs _noinstr
	lda m_t_ptn_index,x
	beq _noinstr
	lda m_t_note,x
	cmp #$ff
	beq _noinstr
	
	
	lda #$3f
	sta m_zp
	lda #0
	sta m_zp+1
	
	
	ldy m_t_instr_id,x
	lda m_instr_env_tbl,y
	bmi _noenv
	tay
	lda m_env_tbl_lo,y
	sta m_zp+2
	lda m_env_tbl_hi,y
	sta m_zp+3
	
	ldy m_t_env_index,x
	lda (m_zp+2),y
	sta m_zp
	iny
	lda (m_zp+2),y
	beq +
	inc m_t_env_index,x
+	
	
_noenv
	
	
	cpx #$0c
	bcs _nopitchenv
	
	ldy m_t_instr_id,x
	lda m_instr_pitch_env_tbl,y
	bmi _nopitchenv
	tay
	lda m_pitch_env_tbl_lo,y
	sta m_zp+2
	lda m_pitch_env_tbl_hi,y
	sta m_zp+3
	
	ldy m_t_pitch_env_index,x
	lda (m_zp+2),y
	sta m_zp+1
	iny
	lda (m_zp+2),y
	bpl _nextpitchenv
	cmp #$81
	bcc _afterpitchenv
	bne _nextpitchenv
	iny
	lda (m_zp+2),y
	sta m_t_pitch_env_index,x
	gpl _afterpitchenv
	
_nextpitchenv
	inc m_t_pitch_env_index,x
_afterpitchenv
	
_nopitchenv
	
	
_noinstr
	
	
	
	
	cpx #$0c
	beq _check_noi
	cpx #$04
	beq _check_sq2
	txa
	bne _do_chn
	
_check_sq1
	lda $060d
	beq _do_chn
	bne _sethi
	
_check_sq2
	lda $0607
	beq _do_chn
_sethi
	lda #$80
	sta m_t_pitch_hi,x
	bne _gonext
	
_check_noi
	lda $060e
	bne _gonext
	
	
_do_chn
	
	lda m_t_ptn_index,x
	beq _cut_chn
	lda m_t_note,x
	cmp #$ff
	beq _cut_chn
	cpx #8
	bcs +
	lda $0100
	lsr
	bcs _cut_main
+	
	
	cpx #$10
	beq _dmc_skip_4000
	lda m_zp
	cpx #$08
	beq +
	and #$0f
	ora m_t_volume,x
	tay
	lda m_volume_tbl,y
	sta m_zp+2
	lda m_zp
	and #$f0
	ora m_zp+2
+
	sta $4000,x
	lda #$08
	sta $4001,x
_dmc_skip_4000
	
	
	ldy m_t_note,x
	
	cpx #$10
	beq _main_dmc
	cpx #$0c
	beq _main_noi
	
	lda #0
	sta m_zp+2
	
	
	
	lda m_zp+1
	clc
	adc m_t_detune,x
	bpl +
	dec m_zp+2
+	clc
	adc m_freq_tbl_lo,y
	sta $4002,x
	
	lda m_zp+2
	adc m_freq_tbl_hi,y
	
	cmp m_t_pitch_hi,x
	beq +
	sta m_t_pitch_hi,x
	sta $4003,x
+	
	
	
_gonext
	jmp _nexttrack
	
	
_main_noi
	sty $400e
	sty $400f
	jmp _nexttrack
	
	
_main_dmc
	tya
	bmi _gonext
	ora #$80
	sta m_t_note,x
	lda m_dmc_tbl,y
	jsr $8317
	jmp _nexttrack
	
	
	
_cut_chn
	cpx #$10
	beq _gonext
	cpx #$08
	beq _cut_tri
	
_cut_main
	lda #$30
	sta $4000,x
	jmp _nexttrack
	
_cut_tri
	lda #$80
	sta $4008
	jmp _nexttrack
	
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
	
	
	
	
	
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
	
	
m_freqs = [$1ab8,$1938,$17cc,$1678,$1534,$1404,$12e4,$11d4,$10d4,$0fe0,$0efc,$0e24]
m_freq_tbl_lo
	.for i = 0, i < $40, i=i+1
		.byte <((m_freqs[i % 12] >> ((i / 12)+2)))
	.next
m_freq_tbl_hi
	.for i = 0, i < $40, i=i+1
		.byte (>((m_freqs[i % 12] >> ((i / 12)+2)))) | 8
	.next
	
	
m_volume_tbl
	.for i = 0, i < 16, i=i+1
		.for j = 0, j < 16, j=j+1
			.if i == 0 || j == 0
				.byte 0
			.elsif i * j < 15
				.byte 1
			.else
				.byte i * j / 15.0
			.endif
		.next
	.next
	
	
m_dmc_tbl
	.byte $01,$02,$04,$08,$10,$20,$40,$80
	
	
	
	
	
	.include "out.asm"
	
	.send
	
	
	
	
	
	
	
	
	
	
	
	.include "sfx-fixes.asm"
	
	