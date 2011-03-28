#include <iostream>

#include "networking.h"

NET_NAMESPACE_BEGIN

#if defined(WIN32)

/* ---------------------- Windows Implementation ------------------ */

char const*
strerror (int err_code)
{
  static TCHAR err_msg[1024];
  DWORD ret = FormatMessage(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      0,
      err_code,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
      (LPSTR)&err_msg,
      1023,
      0);
  err_msg[ret] = 0;
  return (char const*)&err_msg;
}

bool
init (void)
{
  int retval;
  WSADATA wsaData;
  if ((retval = WSAStartup(MAKEWORD(1, 1), &wsaData)) != 0)
  {
    std::cerr << "WSAStartup() failed with error: " << Net::strerror(retval)
          << std::endl;
    return false;
  }
  return true;
}

void
unload (void)
{
  WSACleanup();
}

#endif

NET_NAMESPACE_END
