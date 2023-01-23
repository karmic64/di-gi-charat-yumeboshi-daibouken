#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <sys/stat.h>


#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) < (b) ? (b) : (a))




const char note_names[12][2] = {"C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"};


const char *space_delim = " \f\n\r\t\v";

unsigned lineno = 0;
char *currentfile = NULL;
char *read_line(FILE *f)
{
	static char linebuf[0x800];
	
	unsigned c;
	int ch;
	do
	{
		if (feof(f) || ferror(f)) return NULL;
		c = 0;
		while (1)
		{
			ch = fgetc(f);
			if (ch == EOF || ch == '\n') break;
			
			linebuf[c++] = ch;
		}
		lineno++;
		
		if (c && linebuf[c-1] == 0x0d) c--;
		
		/* no blank lines or comments */
	} while (!c || linebuf[0] == '#');
	
	linebuf[c] = '\0';
	return linebuf;
}

char *strtok_until(char *what)
{
	while (1)
	{
		char *tok = strtok(NULL,space_delim);
		if (!tok || !strcmp(tok,what)) return tok;
	}
}

void strtok_skip(unsigned c)
{
	for (unsigned i = 0; i < c; i++)
		strtok(NULL,space_delim);
}

void err(char *fmt, ...)
{
	printf("%s:%u: ",currentfile,lineno);
	
	va_list v;
	va_start(v, fmt);
	vprintf(fmt,v);
	va_end(v);
	
	putchar('\n');
}






/*****************************************************************************/

#define MAX_TRACKS 5
#define MAX_ORDERS 256

enum {
	CHN_SQ1 = 0,
	CHN_SQ2,
	CHN_TRI,
	CHN_NOI,
	CHN_DMC
};





typedef struct {
	int8_t note;
	int8_t instrument;
	int8_t volume;
	int16_t detune;
	int16_t tempo;
	uint8_t duration;
} note_event_t;

typedef struct {
	uint8_t length;
	note_event_t data[256];
} pattern_t;

typedef struct {
	uint8_t channel_usage;
	uint8_t speed;
	uint8_t tempo;
	
	uint8_t orders;
	uint16_t orderlist[MAX_TRACKS][MAX_ORDERS];
	
	uint8_t loop_point;
} song_t;


typedef struct {
	uint8_t length;
	uint8_t data[255];
} envelope_t;

typedef struct {
	uint8_t length;
	uint8_t loop;
	int8_t data[254];
} pitch_envelope_t;


typedef struct {
	int8_t env;
	int8_t pitch;
} instrument_t;




unsigned convert_tempo(unsigned speed, unsigned tempo)
{
	double o = 0x100 / ((double)speed);
	o *= (tempo / 150.0);
	
	return round(o) - 1;
}



/****************************************************************************/

#define MAX_FT_MACROS 256
#define MAX_FT_INSTRUMENTS 64
#define MAX_FT_SONGS 1

typedef struct {
	uint8_t length;
	uint8_t loop;
	uint8_t data[254];
} ft_macro_t;

typedef struct {
	ft_macro_t *vol_macro;
	ft_macro_t *duty_macro;
	ft_macro_t *pitch_macro;
} ft_instrument_t;


typedef struct {
	int8_t note;
	int8_t instrument;
	int8_t volume;
	int16_t detune;
	int16_t tempo;
} ft_row_t;

typedef struct {
	uint8_t length;
	ft_row_t data[256];
	
	char skip_effect;
	uint8_t skip_param;
	uint8_t skip_row;
} ft_pattern_t;



/****************************************************************************/

pattern_t module_pattern_tbl[512];
unsigned module_patterns = 0;

unsigned add_module_pattern(pattern_t *p)
{
	for (unsigned i = 0; i < module_patterns; i++)
	{
		pattern_t *cp = &module_pattern_tbl[i];
		if (p->length == cp->length && !memcmp(p->data, cp->data, p->length * sizeof(*p->data))) return i;
	}
	memcpy(&module_pattern_tbl[module_patterns], p, sizeof(*p));
	return module_patterns++;
}


song_t module_song_tbl[32];
unsigned module_songs = 0;

unsigned add_module_song(song_t *s)
{
	for (unsigned i = 0; i < module_songs; i++)
	{
		song_t *cs = &module_song_tbl[i];
		if (s->channel_usage != cs->channel_usage) continue;
		if (s->orders != cs->orders) continue;
		if (s->loop_point != cs->loop_point) continue;
		unsigned j = 0;
		for (  ; j < MAX_TRACKS; j++)
		{
			if (memcmp(s->orderlist[j], cs->orderlist[j], s->orders*sizeof(*cs->orderlist[j]))) break;
		}
		if (j == MAX_TRACKS) return i;
	}
	memcpy(&module_song_tbl[module_songs], s, sizeof(*s));
	return module_songs++;
}


envelope_t module_envelope_tbl[64];
unsigned module_envelopes = 0;

