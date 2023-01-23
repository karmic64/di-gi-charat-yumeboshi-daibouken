ifdef COMSPEC
DOTEXE:=.exe
else
DOTEXE:=
endif



CFLAGS := -s -Ofast -Wall
CLIBS := -lm



ORIG_ROM := "Super Mario Bros. 2 (U) (PRG0) [!].nes"



# DMCS := $(addprefix dmc/,nyu_loudester.dmc nyo_loudester.dmc usadahurt_loudester.dmc gema_loudester.dmc)
DMCS := $(wildcard dmc/*.dmc)
SONGS := music/modules/0600_01.txt music/modules/0600_02.txt music/modules/0600_04.txt music/modules/0600_08.txt music/modules/0600_10.txt music/modules/0600_20.txt music/modules/0600_40.txt music/modules/0600_80.txt music/modules/0603_01.txt music/modules/0603_02.txt music/modules/0603_04.txt music/modules/0603_08.txt music/modules/0603_10.txt music/modules/0603_20.txt

MUS_TOOL := music/music-convert$(DOTEXE)
MUS_OUT := music/out.asm

RLE_TOOL := rle/rle$(DOTEXE)
DOTRLE := .rle
RLE_OUT := title/name-out.rle end/name-out.rle

TITLE_TOOL := title/titlegen$(DOTEXE)
TITLE_SRC := title/unknown.data
TITLE_OUT := $(addprefix title/,tiles-out name-out)

END_TOOL := end/endgen$(DOTEXE)
END_SRC := end/unknown.data
END_OUT := $(addprefix end/,tiles-out name-out)

OUT_SRC := a.asm $(wildcard *.asm) $(wildcard */*.asm)
OUT_ROM := a.nes



$(OUT_ROM): $(OUT_SRC) $(DMCS) $(MUS_OUT) $(TITLE_OUT) $(END_OUT) $(RLE_OUT)
	64tass -Woptimize --long-branch -f -o $@ $<



$(MUS_OUT): $(MUS_TOOL) $(SONGS)
	$(MUS_TOOL) $(MUS_OUT) $(SONGS)


$(TITLE_OUT): $(TITLE_TOOL) $(TITLE_SRC)
	$(TITLE_TOOL) $(TITLE_SRC) $(TITLE_OUT)


$(END_OUT): $(END_TOOL) $(END_SRC)
	$(END_TOOL) $(ORIG_ROM) $(END_SRC) $(END_OUT)



.PHONY: clean
clean:
	$(RM) $(MUS_TOOL) $(MUS_OUT) $(RLE_TOOL) $(RLE_OUT) $(TITLE_TOOL) $(TITLE_OUT) $(END_TOOL) $(END_OUT) $(OUT_ROM)




%$(DOTEXE): %.c
	$(CC) $(CFLAGS) -o $@ $< $(CLIBS)

# stupid hack because appending $(RLE_TOOL) to the %$(DOTRLE) rule doesn't work
$(RLE_OUT): $(RLE_TOOL)

%$(DOTRLE): %
	$(RLE_TOOL) $< $@

