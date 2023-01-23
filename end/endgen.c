#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>



#define SRC_IMAGE_WIDTH 783
#define SRC_IMAGE_HEIGHT 394

#define SRC_FRAME_WIDTH 192
#define SRC_FRAME_HEIGHT 128

#define SRC_FRAME_WIDTH_TILES (SRC_FRAME_WIDTH/8)
#define SRC_FRAME_HEIGHT_TILES (SRC_FRAME_HEIGHT/8)

#define SRC_IMAGE_FRAME_X_OFFS 197
#define SRC_IMAGE_FRAME_Y_OFFS 133

#define SRC_FRAMES 12
#define SRC_FRAMES_PER_ROW 4





#define ROM_TILE_OFFS 0x34010
#define ROM_TILE_BANK_SIZE 0x2000
#define ROM_TILE_BANK_TILES 256

const uint8_t tile_bank_remap_tbl[] = {0,1,2,3,4,5,6,7, 10,9,8, 11};

const uint8_t tile_bank_keep_tbl[ROM_TILE_BANK_TILES] = {
      /* 0 1 2 3 4 5 6 7 8 9 A B C D E F */
/* 0x */ 0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,
/* 1x */ 0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,
/* 2x */ 1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
/* 3x */ 1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
/* 4x */ 1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
/* 5x */ 1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
/* 6x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 7x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 8x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 9x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* Ax */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* Bx */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* Cx */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* Dx */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* Ex */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* Fx */ 0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1
};



const uint8_t color_remap_tbl[] = {3,0,2,1,1};



typedef uint8_t tile_t[2][8];

tile_t frame_tiles[SRC_FRAMES][SRC_FRAME_HEIGHT_TILES][SRC_FRAME_WIDTH_TILES];

tile_t out_tiles[SRC_FRAMES][ROM_TILE_BANK_TILES];



#define TILEMAP_WIDTH 32
#define TILEMAP_HEIGHT 30

#define BORDER_WIDTH_TILES 4
#define BORDER_HEIGHT_TILES 4

#define FRAME_X_OFFS_TILES (TILEMAP_WIDTH - BORDER_WIDTH_TILES - SRC_FRAME_WIDTH_TILES)
#define FRAME_Y_OFFS_TILES (TILEMAP_HEIGHT - BORDER_HEIGHT_TILES - SRC_FRAME_HEIGHT_TILES)

#define BORDER_TILE 0xff
#define BG_TILE 0xfc

uint8_t tile_bank_usage_tbl[ROM_TILE_BANK_TILES];

uint8_t out_tilemap[TILEMAP_HEIGHT][TILEMAP_WIDTH];






/*****************************************************************************/