unsigned add_module_envelope(envelope_t *e)
{
	for (unsigned i = 0; i < module_envelopes; i++)
	{
		envelope_t *ce = &module_envelope_tbl[i];
		if (e->length == ce->length && !memcmp(e->data, ce->data, e->length * sizeof(*e->data))) return i;
	}
	
	memcpy(&module_envelope_tbl[module_envelopes], e, sizeof(*e));
	return module_envelopes++;
}


pitch_envelope_t module_pitch_envelope_tbl[64];
unsigned module_pitch_envelopes = 0;

unsigned add_module_pitch_envelope(pitch_envelope_t *e)
{
	for (unsigned i = 0; i < module_pitch_envelopes; i++)
	{
		pitch_envelope_t *ce = &module_pitch_envelope_tbl[i];
		if (e->length == ce->length && e->loop == ce->loop && !memcmp(e->data, ce->data, e->length * sizeof(*e->data))) return i;
	}
	
	memcpy(&module_pitch_envelope_tbl[module_pitch_envelopes], e, sizeof(*e));
	return module_pitch_envelopes++;
}


instrument_t module_instrument_tbl[64];
unsigned module_instruments = 0;

unsigned add_module_instrument(instrument_t *i)
{
	for (unsigned x = 0; x < module_instruments; x++)
	{
		instrument_t *ci = &module_instrument_tbl[x];
		if (i->env == ci->env && i->pitch == ci->pitch) return x;
	}
	
	memcpy(&module_instrument_tbl[module_instruments], i, sizeof(*i));
	return module_instruments++;
}



/*******************************************************************************/

unsigned add_module_envelope_from_instrument(ft_instrument_t *i, int chn)
{
	if (chn == CHN_DMC) return -1;
	
	envelope_t e;
	ft_macro_t *vm = i->vol_macro;
	ft_macro_t *dm = i->duty_macro;
	
	unsigned vl = vm ? vm->length : 0;
	unsigned dl = dm ? dm->length : 0;
	if (!vm && !dm)
	{
		vl = 1;
		dl = 1;
	}
	
	switch (chn)
	{
		case CHN_TRI:
		case CHN_NOI:
			e.length = vl;
			break;
		default:
			e.length = max(vl,dl);
			break;
	}
	
	unsigned vv = 15;
	unsigned dv = 0;
	for (unsigned i = 0; i < e.length; i++)
	{
		if (vm && i < vl) vv = vm->data[i];
		if (dm && i < dl) dv = dm->data[i];
		
		uint8_t ev;
		switch (chn)
		{
			case CHN_TRI:
				ev = vv ? 0x81 : 0x80;
				break;
			case CHN_NOI:
				ev = vv | 0x30;
				break;
			default:
				ev = vv | 0x30 | (dv << 6);
				break;
		}
		e.data[i] = ev;
	}
	
	return add_module_envelope(&e);
}



unsigned add_module_pitch_envelope_from_instrument(ft_instrument_t *i, int chn)
{
	if (chn == CHN_NOI || chn == CHN_DMC) return -1;
	
	pitch_envelope_t e;
	ft_macro_t *pm = i->pitch_macro;
	if (!pm) return -1;
	
	e.length = pm->length;
	e.loop = pm->loop;
	memcpy(e.data,pm->data,pm->length);
	
	return add_module_pitch_envelope(&e);
}



unsigned add_from_instrument(ft_instrument_t *si, int chn)
{
	instrument_t i;
	
	i.env = add_module_envelope_from_instrument(si,chn);
	i.pitch = add_module_pitch_envelope_from_instrument(si,chn);
	
	return add_module_instrument(&i);
}



/*******************************************************************************/

