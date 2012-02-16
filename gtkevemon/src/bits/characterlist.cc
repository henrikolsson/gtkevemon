#include <iostream>

#include "util/helpers.h"

#include "config.h"
#include "characterlist.h"

CharacterListPtr CharacterList::instance;

/* ---------------------------------------------------------------- */

CharacterListPtr
CharacterList::request (void)
{
  if (CharacterList::instance.get() == 0)
  {
    CharacterList::instance = CharacterListPtr(new CharacterList);
    CharacterList::instance->init_from_config();
  }

  return CharacterList::instance;
}

/* ---------------------------------------------------------------- */

void
CharacterList::init_from_config (void)
{
  ConfSectionPtr char_sect = Config::conf.get_section("characters");
  for (conf_values_t::iterator iter = char_sect->values_begin();
      iter != char_sect->values_end(); iter++)
  {
    std::string user_id = iter->first;
    std::string api_key;
    bool is_apiv1 = true;
    try
    {
      ConfSectionPtr key_sect = Config::conf.get_section("accounts." + user_id);
      api_key = key_sect->get_value("apikey")->get_string();
      try { is_apiv1 = key_sect->get_value("apiver")->get_int() == 1; }
      catch (...) { }
    }
    catch (...)
    {
      std::cout << "Error getting account information for "
          << user_id << std::endl;
      continue;
    }

    EveApiAuth auth(user_id, api_key);
    auth.is_apiv1 = is_apiv1;

    std::string char_ids = **iter->second;
    StringVector chars = Helpers::split_string(char_ids, ',');

    for (std::size_t i = 0; i < chars.size(); ++i)
    {
      if (chars[i].empty())
        continue;
      auth.char_id = chars[i];
      this->add_character_intern(auth);
    }
  }
}

/* ---------------------------------------------------------------- */

bool
CharacterList::add_character_intern (EveApiAuth const& auth)
{
  /* Check if character already exists. */
  bool found = false;
  for (std::size_t i = 0; i < this->chars.size(); ++i)
  {
    if (auth.char_id == this->chars[i]->get_char_id())
    {
      found = true;
      break;
    }
  }

  if (found)
    return false;

  /* Insert the character to the list. */
  CharacterPtr c = Character::create(auth);
  this->chars.push_back(c);
  this->sig_char_added.emit(c);

  return true;
}

/* ---------------------------------------------------------------- */

void
CharacterList::add_character (EveApiAuth const& auth)
{
  bool inserted = this->add_character_intern(auth);
  if (!inserted)
    return;

  /* Add the character to the configuration. */
  ConfSectionPtr char_sect = Config::conf.get_section("characters");
  try
  {
    ConfValuePtr value = char_sect->get_value(auth.user_id);
    value->set(**value + "," + auth.char_id);
  }
  catch (...)
  {
    char_sect->add(auth.user_id, ConfValue::create(auth.char_id));
  }

  /* Save the configuration. */
  Config::save_to_file();
}

/* ---------------------------------------------------------------- */

bool
CharacterList::remove_character_intern (std::string const& char_id)
{
  bool removed = false;
  for (CharListVector::iterator iter = this->chars.begin();
      iter != this->chars.end(); iter++)
  {
    if ((*iter)->get_char_id() == char_id)
    {
      this->chars.erase(iter);
      this->sig_char_removed.emit(char_id);
      removed = true;
      break;
    }
  }

  return removed;
}

/* ---------------------------------------------------------------- */

void
CharacterList::remove_character (std::string const& char_id)
{
  bool removed = this->remove_character_intern(char_id);
  if (!removed)
    return;

  /* Remove the character from the configuration. */
  ConfSectionPtr char_sect = Config::conf.get_section("characters");
  char_sect->clear_values();

  for (std::size_t i = 0; i < this->chars.size(); ++i)
  {
    std::string user_id = this->chars[i]->get_user_id();
    std::string char_id = this->chars[i]->get_char_id();

    try
    {
      ConfValuePtr value = char_sect->get_value(user_id);
      value->set(**value + "," + char_id);
    }
    catch (...)
    {
      char_sect->add(user_id, ConfValue::create(char_id));
    }
  }

  /* Save the configuration. */
  Config::save_to_file();
}
