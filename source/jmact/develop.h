#ifndef _develop_public
#define _develop_public

#define DEVELOPMENT          0
//#define SW_SHAREWARE            0
#define LOCATIONINFO         1
#define SOFTWAREERROR        1
#define MEMORYCORRUPTIONTEST 1
#define PRECACHETEST         0
#define DATACORRUPTIONTEST   0
#define RANDOMNUMBERTEST     0


#if ( LOCATIONINFO == 1 )

#define funcstart() \
   { \
   SoftError( "funcstart : module '%s' at line %d.\n", __FILE__, __LINE__ );\
   }

#define funcend() \
   { \
   SoftError( "  funcend : module '%s' at line %d.\n", __FILE__, __LINE__ );\
   }

#else

#define funcstart()
#define funcend()

#endif

#endif
