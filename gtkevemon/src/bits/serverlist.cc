#include <iostream>

#include "util/exception.h"
#include "util/thread.h"

#include "serverlist.h"
#include "config.h"

/* Static members. */
Semaphore ServerList::list_semaphore;
std::vector<ServerPtr> ServerList::list;

/* ---------------------------------------------------------------- */

class ServerChecker : public Thread
{
  protected:
    void* run (void);
};

/* ---------------------------------------------------------------- */

void*
ServerChecker::run (void)
{
  std::vector<ServerPtr> local = ServerList::get_list();
  
  for (unsigned int i = 0; i < local.size(); ++i)
  {
    try
    {
      ServerPtr server = local[i];
      if (!server->is_refreshing())
        server->refresh();
    }
    catch (Exception& s)
    {
      std::cout << "Error getting server status: " << s << std::endl;
    }
  }

  delete this;
  return 0;
}

/* ================================================================ */

void
ServerList::init_from_config (void)
{
  ConfSectionPtr servers = Config::conf.get_section("servermonitor");
  for (conf_values_t::iterator iter = servers->values_begin();
      iter != servers->values_end(); iter++)
  {
    ServerList::add_server(iter->first.substr(2), **iter->second);
  }

  ConfValuePtr check = Config::conf.get_value("settings.startup_servercheck");
  if (check->get_bool())
    ServerList::refresh();
}

/* ---------------------------------------------------------------- */

void
ServerList::unload (void)
{
  try 
  {
    list_semaphore.wait();
    ServerList::list.clear();
    list_semaphore.post();
  } 
  catch (Exception& e)
  {
    list_semaphore.post();
    throw e;
  }
}

/* ---------------------------------------------------------------- */

void
ServerList::add_server (std::string const& name,
    std::string const& host, uint16_t port)
{
  ServerPtr server = ServerPtr(new Server(name, host, port));
  ServerList::list.push_back(server);
}

/* ---------------------------------------------------------------- */

void
ServerList::refresh (void)
{
  /* Prevents the creation of a thread if not neccessary. */
  if (ServerList::list.size() == 0)
    return;

  //std::cout << "Refreshing servers..." << std::endl;

  ServerChecker* checker = new ServerChecker;
  checker->pt_create();
}

/* ---------------------------------------------------------------- */

std::vector<ServerPtr> 
ServerList::get_list(void)
{
  try 
  {
    list_semaphore.wait();

    std::vector<ServerPtr> copy(list);

    list_semaphore.post();

    // actually it will be copied twice
    return copy;
  }
  catch (Exception & e) 
  {
    list_semaphore.post();
    throw e;
  }
}
