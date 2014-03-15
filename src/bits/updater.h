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

#ifndef UPDATER_HEADER
#define UPDATER_HEADER

#include <vector>
#include <string>

#include "util/thread.h"
#include "net/http.h"

/*
 * Information about the data files updated by the GtkEveMon updater.
 * This is usually something like "SkillTree.xml" as file name,
 * the API host is "api.eveonline.com", the server path in case of SkillTree
 * is "eve/SkillTree.xml.aspx". The local path is generated from the
 * directory where the GtkEveMon config resides plus the file name.
 */
struct UpdaterDataFile
{
    std::string file_name;
    std::string server_host;
    std::string server_path;
    std::string local_path;
};

/* ---------------------------------------------------------------- */

/*
 * Base class that manages common aspects of downloading the data files
 * from the EVE servers. As such it provides a list of files to be updated.
 */
class UpdaterBase
{
public:
    UpdaterBase (void);

protected:
    std::vector<UpdaterDataFile> files;
};

/* ---------------------------------------------------------------- */

/*
 * Updater that checks if update interval is expired and downloads
 * new data files from the EVE API. If the data files have changed,
 * the corresponding signal is fired. Otherwise, the no change signal
 * is fired.
 */
class Updater : public UpdaterBase, public Thread
{
private:
    Glib::Dispatcher sig_dispatch_files_changed;
    Glib::Dispatcher sig_dispatch_files_unchanged;

protected:
    void* run (void);
    bool background_check_intern (void);

public:
    Updater (void) {}
    virtual ~Updater (void);

    /*
     * Checks if a download is necessary (once in a while depending on the
     * update interval). Then it checks if the downloaded files differ
     * from the existing one. Appropriate signals are fired when done.
     */
    void background_check (void);
    void background_check_async (void);

    /*
     * Checks if the data files are locally available. If the files are not
     * available, the update GUI is automatically raised. This is usually
     * called from the GtkEveMon main routine before anything else that
     * relies on the data files.
     */
    static void check_data_files (void);

    /*
     * Marks the data files as updated right now. This is called
     * from both the background updater and the GUI updater.
     */
    static void set_last_update_now (void);

    /*
     * Checks whether the given filename has the same contents as
     * the downloaded HTTP data.
     */
    static bool is_same_file (std::string const& filename, HttpDataPtr data);

    Glib::Dispatcher& signal_files_changed (void);
    Glib::Dispatcher& signal_files_unchanged (void);
};

/* ---------------------------------------------------------------- */

inline Glib::Dispatcher&
Updater::signal_files_unchanged (void)
{
    return this->sig_dispatch_files_unchanged;
}

inline Glib::Dispatcher&
Updater::signal_files_changed (void)
{
    return this->sig_dispatch_files_changed;
}

#endif /* UPDATER_HEADER */