int main(int argc, char *argv[])
{
	if (argc != 5)
	{
		puts("usage: endgen srcrom srcimage tilesout nameout");
		return EXIT_FAILURE;
	}
	FILE *inf;
	
	
	/************************** read in old tiles from rom ************************/
	inf = fopen(argv[1],"rb");
	if (!inf)
	{
		printf("can't open rom: %s\n",strerror(errno));
		return EXIT_FAILURE;
	}
	fseek(inf, ROM_TILE_OFFS, SEEK_SET);
	
	for (unsigned frame = 0; frame < SRC_FRAMES; frame++)
	{
		for (unsigned ti = 0; ti < ROM_TILE_BANK_TILES; ti++)
		{
			tile_t t;
			if (sizeof(t) != fread(t,1,sizeof(t),inf))
			{
				printf("error reading rom tiles: %s\n",strerror(errno));
				fclose(inf);
				return EXIT_FAILURE;
			}
			
			tile_t *nt = &out_tiles[frame][ti];
			if (tile_bank_keep_tbl[ti])
				memcpy(nt,t,sizeof(t));
			else
				memset(nt,0,sizeof(t));
		}
	}
	fclose(inf);
	
	
	
	
	/************************* convert source image to tile data *********************/
	memset(frame_tiles,0,sizeof(frame_tiles));
	
	inf = fopen(argv[2],"rb");
	if (!inf)
	{
		printf("can't open image: %s\n",strerror(errno));
		return EXIT_FAILURE;
	}
	
	for (unsigned y = 0; y < SRC_IMAGE_HEIGHT; y++)
	{
		for (unsigned x = 0; x < SRC_IMAGE_WIDTH; x++)
		{
			int c = fgetc(inf);
			if (c == EOF)
			{
				printf("error reading image: %s\n",strerror(errno));
				fclose(inf);
				return EXIT_FAILURE;
			}
			unsigned c2 = color_remap_tbl[c];
			
			unsigned frame_column = x / SRC_IMAGE_FRAME_X_OFFS;
			unsigned frame_row = y / SRC_IMAGE_FRAME_Y_OFFS;
			unsigned frame_x = x % SRC_IMAGE_FRAME_X_OFFS;
			unsigned frame_y = y % SRC_IMAGE_FRAME_Y_OFFS;
			
			unsigned frame_ = (frame_row*SRC_FRAMES_PER_ROW) + frame_column;
			unsigned frame = tile_bank_remap_tbl[frame_];
			
			if (frame_x < SRC_FRAME_WIDTH && frame_y < SRC_FRAME_HEIGHT)
			{
				unsigned tx = frame_x / 8;
				unsigned ty = frame_y / 8;
				unsigned px = frame_x % 8;
				unsigned py = frame_y % 8;
				
				tile_t *t = &frame_tiles[frame][ty][tx];
				if (c2 & 1)
					(*t)[0][py] |= (0x80 >> px);
				if (c2 & 2)
					(*t)[1][py] |= (0x80 >> px);
			}
		}
	}
	fclose(inf);
	
	
	
	
	
	
	/************************** generate output tiles and nametable *******************/
	memcpy(tile_bank_usage_tbl,tile_bank_keep_tbl, ROM_TILE_BANK_TILES);
	
	for (unsigned ty = 0; ty < TILEMAP_HEIGHT; ty++)
	{
		for (unsigned tx = 0; tx < TILEMAP_WIDTH; tx++)
		{
			/********* border tile? ************/
			if (ty < BORDER_HEIGHT_TILES ||
				tx < BORDER_WIDTH_TILES ||
				(TILEMAP_HEIGHT - ty) <= BORDER_HEIGHT_TILES ||
				(TILEMAP_WIDTH - tx) <= BORDER_WIDTH_TILES )
			{
				out_tilemap[ty][tx] = BORDER_TILE;
			}
			/********* bg tile? ************/
			else if (ty < FRAME_Y_OFFS_TILES ||
				tx < FRAME_X_OFFS_TILES)
			{
				out_tilemap[ty][tx] = BG_TILE;
			}
			/********* this is an actual frame tile ***********/
			else
			{
				unsigned ftx = tx - FRAME_X_OFFS_TILES;
				unsigned fty = ty - FRAME_Y_OFFS_TILES;
				
				/******* first check if any of the used tiles are a match ******/
				int tileid = -1;
				int freetile = -1; /* (also keep track of the next empty tile) */
				for (unsigned ti = 0; ti < ROM_TILE_BANK_TILES; ti++)
				{
					if (tile_bank_usage_tbl[ti])
					{
						for (unsigned frame = 0; frame < SRC_FRAMES; frame++)
						{
							tile_t *t1 = &frame_tiles[frame][fty][ftx];
							tile_t *t2 = &out_tiles[frame][ti];
							
							if (memcmp(t1,t2,sizeof(*t1)))
								goto frame_no_match;
						}
						tileid = ti;
						break;
						
frame_no_match:;
					}
					else if (freetile < 0)
					{
						freetile = ti;
					}
				}
				
				
				
				/******* if there is no match, add the tiles to the chr banks ******/
				if (tileid < 0)
				{
					if (freetile < -1)
					{
						puts("no more free tiles");
						return EXIT_FAILURE;
					}
					tileid = freetile;
					
					for (unsigned frame = 0; frame < SRC_FRAMES; frame++)
					{
						tile_t *t1 = &frame_tiles[frame][fty][ftx];
						tile_t *t2 = &out_tiles[frame][tileid];
						
						memcpy(t2,t1,sizeof(*t1));
					}
					
					tile_bank_usage_tbl[tileid] = 1;
				}
				
				out_tilemap[ty][tx] = tileid;
			}
		}
	}
	
	
	
	
	/**************************************** output **********************************/
	FILE *of;
	of = fopen(argv[3],"wb");
	if (!of)
	{
		printf("can't open tilesout: %s\n",strerror(errno));
		return EXIT_FAILURE;
	}
	fwrite(out_tiles,1,sizeof(out_tiles),of);
	fclose(of);
	
	of = fopen(argv[4],"wb");
	if (!of)
	{
		printf("can't open nameout: %s\n",strerror(errno));
		return EXIT_FAILURE;
	}
	fwrite(out_tilemap,1,sizeof(out_tilemap),of);
	fclose(of);
	
	
	
	
}



