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

#ifndef NET_SSLTCPSOCKET_HEADER
#define NET_SSLTCPSOCKET_HEADER

#include <openssl/ssl.h>

#include "netdefines.h"
#include "nettcpsocket.h"

NET_NAMESPACE_BEGIN

class SSLTCPSocket : public TCPSocket
{
  private:
    void check_cert (void);

  private:
    SSL* ssl;
    BIO* sbio;
    std::string hostname;

  protected:
    bool match_hostname (std::string const& ca, std::string const& host);

  public:
    SSLTCPSocket (void);
    SSLTCPSocket (std::string const& host, int port);

    virtual ~SSLTCPSocket (void);

    void check_hostname (std::string const& host);

    virtual void ssl_handshake (void);
    virtual void close (void);

    /* Read chunks of data via SSL. */
    virtual std::size_t read (void* buffer, std::size_t size,
        std::size_t offset = 0);

    /* Alias for 'write', SSL uses all-or-nothing operations. */
    virtual std::size_t full_read (void* buffer, std::size_t size,
        std::size_t offset = 0);

    /* Writes chunks of data via SSL. */
    virtual std::size_t write (void const* buffer,
        std::size_t size, std::size_t offset = 0);

    /* Alias for 'write', SSL uses all-or-nothing operations. */
    virtual std::size_t full_write (void const* buffer,
        std::size_t size, std::size_t offset = 0);
};

/* ---------------------------------------------------------------- */

inline void
SSLTCPSocket::check_hostname (std::string const& host)
{
    this->hostname = host;
}

inline std::size_t
SSLTCPSocket::full_read (void* buffer, std::size_t size,
    std::size_t offset)
{
    return this->read(buffer, size, offset);
}

inline std::size_t
SSLTCPSocket::full_write (void const* buffer,
        std::size_t size, std::size_t offset)
{
    return this->write(buffer, size, offset);
}

NET_NAMESPACE_END

#endif /* NET_SSLTCPSOCKET_HEADER */
