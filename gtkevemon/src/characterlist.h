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

#ifndef CHARACTER_LIST_HEADER
#define CHARACTER_LIST_HEADER

#include <vector>
#include <string>
#include <sigc++/signal.h>

#include "ref_ptr.h"
#include "character.h"

class CharacterList;
typedef ref_ptr<CharacterList> CharacterListPtr;

class CharacterList
{
  public:
    typedef sigc::signal<void, std::string> SignalCharacterRemoved;
    typedef sigc::signal<void, CharacterPtr> SignalCharacterAdded;
    typedef std::vector<CharacterPtr> CharListVector;

  private:
    static CharacterListPtr instance;

  private:
    SignalCharacterRemoved sig_char_removed;
    SignalCharacterAdded sig_char_added;

  protected:
    CharacterList (void);

    void init_from_config (void);
    bool add_character_intern (EveApiAuth const& auth);
    bool remove_character_intern (std::string const& char_id);

  public:
    CharListVector chars;

  public:
    static CharacterListPtr request (void);

    void add_character (EveApiAuth const& auth);
    void remove_character (std::string const& char_id);

    SignalCharacterRemoved& signal_char_removed (void);
    SignalCharacterAdded& signal_char_added (void);
};

/* ---------------------------------------------------------------- */

inline
CharacterList::CharacterList (void)
{
}

inline CharacterList::SignalCharacterRemoved&
CharacterList::signal_char_removed (void)
{
  return this->sig_char_removed;
}

inline CharacterList::SignalCharacterAdded&
CharacterList::signal_char_added (void)
{
  return this->sig_char_added;
}

#endif /* CHARACTER_LIST_HEADER */
