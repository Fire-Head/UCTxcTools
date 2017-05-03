#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <io.h>


//================================================
void CreatePath( const char *path )
{
	char   *ofs, c, *str;

	str = strdup( path );

	for ( ofs = str + 1; *ofs; ofs++ )
	{
		c = *ofs;
		if ( c == '/' || c == '\\' )
		{		// create the directory
			*ofs = 0;
			mkdir( str );
			*ofs = c;
		}
	}
	free( str );
}

//================================================
bool SaveFile( const char *fname, int size, unsigned char * data )
{
	FILE   *f;

	if ( !data )
		return false;

	CreatePath( fname );
	f = fopen( fname, "wb" );

	if ( !f )
		return false;

	fwrite( data, size, 1, f );
	fclose( f );

	return true;

};

//================================================


void toFile(char *filename, char *message, ...)
{
	char chBuffer[4096];
	
	va_list arg;
	va_start(arg, message);
	_vsnprintf(chBuffer, 4095, message, arg);
	chBuffer[4095] = '\0';

	FILE *log = fopen(filename, "ab");
	if(log)
	{
		fprintf(log, "%s\r\n", chBuffer);
		fclose(log);
	}	
}

int main(int argc, char **argv)
{
	int nNumElements = 0;
	
    if ( argc < 2 )
    {
		printf("usage: %s <txcfile>\n", argv[0]);
		return 0;
	}
	
	FILE *f = fopen(argv[1], "rb");
	if ( f )
	{
		fread(&nNumElements, sizeof(nNumElements), 1, f);
		
		int *offsets = new int[nNumElements];
		int *sizes = new int[nNumElements];
		
		fread(offsets, sizeof(int), nNumElements, f);
		fread(sizes, sizeof(int), nNumElements, f);
		
		for ( int i = 0; i < nNumElements; i++ )
		{
			char buff[1024];
			char batbuff[1024];
			sprintf(buff, "%s out\\tex%d.tex", argv[1], i);
			sprintf(batbuff, "%s out 2 tga.bat", argv[1], i);
	
			
			if ( offsets[i] != 0 && sizes[i] != 0 )
			{
				fseek(f, offsets[i], SEEK_SET);
				
				unsigned char *data = new unsigned char[sizes[i]];
				
				fread(data, sizeof(char), sizes[i], f);
				
				printf("filename: %s, offset: 0x%X, filesize: %d\n", buff, offsets[i], sizes[i]);
				SaveFile(buff, sizes[i], data);
				toFile(batbuff, "tex2tga.exe \"%s out\\tex%d.tex\" \"%s out\\tex%d.tga\"",  argv[1], i, argv[1], i);
	
				delete[] data;
			}
			else
				printf("[Error] Empty file: filename: %s, offset: 0x%X, size: %d\n", buff, offsets[i], sizes[i]);
		}
		
		delete[] offsets;
		delete[] sizes;
	
		fclose(f);
	}
	else
	{
		printf("cannot open %s\n", argv[1]);
		return 0;
	}
}