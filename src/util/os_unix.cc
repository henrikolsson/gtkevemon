#include <iostream>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <cstring>
#include <ctime>

#include "os.h"

namespace {
  char  home_path[PATH_MAX] = { 0 };
}

/* ---------------------------------------------------------------- */

bool
OS::dir_exists(char const* pathname)
{
  struct stat statbuf;
  if (::stat(pathname, &statbuf) < 0)
  {
    return false;
  }

  if (!S_ISDIR(statbuf.st_mode))
    return false;

  return true;
}

/* ---------------------------------------------------------------- */

bool
OS::file_exists(char const* pathname)
{
  struct stat statbuf;
  if (::stat(pathname, &statbuf) < 0)
  {
    return false;
  }

  if (!S_ISREG(statbuf.st_mode))
    return false;

  return true;
}

/* ---------------------------------------------------------------- */

char*
OS::get_default_home_path(void)
{
  if (*home_path != 0)
    return home_path;

  uid_t user_id = ::geteuid();
  struct passwd* user_info = ::getpwuid(user_id);

  if (user_info == 0 || user_info->pw_dir == 0)
  {
    std::cout << "Warning: Couldn't determine home directory!" << std::endl;
    return 0;
  }

  std::strncpy(home_path, user_info->pw_dir, PATH_MAX);

  return home_path;
}

/* ---------------------------------------------------------------- */

char*
OS::getcwd(char* buf, size_t size)
{
  return ::getcwd(buf, size);
}

/* ---------------------------------------------------------------- */

bool
OS::mkdir(char const* pathname/*, mode_t mode*/)
{
  if (::mkdir(pathname, S_IRWXU) < 0)
    return false;

  return true;
}

/* ---------------------------------------------------------------- */

bool
OS::unlink(char const* pathname)
{
  if (::unlink(pathname) < 0)
    return false;

  return true;
}

/* ---------------------------------------------------------------- */

std::size_t
OS::file_size(char const* pathname)
{
  struct stat filestats;
  if (::stat(pathname, &filestats) < 0)
    return 0;
  return static_cast<std::size_t>(filestats.st_size);
}

/* ---------------------------------------------------------------- */

char*
OS::strptime(const char *buf, const char *fmt, struct tm *tm)
{
  return ::strptime(buf, fmt, tm);
}

/* ---------------------------------------------------------------- */

#if defined(__SunOS)
# include "timegm.h"
#endif

time_t
OS::timegm(struct tm *tm)
{
  return ::timegm(tm);
}

/* ---------------------------------------------------------------- */

int
OS::execv (char const* path, char* const argv[])
{
    return ::execv(path, argv);
}
