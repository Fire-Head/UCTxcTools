#pragma once

typedef char Int8;
typedef short Int16;
typedef int Int32;
typedef unsigned char UInt8;
typedef unsigned short UInt16;
typedef unsigned int UInt32;
typedef signed char SInt8;
typedef signed short SInt16;
typedef signed int SInt32;

typedef float Float;
typedef Int32 Int;
typedef Int8 Char;

#ifdef __MINGW32__
typedef bool Bool;
#else
typedef Int8 Bool;
#endif

typedef Int8 Bool8;
typedef Int16 Bool16;
typedef Int32 Bool32;

typedef char Char;
typedef UInt16 WChar;