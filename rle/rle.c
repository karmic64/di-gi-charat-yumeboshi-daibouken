#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>


#define MAX_LIT_LEN 0x7f
#define MAX_RUN_LEN 0x82

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		puts("usage: rle inname outname");
		return EXIT_FAILURE;
	}
	
	FILE *inf = fopen(argv[1],"rb");
	if (!inf)
	{
		printf("couldn't open %s: %s\n",argv[1],strerror(errno));
		return EXIT_FAILURE;
	}
	FILE *of = fopen(argv[2],"wb");
	if (!of)
	{
		printf("couldn't open %s: %s\n",argv[2],strerror(errno));
		return EXIT_FAILURE;
	}
	
	
	uint8_t litbuf[MAX_LIT_LEN];
	uint8_t litlen = 0;
	
	int endflag = 0;
	int nextendflag = 0;
	
	while (!endflag)
	{
		if (nextendflag) endflag = 1;
		
		/* get next run length */
		int runch = -1;
		int runlen = 0;
		
		if (!nextendflag)
		{
			while (1)
			{
				int ch = fgetc(inf);
				if (runch < 0)
				{
					if (ch == EOF)
					{
						endflag = 1;
					}
					else
					{
						runch = ch;
						runlen = 1;
					}
				}
				else
				{
					if (ch == runch)
					{
						runlen++;
						if (runlen == MAX_RUN_LEN) break;
					}
					else
					{
						if (ch == EOF)
							nextendflag = 1;
						else
							ungetc(ch,inf);
						break;
					}
				}
			}
		}
		
		
		
		/* see if we should output */
		
		int flushflag = 0;
		if (nextendflag || endflag) /* always flush if this is the last run */
			flushflag = 1;
		else if (runlen >= 3) /* if the next run is large */
			flushflag = 1;
		else if (litlen + runlen > MAX_LIT_LEN) /* or if the next run is too big */
			flushflag = 1;
		
		if (flushflag && litlen > 0)
		{
			fputc(litlen,of);
			fwrite(litbuf,1,litlen,of);
			litlen = 0;
		}
		
		
		/* either append to the literal or output a run */
		if (runlen >= 3)
		{
			fputc(0xff-(runlen-3),of);
			fputc(runch,of);
		}
		else
		{
			memset(litbuf+litlen,runch,runlen);
			litlen += runlen;
		}
		
		
		
	}
	
	
	
	fputc(0,of);
	fclose(inf);
	fclose(of);
}