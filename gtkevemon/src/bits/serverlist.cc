#include <iostream>

#include "util/exception.h"
#include "util/thread.h"

#include "serverlist.h"
#include "config.h"

/* Static members. */
std::vector<ServerPtr> ServerList::list;

/* ---------------------------------------------------------------- */

class ServerChecker : public Thread
{
  private:
    std::vector<ServerPtr> m_list;
  protected:
    void* run (void);
  public:
    ServerChecker(std::vector<ServerPtr> list) : m_list(list) {}
};

/* ---------------------------------------------------------------- */

void*
ServerChecker::run (void)
{
  
  for (unsigned int i = 0; i < m_list.size(); ++i)
  {
    ServerPtr server = m_list[i];
    try
    {
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
  ServerList::list.clear();
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

  ServerChecker* checker = new ServerChecker(ServerList::list);
  checker->pt_create();
}
