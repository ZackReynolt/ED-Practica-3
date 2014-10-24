/* 
 * File:   millisleep.h
 * Author: ajrueda
 *
 * Created on 21 de octubre de 2014, 17:22
 */

#if defined(WIN32)
  #include <windows.h>
#elif defined(__UNIX__)
  #include <unistd.h>
#else
#endif

int millisleep(unsigned ms)
{
#if defined(WIN32)
  SetLastError(0);
  Sleep(ms);
  return GetLastError() ?-1 :0;
#elif defined(__UNIX__) || defined(__APPLE__)
  return usleep(1000 * ms);
#else
#error ("no milli sleep available for platform")
  return -1;
#endif
}

