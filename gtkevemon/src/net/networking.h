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

#include <cstring>
#include <openssl/ssl.h>

#include "netdefines.h"

NET_NAMESPACE_BEGIN

/* Platform-independent function to gernerate strings for error codes. */
char const* strerror (int err_code);

/* Initializes the socket subsystem required for networking. */
bool init (void);

/* Unloads the socket subsystem. */
void unload (void);

/* Requests OpenSSL context, initializes on first call. */
SSL_CTX* ssl_context (void);

/* -------------------------- Implementation ---------------------- */

#if !defined(WIN32)

inline char const*
strerror (int err_code)
{
    return std::strerror(err_code);
}

inline bool
init (void)
{
    ssl_context();
    return true;
}

inline void
unload (void)
{
}

#endif

NET_NAMESPACE_END

#endif /* NET_NETWORKING_HEADER */
