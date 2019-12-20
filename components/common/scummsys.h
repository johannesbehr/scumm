#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <assert.h>
#include <ctype.h>
#include <new>
#include <math.h>

/*
void* operator new (std::size_t size) throw (std::bad_alloc);
void* operator new (std::size_t size, const std::nothrow_t& nothrow_constant) throw();
void* operator new (std::size_t size, void* ptr) throw();
*/

typedef unsigned char byte;
typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned short uint16;
typedef signed short int16;
typedef unsigned int uint32;
typedef signed int int32;
typedef unsigned int uint;
typedef unsigned long uint64;
typedef long int64;

#define SCUMM_LITTLE_ENDIAN
#define SCUMM_NEED_ALIGNMENT
//#define NORETURN_PRE void
//#define error(a...) printf(a); exit(1);
#define warning(a...) printf(a)
//#define usage(a...) printf(a)
#define FORCEINLINE inline
#define STATIC_ASSERT(a,b) assert(a)
#define STRINGBUFLEN 256
#define GCC_ATLEAST(major, minor) (__GNUC__ > (major) || (__GNUC__ == (major) && __GNUC_MINOR__ >= (minor)))
#define GCC_PRINTF(x,y) __attribute__((__format__(__printf__, x, y)))
#define PACKED_STRUCT __attribute__((__packed__))
//#define MAXPATHLEN 128
#define POSIX_MAXPATHLEN 128
#define POSIX
//int getcwd(char *buffer,size_t size);// {return 1;}
//#define getcwd f_getcwd

void initGraphics(uint screenWidth,uint screenHeight);

#define NORETURN_PRE
#define NORETURN_POST
/*
#ifndef NORETURN_PRE
	#if defined(_MSC_VER)
		#define NORETURN_PRE __declspec(noreturn)
	#else
		#define NORETURN_PRE
	#endif
#endif

#ifndef NORETURN_POST
	#if defined(__GNUC__) || defined(__INTEL_COMPILER)
		#define NORETURN_POST __attribute__((__noreturn__))
	#else
		#define NORETURN_POST
	#endif
#endif
*/
#ifndef scumm_va_copy
	#if defined(va_copy)
		#define scumm_va_copy va_copy
	#elif defined(__va_copy)
		#define scumm_va_copy __va_copy
	#elif defined(_MSC_VER) || defined(__SYMBIAN32__)
		#define scumm_va_copy(dst, src)       ((dst) = (src))
	#else
		#error scumm_va_copy undefined for this port
	#endif
#endif