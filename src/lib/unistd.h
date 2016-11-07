#ifndef _UNISTD_H
#define _UNISTD_H    1

#ifdef _WIN32
    #include <io.h>
    #include <stdint.h>
 
    typedef uint8_t u_int8_t;
    typedef uint16_t u_int16_t;
    typedef uint32_t u_int32_t;

#else
    #include <unistd.h>
#endif

#ifndef _SSIZE_T_DEFINED
    #ifdef  _WIN32   
        typedef unsigned __int32 ssize_t;
    #else
        typedef _W32 unsigned int ssize_t;
    #endif
#define _SSIZE_T_DEFINED
#endif

#endif /* unistd.h  */