int read_ft_module(char *filename)
{
	printf("Opening %s...",filename);
	FILE *f = fopen(filename,"r");
	if (!f)
	{
		puts(strerror(errno));
		return 1;
	}
	puts("OK");
	
	
	/************ read ftm macros/instruments/patterns/songs ****************/
	
	song_t song_tbl[MAX_FT_SONGS];
	unsigned songs = 0;
	int current_song = -1;
	memset(song_tbl,0,sizeof(song_tbl));
	
	typedef ft_pattern_t ft_pattern_tbl_t[MAX_FT_SONGS][MAX_TRACKS][MAX_ORDERS];
	ft_pattern_tbl_t *ft_pattern_tbl = malloc(sizeof(ft_pattern_tbl_t));
	memset(ft_pattern_tbl,0,sizeof(ft_pattern_tbl_t));
	
	ft_macro_t ft_vol_macro_tbl[MAX_FT_MACROS];
	ft_macro_t ft_duty_macro_tbl[MAX_FT_MACROS];
	ft_macro_t ft_pitch_macro_tbl[MAX_FT_MACROS];
	ft_instrument_t ft_instrument_tbl[MAX_FT_INSTRUMENTS];
	memset(ft_vol_macro_tbl,0,sizeof(ft_vol_macro_tbl));
	memset(ft_duty_macro_tbl,0,sizeof(ft_duty_macro_tbl));
	memset(ft_pitch_macro_tbl,0,sizeof(ft_pitch_macro_tbl));
	memset(ft_instrument_tbl,0,sizeof(ft_instrument_tbl));
	
	
	lineno = 0;
	currentfile = strrchr(filename,'/');
	if (!currentfile) currentfile = filename;
	else currentfile++;
	char *line = NULL;
	while (1)
	{
next_line_read:
		line = read_line(f);
skip_next_line_read:
		
		if (line == NULL) break;
		char *cmd = strtok(line,space_delim);
		if (cmd == NULL) continue;
		
		if (!strcmp(cmd,"MACRO"))
		{
			/************************************************/
			char *type_str = strtok(NULL,space_delim);
			if (!type_str)
			{
				err("can't get macro type");
				continue;
			}
			char *id_str = strtok(NULL,space_delim);
			if (!id_str)
			{
				err("can't get macro id");
				continue;
			}
			char *loop_str = strtok(NULL,space_delim);
			if (!loop_str)
			{
				err("can't get loop point");
				continue;
			}
			
			int type = atoi(type_str);
			int id = atoi(id_str);
			int loop = atoi(loop_str);
			
			ft_macro_t *mac;
			switch (type)
			{
				case 0:
					mac = &ft_vol_macro_tbl[id];
					break;
				case 2:
					mac = &ft_pitch_macro_tbl[id];
					break;
				case 4:
					mac = &ft_duty_macro_tbl[id];
					break;
				default:
					continue;
			}
			
			if (mac->length)
			{
				err("macro %u already defined",id);
				continue;
			}
			
			
			/**********************************************/
			char *data_str_tbl[256];
			unsigned data_strs = 0;
			
			char *tok = strtok_until(":");
			while ((tok = strtok(NULL,space_delim)) != NULL)
			{
				if (data_strs == 256)
				{
					err("too much data");
					goto next_line_read;
				}
				data_str_tbl[data_strs++] = tok;
			}
			if (!data_strs)
			{
				err("no macro data");
				continue;
			}
			
			
			mac->length = data_strs;
			mac->loop = loop;
			for (unsigned i = 0; i < data_strs; i++)
				mac->data[i] = atoi(data_str_tbl[i]);
			
			
		}
		else if (!strcmp(cmd,"INST2A03"))
		{
			/****************************************************/
			char *id_str = strtok(NULL,space_delim);
			if (!id_str)
			{
				err("can't get instrument id");
				continue;
			}
			char *vol_str = strtok(NULL,space_delim);
			if (!vol_str)
			{
				err("can't get instrument volume");
				continue;
			}
			strtok_skip(1);
			char *pitch_str = strtok(NULL,space_delim);
			if (!pitch_str)
			{
				err("can't get instrument pitch");
				continue;
			}
			strtok_skip(1);
			char *duty_str = strtok(NULL,space_delim);
			if (!duty_str)
			{
				err("can't get instrument duty");
				continue;
			}
			
			/****************************************************/
			int id = atoi(id_str);
			int vol = atoi(vol_str);
			int duty = atoi(duty_str);
			int pitch = atoi(pitch_str);
			if (id < 0 || id >= MAX_FT_INSTRUMENTS)
			{
				err("bad instrument id");
				continue;
			}
			if (vol >= MAX_FT_MACROS)
			{
				err("bad instrument volume macro id");
				continue;
			}
			if (duty >= MAX_FT_MACROS)
			{
				err("bad instrument duty macro id");
				continue;
			}
			if (pitch >= MAX_FT_MACROS)
			{
				err("bad instrument pitch macro id");
				continue;
			}
			
			
			/****************************************************/
			ft_macro_t *vm = vol >= 0 ? &ft_vol_macro_tbl[vol] : NULL;
			ft_macro_t *dm = duty >= 0 ? &ft_duty_macro_tbl[duty] : NULL;
			ft_macro_t *pm = pitch >= 0 ? &ft_pitch_macro_tbl[pitch] : NULL;
			
			if (vm && !vm->length)
			{
				err("WARNING: volume macro %i does not exist",vol);
				vm = NULL;
				//continue;
			}
			if (dm && !dm->length)
			{
				err("WARNING: duty macro %i does not exist",duty);
				dm = NULL;
				//continue;
			}
			if (pm && !pm->length)
			{
				err("WARNING: pitch macro %i does not exist",pitch);
				pm = NULL;
				//continue;
			}
			
			/****************************************************/
			ft_instrument_t *i = &ft_instrument_tbl[id];
			i->vol_macro = vm;
			i->duty_macro = dm;
			i->pitch_macro = pm;
		}
		else if (!strcmp(cmd,"TRACK"))
		{
			if (songs == MAX_FT_SONGS)
			{
				err("too many songs");
				current_song = -1;
				continue;
			}
			strtok_skip(1);
			char *speed_str = strtok(NULL,space_delim);
			char *tempo_str = strtok(NULL,space_delim);
			if (!speed_str)
			{
				err("can't get speed");
				continue;
			}
			if (!tempo_str)
			{
				err("can't get tempo");
				continue;
			}
			current_song = songs++;
			song_tbl[current_song].speed = atoi(speed_str);
			song_tbl[current_song].tempo = atoi(tempo_str);
		}
		else if (!strcmp(cmd,"ORDER"))
		{
			/****************************************************/
			if (current_song < 0)
			{
				err("no song defined yet");
				continue;
			}
			
			char *index_str = strtok(NULL,space_delim);
			if (!index_str)
			{
				err("can't get index");
				continue;
			}
			int index = strtol(index_str,NULL,16);
			if (index != song_tbl[current_song].orders)
			{
				err("unexpected index");
				continue;
			}
			
			/****************************************************/
			char *tok = strtok_until(":");
			char *data_str_tbl[MAX_TRACKS];
			unsigned data_strs = 0;
			
			while ((tok = strtok(NULL,space_delim)) != NULL)
			{
				if (data_strs == 5) break;
				data_str_tbl[data_strs++] = tok;
			}
			if (data_strs < MAX_TRACKS)
			{
				err("not enough order data");
				continue;
			}
			
			
			for (int i = 0; i < MAX_TRACKS; i++)
				song_tbl[current_song].orderlist[i][index] = strtol(data_str_tbl[i],NULL,16);
			
			song_tbl[current_song].orders++;
		}
		else if (!strcmp(cmd,"PATTERN"))
		{
			if (current_song < 0)
			{
				err("no song defined yet");
				continue;
			}
			
			char *id_str = strtok(NULL,space_delim);
			if (!id_str)
			{
				err("can't get pattern id");
				continue;
			}
			unsigned id = strtol(id_str,NULL,16);
			
			/****************************************************************/
			unsigned timers[MAX_TRACKS];
			unsigned next_notes[MAX_TRACKS];
			unsigned prv_instrs[MAX_TRACKS];
			unsigned next_instrs[MAX_TRACKS];
			unsigned cur_instrs[MAX_TRACKS];
			memset(timers,-1,sizeof(timers));
			memset(next_notes,-1,sizeof(next_notes));
			memset(prv_instrs,-1,sizeof(prv_instrs));
			memset(next_instrs,-1,sizeof(next_instrs));
			memset(cur_instrs,-1,sizeof(cur_instrs));
			
			int endflag = 0;
			unsigned rownum = 0;
			while (!endflag)
			{
				/*********** try reading ROW line *******************************/
				line = read_line(f);
				
				if (line == NULL) endflag = 1;
				else if (memcmp(line,"ROW ",4)) endflag = 1;
				else strtok(line,space_delim);
				
				unsigned this_notes[MAX_TRACKS];
				unsigned this_instrs[MAX_TRACKS];
				memset(this_notes,-1,sizeof(this_notes));
				memset(this_instrs,-1,sizeof(this_instrs));
				if (!endflag)
				{
					char *index_str = strtok(NULL,space_delim);
					if (!index_str)
					{
						err("can't get row number");
						goto next_line_read;
					}
					if (strtol(index_str,NULL,16) != rownum)
					{
						err("unexpected row number");
						goto next_line_read;
					}
					strtok_until(":");
					
					for (unsigned i = 0; i < MAX_TRACKS; i++)
					{
						ft_pattern_t *p = &((*ft_pattern_tbl)[current_song][i][id]);
						
						char *tok;
						unsigned coli = 0;
						ft_row_t *r = &p->data[rownum];
						memset(r,-1,sizeof(*r));
						while ((tok = strtok(NULL,space_delim)) != NULL)
						{
							if (!strcmp(tok,":")) break;
							switch (coli)
							{
								case 0:
								{
									if (!strcmp(tok,"..."))
									{
										r->note = -1;
										break;
									}
									if (!strcmp(tok,"---"))
									{
										r->note = -2;
										break;
									}
									
									if (i == CHN_NOI)
									{
										tok[1] = '\0';
										r->note = strtol(tok,NULL,16) ^ 0x0f;
									}
									else
									{
										unsigned note = -1;
										unsigned octave = tok[2] - '1';
										for (unsigned i = 0; i < 12; i++)
										{
											if (!memcmp(note_names[i],tok,2))
											{
												note = i;
												break;
											}
										}
										if (note == -1U)
										{
											err("bad note name %s",tok);
											goto next_line_read;
										}
										r->note = octave * 12 + note;
									}
									break;
								}
								case 1:
								{
									if (strcmp(tok,"..")) r->instrument = strtol(tok,NULL,16);
									else r->instrument = -1;
									break;
								}
								case 2:
								{
									if (tok[0] == '.') r->volume = -1;
									else r->volume = strtol(tok,NULL,16);
									break;
								}
								default:
								{
									if (!p->skip_effect)
									{
										switch (tok[0])
										{
											case 'B':
											case 'C':
											case 'D':
												p->skip_effect = tok[0];
												p->skip_param = strtol(tok+1,NULL,16);
												p->skip_row = rownum;
												break;
										}
									}
									
									switch (tok[0])
									{
										case 'F':
										{
											r->tempo = strtol(tok+1,NULL,16);
											break;
										}
										case 'P':
										{
											int8_t d = (int8_t)(strtol(tok+1,NULL,16) ^ 0x80);
											r->detune = (uint8_t)(-d);
											break;
										}
										case 'B':
										case 'C':
										case 'D':
										case '.':
											break;
										default:
											err("WARNING: unrecognized effect %s", tok);
											break;
									}
								}
							}
							coli++;
						}
						p->length = rownum+1;
					}
					rownum++;
				}
				
				
				
			}
			
			
			
			
			
			goto skip_next_line_read;
		}
	}
	
	
	
	fclose(f);
	
	
	/************* add ftm data to module **********************/
	
	for (int current_song = 0; current_song < songs; current_song++)
	{
		song_t *song = &song_tbl[current_song];
		
		/**************** determine actual song length/loop point *********/
		for (unsigned ord = 0; ord < song->orders; ord++)
		{
			for (unsigned chn = 0; chn < MAX_TRACKS; chn++)
			{
				unsigned pati = song->orderlist[chn][ord];
				ft_pattern_t *p = &((*ft_pattern_tbl)[current_song][chn][pati]);
				
				switch (p->skip_effect)
				{
					case 'B':
						if (p->skip_param <= ord)
							song->loop_point = p->skip_param;
						if (p->skip_param >= ord) break;
						song->orders = ord+1;
						goto got_actual_song_length;
					case 'C':
						song->loop_point = -1;
						song->orders = ord+1;
						goto got_actual_song_length;
				}
			}
		}
got_actual_song_length:
		
		
		/*************** determine channel usage ***********************/
		for (unsigned chn = 0; chn < MAX_TRACKS; chn++)
		{
			for (unsigned ord = 0; ord < song->orders; ord++)
			{
				unsigned pati = song->orderlist[chn][ord];
				ft_pattern_t *p = &((*ft_pattern_tbl)[current_song][chn][pati]);
				
				for (unsigned row = 0; row < p->length; row++)
				{
					if (p->data[row].note >= 0)
					{
						song->channel_usage |= (0x10 >> chn);
						goto check_next_channel_usage;
					}
				}
			}
check_next_channel_usage:;
		}
		
		
		/**************** convert ft-patterns to module patterns *********/
		for (unsigned ord = 0; ord < song->orders; ord++)
		{
			/********** first determine actual order row length *********/
			unsigned rows = UINT_MAX;
			
			for (unsigned chn = 0; chn < MAX_TRACKS; chn++)
			{
				unsigned pati = song->orderlist[chn][ord];
				ft_pattern_t *p = &((*ft_pattern_tbl)[current_song][chn][pati]);
				
				unsigned this_rows = p->length;
				if (p->skip_effect)
					this_rows = p->skip_row+1;
				
				if (this_rows < rows) rows = this_rows;
			}
			
			
			/************** now do real conversion *****************/
			for (unsigned chn = 0; chn < MAX_TRACKS; chn++)
			{
				if (song->channel_usage & (0x10 >> chn))
				{
					unsigned pati = song->orderlist[chn][ord];
					ft_pattern_t *op = &((*ft_pattern_tbl)[current_song][chn][pati]);
					pattern_t p;
					memset(&p,0,sizeof(p));
					
					int prv_instr = -1;
					int cur_instr = -1;
					int prv_volume = -1;
					int prv_detune = -1;
					note_event_t next_event;
					memset(&next_event,-1,sizeof(next_event));
					
					for (unsigned rownum = 0; rownum <= rows; rownum++)
					{
						ft_row_t *r = rownum < rows ? &op->data[rownum] : NULL;
						
						if (r && r->instrument != -1)
							cur_instr = add_from_instrument(&ft_instrument_tbl[r->instrument], chn);
						
						int newflag = 0;
						if (rownum == rows)
							newflag = 1;
						else if (next_event.duration == 0x7f)
							newflag = 1;
						else if (r->note != -1 && (chn != CHN_DMC || (r->note != 24 || r->note != 25)))
							newflag = 1;
						else if (chn != CHN_DMC && r->volume >= 0 && r->volume != prv_volume)
							newflag = 1;
						else if (chn != CHN_NOI && chn != CHN_DMC && r->detune >= 0 && r->detune != prv_detune)
							newflag = 1;
						else if (r->tempo >= 0)
							newflag = 1;
						
						if (newflag)
						{
							if (rownum)
							{
								memcpy(&p.data[p.length++],&next_event,sizeof(next_event));
							}
							
							if (r)
							{
								next_event.note = r->note;
								next_event.instrument = prv_instr != cur_instr ? cur_instr : -1;
								if (r->volume >= 0 && r->volume != prv_volume)
								{
									next_event.volume = r->volume;
									prv_volume = r->volume;
								}
								else
								{
									next_event.volume = -1;
								}
								if (r->detune >= 0 && r->detune != prv_detune)
								{
									next_event.detune = r->detune;
									prv_detune = r->detune;
								}
								else
								{
									next_event.detune = -1;
								}
								if (r->tempo >= 0x20)
									next_event.tempo = convert_tempo(song->speed, r->tempo);
								else if (r->tempo >= 0)
									next_event.tempo = convert_tempo(r->tempo, song->tempo);
								next_event.duration = -1;
								
								if (chn == CHN_DMC)
								{
									if (r->note == 24) next_event.note = 1;
									else if (r->note == 25) next_event.note = 5;
									else next_event.note = -1;
									next_event.volume = -1;
									next_event.instrument = -1;
								}
								
								if (chn == CHN_NOI || chn == CHN_DMC)
									next_event.detune = -1;
								
								prv_instr = cur_instr;
							}
						}
						
						next_event.duration++;
					}
					
					
					
					song->orderlist[chn][ord] = add_module_pattern(&p);
				}
			}
			
			
			
			
			
		}
		
		
		
		
		
		
		add_module_song(song);
	}
	
	
	
	free(ft_pattern_tbl);
	
	return 0;
}






