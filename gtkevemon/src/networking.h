/*
 * This file is part of GtkEveMon.
 *
 * GtkEveMon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NET_NETWORKING_HEADER
#define NET_NETWORKING_HEADER

#include <iostream>
#include <cstring>
#include "netdefines.h"

NET_NAMESPACE_BEGIN

inline char const*
strerror (int err_code)
{
#if defined(WIN32)
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
  return &err_msg;
#else
  return std::strerror(err_code);
#endif
}

/* ---------------------------------------------------------------- */

inline bool
init (void)
{
#ifdef WIN32
  int retval;
  WSADATA wsaData;
  if ((retval = WSAStartup(MAKEWORD(1, 1), &wsaData)) != 0)
  {
    std::cerr << "WSAStartup() failed with error: " << Net::strerror(retval)
	      << std::endl;
    return false;
  }
#endif
  return true;
}

/* ---------------------------------------------------------------- */

inline void
unload (void)
{
#ifdef WIN32
  WSACleanup();
#endif
}

NET_NAMESPACE_END

#endif /* NET_NETWORKING_HEADER */
