#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "pretiles.h"



#define BG_COL 12
#define AMT_PALETTES 3
const uint8_t palettes[AMT_PALETTES][3] = {
	{56,13,28},
	{37,13,42},
	{32,27,255}
};


#define IMG_WIDTH 256
#define IMG_HEIGHT 240
#define MAX_TILES 0xcf
#define FIRST_PRETILE 0xcf
#define AMT_PRETILES 0x31
unsigned tile_cnt = 0;
uint8_t image[IMG_HEIGHT][IMG_WIDTH];
uint8_t tiles[MAX_TILES][2][8];
uint8_t name[IMG_HEIGHT/8][IMG_WIDTH/8];
uint8_t attrib[0x40];


int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		puts("usage: titlegen inname tilesout nameout");
		return EXIT_FAILURE;
	}
	
	FILE *f = fopen(argv[1],"rb");
	if (!f)
	{
		printf("can't open image: %s\n",strerror(errno));
		return EXIT_FAILURE;
	}
	if (fread(image,1,sizeof(image),f) != sizeof(image))
	{
		fclose(f);
		printf("error reading image: %s\n",strerror(errno));
		return EXIT_FAILURE;
	}
	fclose(f);
	
	
	/****************** generate attributes *************************/
	memset(attrib,0,sizeof(attrib));
	for (unsigned yblk = 0; yblk < IMG_HEIGHT/16; yblk++)
	{
		for (unsigned xblk = 0; xblk < IMG_WIDTH/16; xblk++)
		{
			unsigned used_color_cnt = 0;
			uint8_t used_colors[3];
			for (unsigned ypix = 0; ypix < 16; ypix++)
			{
				for (unsigned xpix = 0; xpix < 16; xpix++)
				{
					unsigned x = xblk*16 + xpix;
					unsigned y = yblk*16 + ypix;
					uint8_t col = image[y][x];
					if (col == BG_COL) continue;
					if (!used_color_cnt || !memchr(used_colors,col,used_color_cnt))
					{
						if (used_color_cnt == 3)
						{
							printf("too many colors at %u,%u\n",x,y);
							return EXIT_FAILURE;
						}
						used_colors[used_color_cnt++] = col;
					}
				}
			}
			
			int best_palette = -1;
			for (unsigned pal = 0; pal < AMT_PALETTES; pal++)
			{
				unsigned matches = 0;
				for (unsigned i = 0; i < used_color_cnt; i++)
				{
					if (memchr(palettes[pal], used_colors[i], 3))
						matches++;
				}
				if (matches == used_color_cnt)
				{
					best_palette = pal;
					break;
				}
			}
			if (best_palette < 0)
			{
				printf("no suitable palette for %u,%u\n",xblk*16,yblk*16);
				return EXIT_FAILURE;
			}
			
			unsigned attribindex = (yblk/2 * 8) + (xblk/2);
			attrib[attribindex] |= best_palette << (yblk & 1 ? 4 : 0) << (xblk & 1 ? 2 : 0);
		}
	}
	
	
	/*************** make tiles and tilemap ***************************/
	for (unsigned ytile = 0; ytile < IMG_HEIGHT/8; ytile++)
	{
		for (unsigned xtile = 0; xtile < IMG_WIDTH/8; xtile++)
		{
			unsigned attribindex = (ytile/4 * 8) + (xtile/4);
			unsigned palette = (attrib[attribindex] >> (ytile & 2 ? 4 : 0) >> (xtile & 2 ? 2 : 0)) & 3;
			
			uint8_t tile[2][8];
			memset(tile,0,sizeof(tile));
			for (unsigned ypix = 0; ypix < 8; ypix++)
			{
				for (unsigned xpix = 0; xpix < 8; xpix++)
				{
					unsigned x = xtile*8 + xpix;
					unsigned y = ytile*8 + ypix;
					uint8_t col = image[y][x];
					if (col == BG_COL) continue;
					
					uint8_t *pval = memchr(palettes[palette],col,3);
					if (!pval)
					{
						printf("bad color at %u,%u\n",x,y);
						return EXIT_FAILURE;
					}
					uint8_t pi = (pval - palettes[palette]) + 1;
					if (pi & 1) tile[0][ypix] |= (0x80 >> xpix);
					if (pi & 2) tile[1][ypix] |= (0x80 >> xpix);
				}
			}
			
			
			unsigned tileid = 0;
			for (tileid = 0; tileid < AMT_PRETILES; tileid++)
			{
				if (!memcmp(tile,&pretiles[tileid * 0x10],sizeof(tile)))
				{
					tileid = tileid + FIRST_PRETILE;
					goto got_tile_id;
				}
			}
			
			for (tileid = 0; tileid < tile_cnt; tileid++)
			{
				if (!memcmp(tile,tiles[tileid],sizeof(tile))) break;
			}
			if (tileid == tile_cnt)
			{
				if (tile_cnt == MAX_TILES)
				{
					printf("too many tiles\n");
					return EXIT_FAILURE;
				}
				memcpy(tiles[tile_cnt++],tile,sizeof(tile));
			}
			
got_tile_id:
			name[ytile][xtile] = tileid;
		}
	}
	
	
	
	/***************** export *********************************/
	f = fopen(argv[2],"wb");
	fwrite(tiles,sizeof(*tiles),tile_cnt,f);
	fclose(f);
	
	f = fopen(argv[3],"wb");
	fwrite(name,1,sizeof(name),f);
	fwrite(attrib,1,sizeof(attrib),f);
	fclose(f);
}
