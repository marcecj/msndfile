#ifndef __EXPORT_MACROS_H__
#define __EXPORT_MACROS_H__

/* Taken with slight modifications from https://gcc.gnu.org/wiki/Visibility */

#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_DLL
    #ifdef __GNUC__
      #define EXPORT __attribute__ ((dllexport))
    #else
      #define EXPORT __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #else
    #ifdef __GNUC__
      #define EXPORT __attribute__ ((dllimport))
    #else
      #define EXPORT __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #endif
  #define NOEXPORT
#else
  #if __GNUC__ >= 4
    #define EXPORT __attribute__ ((visibility ("default")))
    #define NOEXPORT  __attribute__ ((visibility ("hidden")))
  #else
    #define EXPORT
    #define NOEXPORT
  #endif
#endif

#endif /* __EXPORT_MACROS_H__ */