int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		puts("usage: music-convert outname modules...");
		return EXIT_FAILURE;
	}
	
	/**********************************/
	for (int i = 2; i < argc; i++)
		read_ft_module(argv[i]);
	
	
	
	
	/*********** generate duration table **************************/
	typedef struct {
		uint8_t length;
		uint8_t data[4];
	} duration_ent_t;
	
	typedef struct {
		unsigned id;
		unsigned freq;
	} histogram_ent_t;
	
	int histogram_cmp_desc(const void *a, const void *b)
	{
		unsigned fa = ((const histogram_ent_t*)a)->freq;
		unsigned fb = ((const histogram_ent_t*)b)->freq;
		
		return -((fa > fb) - (fa < fb));
	}
	
	
	
	
	duration_ent_t pattern_duration_tbl[512];
	memset(&pattern_duration_tbl,0,sizeof(pattern_duration_tbl));
	for (unsigned pi = 0; pi < module_patterns; pi++)
	{
		pattern_t *p = &module_pattern_tbl[pi];
		
		histogram_ent_t hi[0x80];
		for (unsigned i = 0; i < 0x80; i++)
		{
			hi[i].id = i;
			hi[i].freq = 0;
		}
		
		for (unsigned i = 0; i < p->length; i++)
			hi[p->data[i].duration].freq++;
		
		
		qsort(hi, 0x80,sizeof(*hi), histogram_cmp_desc);
		duration_ent_t *de = &pattern_duration_tbl[pi];
		for (de->length = 0; de->length < 4; de->length++)
		{
			if (!hi[de->length].freq) break;
			de->data[de->length] = hi[de->length].id;
		}
	}
	
	
	
	uint8_t duration_tbl_length = 0;
	uint8_t duration_tbl_map[512];
	uint8_t pattern_duration_map[512];
	histogram_ent_t duration_histogram[512];
	for (unsigned i = 0; i < 512; i++)
	{
		duration_histogram[i].id = i;
		duration_histogram[i].freq = 0;
	}
	
	for (unsigned target_count = 4; target_count > 0; target_count--)
	{
		for (unsigned pi = 0; pi < module_patterns; pi++)
		{
			duration_ent_t *od = &pattern_duration_tbl[pi];
			if (od->length != target_count) continue;
			
			unsigned newi = duration_tbl_length;
			for (unsigned cdi = 0; cdi < duration_tbl_length; cdi++)
			{
				duration_ent_t *cd = &pattern_duration_tbl[duration_tbl_map[cdi]];
				if (cd->length < od->length) continue;
				
				unsigned matches = 0;
				for (unsigned i = 0; i < od->length; i++)
				{
					if (memchr(cd->data,od->data[i],cd->length)) matches++;
				}
				if (matches == od->length)
				{
					newi = cdi;
					break;
				}
			}
			
			
			if (newi == duration_tbl_length)
			{
				duration_tbl_map[duration_tbl_length++] = pi;
			}
			
			pattern_duration_map[pi] = newi;
			duration_histogram[newi].freq++;
		}
	}
	
	
	qsort(duration_histogram, duration_tbl_length, sizeof(*duration_histogram), histogram_cmp_desc);
	unsigned sorted_duration_map[512];
	for (unsigned i = 0; i < duration_tbl_length; i++)
		sorted_duration_map[duration_histogram[i].id] = i;
	
	
	
	
	
	
	
	/************************* export *******************/
	FILE *f = fopen(argv[1],"w");
	
	
	fprintf(f,"m_duration_tbl\n");
	for (unsigned i = 0; i < min(duration_tbl_length,0x40); i++)
	{
		duration_ent_t *d = &pattern_duration_tbl[duration_tbl_map[duration_histogram[i].id]];
		
		fprintf(f," .byte %u,%u,%u,%u\n",d->data[0],d->data[1],d->data[2],d->data[3]);
	}
	
	fputc('\n',f);
	
	
	/****************************/
	
	fprintf(f,"m_song_tempo .byte ");
	for (unsigned s = 0; s < module_songs; s++)
	{
		fprintf(f,"%u",convert_tempo(module_song_tbl[s].speed,module_song_tbl[s].tempo));
		if (s < module_songs-1) fputc(',',f);
	}
	fputc('\n',f);
	
	fprintf(f,"m_song_tracks .byte ");
	for (unsigned s = 0; s < module_songs; s++)
	{
		fprintf(f,"$%02x",module_song_tbl[s].channel_usage);
		if (s < module_songs-1) fputc(',',f);
	}
	fputc('\n',f);
	
	unsigned seq_count = 0;
	fprintf(f,"m_song_seq .byte ");
	for (unsigned s = 0; s < module_songs; s++)
	{
		fprintf(f,"%u",seq_count);
		if (s < module_songs-1) fputc(',',f);
		
		unsigned usage = module_song_tbl[s].channel_usage;
		for (unsigned chn = 0; chn < MAX_TRACKS; chn++)
			if (usage & (0x10 >> chn))
				seq_count++;
	}
	fputc('\n',f);
	
	
	/*********************************/
	fputc('\n',f);
	fprintf(f,"m_seq_tbl_lo .byte ");
	for (unsigned s = 0; s < seq_count; s++)
	{
		fprintf(f,"<m_seq_%u",s);
		if (s < seq_count-1) fputc(',',f);
	}
	fputc('\n',f);
	fprintf(f,"m_seq_tbl_hi .byte ");
	for (unsigned s = 0; s < seq_count; s++)
	{
		fprintf(f,">m_seq_%u",s);
		if (s < seq_count-1) fputc(',',f);
	}
	fputc('\n',f);
	
	
	
	/*********************************/
	fputc('\n',f);
	fprintf(f,"m_instr_env_tbl .char ");
	for (unsigned s = 0; s < module_instruments; s++)
	{
		fprintf(f,"%i",module_instrument_tbl[s].env);
		if (s < module_instruments-1) fputc(',',f);
	}
	fputc('\n',f);
	fprintf(f,"m_instr_pitch_env_tbl .char ");
	for (unsigned s = 0; s < module_instruments; s++)
	{
		fprintf(f,"%i",module_instrument_tbl[s].pitch);
		if (s < module_instruments-1) fputc(',',f);
	}
	fputc('\n',f);
	
	
	/*********************************/
	fputc('\n',f);
	fprintf(f,"m_env_tbl_lo .byte ");
	for (unsigned s = 0; s < module_envelopes; s++)
	{
		fprintf(f,"<m_env_%u",s);
		if (s < module_envelopes-1) fputc(',',f);
	}
	fputc('\n',f);
	fprintf(f,"m_env_tbl_hi .byte ");
	for (unsigned s = 0; s < module_envelopes; s++)
	{
		fprintf(f,">m_env_%u",s);
		if (s < module_envelopes-1) fputc(',',f);
	}
	fputc('\n',f);
	
	/*********************************/
	fputc('\n',f);
	fprintf(f,"m_pitch_env_tbl_lo .byte ");
	for (unsigned s = 0; s < module_pitch_envelopes; s++)
	{
		fprintf(f,"<m_pitch_env_%u",s);
		if (s < module_pitch_envelopes-1) fputc(',',f);
	}
	fputc('\n',f);
	fprintf(f,"m_pitch_env_tbl_hi .byte ");
	for (unsigned s = 0; s < module_pitch_envelopes; s++)
	{
		fprintf(f,">m_pitch_env_%u",s);
		if (s < module_pitch_envelopes-1) fputc(',',f);
	}
	fputc('\n',f);
	
	/************************************/
	fputc('\n',f);
	for (unsigned i = 0; i < 79; i++)
		fputc(';',f);
	fputc('\n',f);
	fputc('\n',f);
	
	
	/**************************************/
	for (unsigned s = 0; s < module_envelopes; s++)
	{
		envelope_t *e = &module_envelope_tbl[s];
		fprintf(f,"m_env_%u .byte ",s);
		
		for (unsigned i = 0; i < e->length; i++)
		{
			fprintf(f,"$%02X",e->data[i]);
			fputc(',',f);
		}
		fprintf(f," 0\n");
	}
	fputc('\n',f);
	
	/**************************************/
	for (unsigned s = 0; s < module_pitch_envelopes; s++)
	{
		pitch_envelope_t *e = &module_pitch_envelope_tbl[s];
		fprintf(f,"m_pitch_env_%u .char ",s);
		
		for (unsigned i = 0; i < e->length; i++)
		{
			fprintf(f,"%i",e->data[i]);
			fputc(',',f);
		}
		if (e->loop == 255)
			fprintf(f," -128\n");
		else
			fprintf(f," -127,%u\n",e->loop);
	}
	fputc('\n',f);
	
	/************************************/
	fputc('\n',f);
	for (unsigned i = 0; i < 79; i++)
		fputc(';',f);
	fputc('\n',f);
	fputc('\n',f);
	
	
	/*************************************/
	unsigned cur_seq = 0;
	for (unsigned s = 0; s < module_songs; s++)
	{
		song_t *sp = &module_song_tbl[s];
		for (int chn = MAX_TRACKS-1; chn >= 0; chn--)
		{
			if (sp->channel_usage & (0x10 >> chn))
			{
				fprintf(f,"m_seq_%u .byte ",cur_seq++);
				
				for (unsigned i = 0; i < sp->orders; i++)
				{
					unsigned p = sp->orderlist[chn][i];
					fprintf(f,">m_ptn_%u,<m_ptn_%u, ",p,p);
				}
				fprintf(f," $%02X\n", sp->loop_point < 0x80 ? sp->loop_point : 0x7f);
			}
		}
	}
	
	/************************************/
	fputc('\n',f);
	for (unsigned i = 0; i < 79; i++)
		fputc(';',f);
	fputc('\n',f);
	fputc('\n',f);
	
	/*************************************/
	for (unsigned pi = 0; pi < module_patterns; pi++)
	{
		pattern_t *p = &module_pattern_tbl[pi];
		
		unsigned di = pattern_duration_map[pi];
		duration_ent_t *dt = &pattern_duration_tbl[duration_tbl_map[di]];
		
		unsigned sdi = sorted_duration_map[di];
		if (sdi >= 0x40)
		{
			unsigned besti = 0;
			unsigned bestc = 0;
			
			for (unsigned cdi = 0; cdi < 0x40; cdi++)
			{
				duration_ent_t *cdt = &pattern_duration_tbl[duration_tbl_map[duration_histogram[di].id]];
				unsigned matches = 0;
				for (unsigned i = 0; i < dt->length; i++)
					if (memchr(cdt->data, dt->data[i], cdt->length))
						matches++;
				if (matches > bestc)
				{
					besti = cdi;
					bestc = matches;
				}
			}
			
			sdi = besti;
			dt = &pattern_duration_tbl[duration_tbl_map[duration_histogram[besti].id]];
		}
		
		
		/*************** find base note ***************/
		unsigned best_note = 0;
		unsigned best_match = 0;
		for (unsigned t = 0; t < 0x30; t++)
		{
			unsigned match = 0;
			for (unsigned i = 0; i < p->length; i++)
			{
				unsigned n = p->data[i].note;
				int diff = n - t;
				if (diff < 0x1d && diff >= 0) match++;
			}
			
			if (match > best_match)
			{
				best_note = t;
				best_match = match;
			}
		}
		
		fprintf(f,"m_ptn_%u .byte $%02X,%u, ",pi, sdi<<2,best_note);
		int pd = -1;
		
		for (unsigned i = 0; i < p->length; i++)
		{
			note_event_t *e = &p->data[i];
			unsigned vd = 0;
			unsigned vn = 0;
			
			int edi = -1;
			for (unsigned i = 0; i < dt->length; i++)
			{
				if (e->duration == dt->data[i])
				{
					edi = i;
					break;
				}
			}
			
			
			if (edi >= 0)
			{
				vd = edi << 5;
			}
			else if (e->duration != pd)
			{
				vd = 0x80;
				pd = e->duration;
			}
			else
			{
				vd = 0xa0;
			}
			
			
			int diff = e->note - best_note;
			if (e->note == -1)
				vn = 0x1f;
			else if (e->note == -2)
				vn = 0x1d;
			else if (diff >= 0 && diff < 0x1d)
				vn = diff;
			else
				vn = 0x1e;
			
			
			if (e->tempo >= 0)
				fprintf(f,"$fe,$%02X,",e->tempo);
			if (e->detune >= 0)
				fprintf(f,"$fd,$%02X,",e->detune);
			if (e->instrument >= 0)
				fprintf(f,"$%02X,",e->instrument + 0xd0);
			if (e->volume >= 0)
				fprintf(f,"$%02X,",e->volume + 0xc0);
			fprintf(f,"$%02X,",vd | vn);
			if (vd == 0x80)
				fprintf(f,"$%02X,",e->duration);
			if (vn == 0x1e)
				fprintf(f,"$%02X,",e->note);
			
			
			fputc(' ',f);
		}
		
		fprintf(f," $ff\n");
	}
	
	
	
	fclose(f);
}