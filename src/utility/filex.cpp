// utility/filex.cpp -- Various utility functions that deal with creating, deleting, and manipulating files.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "utility/filex.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>


// Deletes a specified file. Simple enough, but we'll keep this function around in case there's any platform-specific weirdness that needs to be worked in.
void FileX::delete_file(const std::string &filename) { unlink(filename.c_str()); }

// Check if a directory exists.
bool FileX::directory_exists(const std::string &dir)
{
    struct stat info;
    if (stat(dir.c_str(), &info) != 0) return false;
    if (info.st_mode & S_IFDIR) return true;
    return false;
}

// Checks if a file exists.
bool FileX::file_exists(const std::string &file)
{
    struct stat info;
    return (stat(file.c_str(), &info) == 0);
}

// Returns a list of files in a given directory.
std::vector<std::string> FileX::files_in_dir(const std::string &directory, bool recursive)
{
    DIR *dir;
    struct dirent *ent;
    std::vector<std::string> files;
    if (!(dir = opendir(directory.c_str()))) throw std::runtime_error("Could not open directory: " + directory);
    while ((ent = readdir(dir)))
    {
        std::string filename = std::string(ent->d_name);
        if (filename == "." || filename == "..") continue;
        struct stat s;
        if (stat((directory + "/" + filename).c_str(), &s) == 0)
        {
            if (s.st_mode & S_IFDIR)
            {
                if (recursive)
                {
                    std::vector<std::string> result = files_in_dir(directory + "/" + filename, true);
                    for (unsigned int i = 0; i < result.size(); i++)
                        result.at(i) = filename + "/" + result.at(i);
                    files.reserve(files.size() + result.size());
                    files.insert(files.end(), result.begin(), result.end());
                }
            }
            else if (s.st_mode & S_IFREG) files.push_back(filename);
        }
    }
    closedir(dir);
    return files;
}

// Makes a new directory, if it doesn't already exist.
void FileX::make_dir(const std::string &dir)
{
    if (directory_exists(dir)) return;

#ifdef GREAVE_TARGET_WINDOWS
    mkdir(dir.c_str());
#else
    mkdir(dir.c_str(), 0777);
#endif
}