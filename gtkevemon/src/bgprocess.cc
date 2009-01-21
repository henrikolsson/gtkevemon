#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>

#include "exception.h"
#include "helpers.h"
#include "bgprocess.h"

BGProcess::BGProcess (std::vector<std::string>& cmd, std::string const& chdir)
{
  /*
   * We need to store the arguments in the object because
   * the creator of this object might delete them soon.
   * The call to create() will return instantly and run() is
   * executed in the new thread.
   */
  this->cmd = cmd;
  this->chdir = chdir;

  /*
   * Create a thread there the new process will be created in.
   * waitpid() is then waiting for the termination of the process.
   */
  this->pt_create();
}

/* ---------------------------------------------------------------- */

void*
BGProcess::run (void)
{
  /* Create the new progress. */
  pid_t pid = ::fork();
  if (pid < 0)
  {
    /* This should probably never happen. */
    delete this;
    throw Exception("Cannot create process: "
        + std::string(::strerror(errno)));
  }

  /* This is the new child process. */
  if (pid == 0)
  {
    /* Set cwd if requested. */
    if (this->chdir.size() > 0)
      if (::chdir(this->chdir.c_str()) < 0)
         std::cout << "Cannot set PWD: " << ::strerror(errno) << std::endl;

    /* Let the utils build an arguemnt array for the new process. */
    char** args = Helpers::create_argv(this->cmd);

    /* Try to execute the command. */
    ::execvp(args[0], args);

    /* Give up. */
    std::cout << "Cannot execute program: " << ::strerror(errno) << std::endl;
    ::exit(1);
  }

  std::cout << "PID " << pid << " created." << std::endl;

  /* Release directory. */
  ::chdir("/");

  int status;
  ::waitpid(pid, &status, 0);

  std::cout << "PID " << pid << " terminated. Status: " << status << std::endl;

  /* Destory the thread object and exit thread. */
  delete this;
  return 0;
}
