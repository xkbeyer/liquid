
#if defined(_MSC_VER)
#define DECLSPEC __declspec(dllexport) 
#else
#define DECLSPEC 
#endif


extern "C" DECLSPEC int printvalue( int val );
extern "C" DECLSPEC double printdouble( double val );
extern "C" DECLSPEC void display( char * str, ... );
extern "C" DECLSPEC void displayln( char * str, ... );
