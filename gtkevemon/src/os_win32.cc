#include <iostream>

#include <direct.h>
#include <io.h>
#include <process.h>
#include <shlobj.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "strptime.h"
#include "timegm.h"

#include "os.h"

#define PATH_MAX MAX_PATH

namespace {
  char  home_path[PATH_MAX] = { 0 };
}

/* ---------------------------------------------------------------- */

bool
OS::dir_exists(char const* pathname)
{
  struct _stat statbuf;
  if (::_stat(pathname, &statbuf) < 0)
  {
    return false;
  }

  if (!(statbuf.st_mode & _S_IFDIR))
    return false;

  return true;
}

/* ---------------------------------------------------------------- */

bool
OS::file_exists(char const* pathname)
{
  struct _stat statbuf;
  if (::_stat(pathname, &statbuf) < 0)
  {
    return false;
  }

  if (!(statbuf.st_mode & _S_IFREG))
    return false;

  return true;
}

/* ---------------------------------------------------------------- */

char*
OS::get_default_home_path(void)
{
  if (*home_path != 0)
    return home_path;

  if (!SUCCEEDED(::SHGetFolderPath(0, CSIDL_APPDATA, 0, 0, home_path)))
  {
    std::cout << "Warning: Couldn't determine home directry!" << std::endl;
    return 0;
  }

  return home_path;
}

/* ---------------------------------------------------------------- */

char*
OS::getcwd(char* buf, size_t size)
{
  return ::_getcwd(buf, size);
}

/* ---------------------------------------------------------------- */

bool
OS::mkdir(char const* pathname/*, mode_t mode*/)
{
  if (::_mkdir(pathname) < 0)
    return false;

  return true;
}

/* ---------------------------------------------------------------- */

bool
OS::unlink(char const* pathname)
{
  if (::_unlink(pathname) < 0)
    return false;

  return true;
}

/* ---------------------------------------------------------------- */

char*
OS::strptime(const char *buf, const char *fmt, struct tm *tm)
{
  return ::strptime(buf, fmt, tm);
}

/* ---------------------------------------------------------------- */

time_t
OS::timegm(struct tm *t)
{
  return ::timegm(t);
}

/* ---------------------------------------------------------------- */

int
OS::execv(char const* path, char* const argv[])
{
	return ::_execv(path, argv);
}

