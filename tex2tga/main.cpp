#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include "types.h"

#define EXIT(text) { printf(text); exit(EXIT_FAILURE); }

UInt8 LOOKUP_TABLE_4_BIT_TO_8_BIT[16];
UInt8 LOOKUP_TABLE_5_BIT_TO_8_BIT[32];
UInt8 LOOKUP_TABLE_6_BIT_TO_8_BIT[64];

void InitialiseLookupTable()
{
	for (int i = 0; i < 16; i++)
		LOOKUP_TABLE_4_BIT_TO_8_BIT[i] = i | 16 * i;
	
	for (int i = 0; i < 32; i++)
		LOOKUP_TABLE_5_BIT_TO_8_BIT[i] = 8 * i | (i >> 2);
	
	for (int i = 0; i < 64; i++)
		LOOKUP_TABLE_6_BIT_TO_8_BIT[i] = 4 * i | (i >> 4);
}

#pragma pack(push, 1)
struct tTxcTexHeader
{
  Int16 sig;
  Bool16 hasAlpha;
  UInt16 width;
  UInt16 height;
  UInt16 size;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct TGAHeader
{
	UInt8  IDlen;
	UInt8  colorMapType;
	UInt8  imageType;
	UInt16 colorMapOrigin;
	UInt16 colorMapLength;
	UInt8  colorMapDepth;
	UInt16 xOrigin;
	UInt16 yOrigin;
	UInt16 width;
	UInt16 height;
	UInt8 depth;
	UInt8 descriptor;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct RGBA
{
	UInt8 r;
	UInt8 g;
	UInt8 b;
	UInt8 a;
};
#pragma pack(pop)

void ExportTga(const Char *filename, UInt16 w, UInt16 h, RGBA *pixels, Bool hasAlpha)
{
	FILE *f = fopen(filename, "wb");
	
	if ( f )
	{
		TGAHeader header;
		header.IDlen = 0;
		header.colorMapType = 0;
		header.imageType = 2;
		header.colorMapOrigin = 0;
		header.colorMapLength = 0;
		header.colorMapDepth = 0;
		header.xOrigin = 0;
		header.yOrigin = 0;
		header.width = w;
		header.height = h;
		header.depth = 24;
		header.descriptor = 0;
		header.depth = hasAlpha != false ? 32 : 24;

		fwrite(&header, sizeof(TGAHeader), 1, f);

		for ( Int32 y = h - 1; y >= 0; y-- )
		{
			for ( Int32 x = 0; x < w; x++ )
			{
				RGBA &pixel = pixels[(w * y) + x];
				fwrite(&pixel.r, sizeof(UInt8), 1, f);
				fwrite(&pixel.g, 1u, sizeof(UInt8), f);
				fwrite(&pixel.b, sizeof(UInt8), 1, f);
				if ( hasAlpha )
					fwrite(&pixel.a, sizeof(UInt8), 1, f);
			}
		}

		fclose(f);
	}
}

UInt16 *PrepareImage(UInt8 *inData, UInt16 &w, UInt16 &h, Bool &hasAlpha)
{	
	tTxcTexHeader *header = (tTxcTexHeader *)inData;
	UInt16 *imageData = ((UInt16 *)((UInt8 *)inData + sizeof(tTxcTexHeader)));
	
	if ( header->sig != -1 )
		EXIT("[Error] Input file is not an Urban Chaos .tex file\n");
	
	if ( header && header->sig == -1 )
	{
		Int32 wh = header->width * header->height;
		UInt16 *pixels = new UInt16[wh];
	
		hasAlpha = header->hasAlpha;
		w = header->width;
		h = header->height;

		UInt16 *pTexData = imageData;
		UInt32 iSize = (UInt16)header->size;
		UInt16 *pPixels = pixels;
		UInt16 colorTable[0x10000] = {0};
		
		if ( iSize )
		{
			pTexData += iSize;
			memcpy(colorTable, imageData, sizeof(UInt16) * iSize);
		}
		
		int iTempSize = 1;
		if ( iSize > 2 )
		{
			do
			{
				++iTempSize;
			}
			while ( iSize > 1 << iTempSize );
		}

		UInt16 TexPixelsData = *pTexData;
		UInt16 *NextPixelsData = pTexData + 1;

		Int32 n = 16;
		UInt16 *ppPixels = pPixels;
		UInt16 colorIndex;
		
		// convert color palette indexes to pixel color
		
		for ( Int32 i = wh; i; i-- )
		{
			if ( n <= iTempSize )
			{
				UInt8 a = iTempSize - n;
				UInt16 b = (UInt16)TexPixelsData;
				TexPixelsData = *NextPixelsData;
				++NextPixelsData;
				UInt16 c = (UInt16)TexPixelsData >> (16 - (iTempSize - (UInt8)n));
				n = 16 - (iTempSize - n);
				colorIndex = c | (b >> (16 - iTempSize));
				pPixels = ppPixels;
				TexPixelsData <<= a;
			}
			else
			{
				colorIndex = (UInt16)TexPixelsData >> (16 - iTempSize);
				TexPixelsData <<= iTempSize;
				n -= iTempSize;
			}
			
			*pPixels = colorTable[colorIndex];
			++pPixels;
			ppPixels = pPixels;
		}
		
		return pixels;
	}

	return NULL;
}


void UncompressPixels(UInt16 width, UInt16 height, Bool hasAlpha, const UInt16 *data, RGBA *outPixels)
{
	if ( hasAlpha )
	{
		// convert RGBA4444 to RGBA8888
		if ( width * height > 0 )
		{
			for ( Int32 i = width * height; i; i-- )
			{
				Int32 mask = *data & 0xFFFF;
				++data;
				
				outPixels->r = LOOKUP_TABLE_4_BIT_TO_8_BIT[mask & 15];
				outPixels->g = LOOKUP_TABLE_4_BIT_TO_8_BIT[(mask >> 4) & 15];
				outPixels->b = LOOKUP_TABLE_4_BIT_TO_8_BIT[(mask >> 8) & 15];
				outPixels->a = LOOKUP_TABLE_4_BIT_TO_8_BIT[mask >> 12];
				outPixels++;
			}
		}
	}
	else
	{
		// convert RGB565 to RGBA8888
		if ( width * height > 0 )
		{
			for ( Int32 i = width * height; i; i-- )
			{
				Int32 mask = *data & 0xFFFF;
				++data;
				
				outPixels->r = LOOKUP_TABLE_5_BIT_TO_8_BIT[mask & 31];
				outPixels->g = LOOKUP_TABLE_6_BIT_TO_8_BIT[(mask >> 5) & 63];
				outPixels->b = LOOKUP_TABLE_5_BIT_TO_8_BIT[mask >> 11];
				outPixels->a = 0;
				outPixels++;
			}
		}
	}
}

void ConvertTxcToTga(UInt8 *inData, const Char *name, const Char *name2)
{	
	UInt16 w = 0;
	UInt16 h = 0;
	Bool hasAlpha = false;
	UInt16 *image = PrepareImage(inData, w, h, hasAlpha);

	if ( image )
	{
		Char outName[1024];

		RGBA *pixels = new RGBA[w * h];
		UncompressPixels(w, h, hasAlpha, image, pixels);
		
		if ( !name )
			sprintf(outName, "%s.tga", name2);
		else
			sprintf(outName, "%s", name);
	
		ExportTga(outName, w, h, pixels, hasAlpha);
	
		delete[] image;
		delete[] pixels;
	}
}

int main(int argc, char **argv)
{
	if (argc < 3 || strlen(argv[1]) < 1)
	//if (argc < 3 || strlen(argv[1]) < 0)
	{
		printf("usage: %s <infile.tex> <outfile.tga>\n", argv[0]);
		if ( !argv[1] )
			return 0;
	}

	InitialiseLookupTable();
	
	FILE *f = fopen(argv[1], "rb");
	
	if ( f )
	{
		long fileSize = _filelength(_fileno(f));
		
		UInt8 *data = new UInt8[fileSize];
	
		fread(data, sizeof(UInt8), fileSize, f);
	
		ConvertTxcToTga(data, argv[2], argv[1]);
	
		fclose(f);
	
		delete[] data;
	}
	else
		EXIT("[Error] Input file does not exist\n");
}