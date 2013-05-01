#ifndef WIN32
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/socket.h> // netdb.h, ::get{peer,sock}name
# include <sys/types.h> // netdb.h, ::select
# include <netdb.h> // getaddrinfo
#else
# include <winsock2.h>
# include <ws2tcpip.h>
typedef unsigned short uint16_t;
#endif
#include <fcntl.h> // ::fcntl
#include <cerrno>
#include <cstring>
#include <string>
#include <sstream>

#include "util/exception.h"
#include "networking.h"
#include "nettcpsocket.h"

NET_NAMESPACE_BEGIN

TCPSocket::TCPSocket (void)
{
  this->timeout = 0;
}

/* ---------------------------------------------------------------- */

TCPSocket::TCPSocket (const std::string& host, int port)
{
  this->timeout = 0;
  this->connect(host, port);
}

/* ---------------------------------------------------------------- */

TCPSocket::TCPSocket (int sock_fd)
{
  this->sock = sock_fd;
  this->timeout = 0;

  socklen_t remote_len = sizeof(struct sockaddr_in);
  socklen_t local_len = sizeof(struct sockaddr_in);

  if (::getpeername(sock_fd, (struct sockaddr*)&this->remote, &remote_len) < 0)
    throw Exception(Net::strerror(errno));

  if (::getsockname(sock_fd, (struct sockaddr*)&this->local, &local_len) < 0)
    throw Exception(Net::strerror(errno));
}

/* ---------------------------------------------------------------- */

TCPSocket::~TCPSocket (void)
{
}

/* ---------------------------------------------------------------- */

void
TCPSocket::set_connect_timeout (std::size_t timeout_ms)
{
  this->timeout = timeout_ms;
}

/* ---------------------------------------------------------------- */

void
TCPSocket::connect (std::string const& host, int port)
{
  struct addrinfo *res;
  struct addrinfo hints;
  ::memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_socktype = SOCK_STREAM;
  int retval;
  if ((retval = ::getaddrinfo(host.c_str(), 0, &hints, &res)) != 0)
  {
    throw Exception(std::string("getaddrinfo() failed: ")
        + ::gai_strerror(retval));
  }

  struct sockaddr_in tmp = *(struct sockaddr_in*)res->ai_addr;
  tmp.sin_port = htons((uint16_t)port);
  ::freeaddrinfo(res);

  this->connect(tmp.sin_addr.s_addr, port);
}

/* ---------------------------------------------------------------- */

void
TCPSocket::connect (in_addr_t host, int port)
{
  if (this->is_connected())
    this->close();

  this->sock = ::socket(PF_INET, SOCK_STREAM, 0);
  if (this->sock < 0)
    throw Exception(Net::strerror(errno));

  this->remote.sin_family = AF_INET;
  this->remote.sin_port = htons((uint16_t)port);
  this->remote.sin_addr.s_addr = host;

  //if (::inet_aton(host.c_str(), &this->remote.sin_addr) == 0)
  //  throw Exception("Invalid host format");

  if (this->timeout != 0)
  {
    /* Set non-blocking flag for the socket. */
#ifndef WIN32
    int flags = ::fcntl(this->sock, F_GETFL, 0);
    if (flags < 0)
    {
      this->close();
      throw Exception(Net::strerror(errno));
    }

    flags |= O_NONBLOCK;
    if (::fcntl(this->sock, F_SETFL, flags) < 0)
    {
      this->close();
      throw Exception(Net::strerror(errno));
    }
#else
    u_long flags = 1;
    if (::ioctlsocket(sock, FIONBIO, &flags) == SOCKET_ERROR)
    {
      this->close();
      throw Exception(Net::strerror(::WSAGetLastError()));
    }
#endif
  }

  /* Do the actual connect call. */
  int connect_ret = ::connect(this->sock,
      (struct sockaddr*)&this->remote, sizeof(struct sockaddr_in));

#ifndef WIN32
  if (connect_ret < 0 && errno == EINPROGRESS)
#else
  if (connect_ret == SOCKET_ERROR
    && ::WSAGetLastError() == WSAEWOULDBLOCK)
#endif
  {
    /* Non-blocking flag is set and connection is in progress. */
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(this->sock, &wfds);

    struct timeval timeout;
    timeout.tv_sec = this->timeout / 1000;
    timeout.tv_usec = (this->timeout % 1000) * 1000;

    connect_ret = ::select(sock + 1, 0, &wfds, 0, &timeout);

#ifndef WIN32
    if (connect_ret <= 0)
    {
      this->close();
      if (errno == EINPROGRESS)
        throw Exception("Connection timed out");
      else
        throw Exception(Net::strerror(errno));
    }
#else
    if (connect_ret == SOCKET_ERROR)
    {
      this->close();
      if (::WSAGetLastError() == WSAEINPROGRESS)
        throw Exception("Connection timed out");
      else
        throw Exception(Net::strerror(::WSAGetLastError()));
    }
#endif

  }
  else if (connect_ret < 0)
  {
    /* This is clearly an error. */
    this->close();
#ifndef WIN32
    throw Exception(Net::strerror(errno));
#else
    throw Exception(Net::strerror(::WSAGetLastError()));
#endif
  }

  if (this->timeout != 0)
  {
#ifndef WIN32
    /* Remove non-blocking flag from the socket. */
    int flags = ::fcntl(this->sock, F_GETFL, 0);
    if (flags < 0)
    {
      this->close();
      throw Exception(Net::strerror(errno));
    }
    flags &= ~O_NONBLOCK;
    if (::fcntl(this->sock, F_SETFL, flags) < 0)
    {
      this->close();
      throw Exception(Net::strerror(errno));
    }
#else
    u_long sockopt = 0;
    if (::ioctlsocket(sock, FIONBIO, &sockopt) == SOCKET_ERROR)
    {
      this->close();
      throw Exception(Net::strerror(::WSAGetLastError()));
    }
#endif
  }

  /* Get information about the local socket. We're not
   * terribly much interested in errors of this call. */
  socklen_t local_len = sizeof(struct sockaddr_in);
  ::getsockname(this->sock, (struct sockaddr*)&this->local, &local_len);
}

/* ---------------------------------------------------------------- */

int
TCPSocket::get_local_port (void) const
{
  return ntohs(this->local.sin_port);
}

/* ---------------------------------------------------------------- */

int
TCPSocket::get_remote_port (void) const
{
  return ntohs(this->remote.sin_port);
}

/* ---------------------------------------------------------------- */

std::string
TCPSocket::get_local_address (void) const
{
  return ::inet_ntoa(this->local.sin_addr);
}

/* ---------------------------------------------------------------- */

std::string
TCPSocket::get_remote_address (void) const
{
  return ::inet_ntoa(this->remote.sin_addr);
}

/* ---------------------------------------------------------------- */

std::string
TCPSocket::get_full_local (void) const
{
  std::stringstream ss;
  ss << this->get_local_address() << ":" << this->get_local_port();
  return ss.str();
}

/* ---------------------------------------------------------------- */

std::string
TCPSocket::get_full_remote (void) const
{
  std::stringstream ss;
  ss << this->get_remote_address() << ":" << this->get_remote_port();
  return ss.str();
}

NET_NAMESPACE_END
