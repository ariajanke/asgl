/****************************************************************************

    MIT License

    Copyright (c) 2020 Aria Janke

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*****************************************************************************/

#include <common/CurrentWorkingDirectory.hpp>
#include <common/Util.hpp>
#include <common/StringUtil.hpp>

#if defined(MACRO_PLATFORM_LINUX)
#   include <linux/limits.h>
#   include <unistd.h>
#elif defined(MACRO_PLATFORM_WINDOWS)
#   include <windows.h>
#endif

#include <mutex>
#include <stdexcept>
#include <cassert>

#if !defined(MACRO_PLATFORM_LINUX) && !defined (MACRO_PLATFORM_WINDOWS)
#   error("Platform Macros are either unspecified, or the platform is not supported.")
#endif

namespace {
    using Error = std::runtime_error;
    void trim_to_parent_path(std::string & path) noexcept;
#ifdef MACRO_PLATFORM_WINDOWS
    int find_null_terminator(const std::string & str);
#endif
} // end of <anonymous> namespace

void get_current_working_directory(std::string & rv) {
    rv.clear();
#   if defined(MACRO_PLATFORM_LINUX)
    //
    //                       LINUX IMPLEMENTATION
    //
    rv.resize(PATH_MAX, '\0');
    // getcwd is thread-safe (by POSIX)
    while (!::getcwd(&rv.front(), rv.size())) {
        rv.resize(rv.size()*2, '\0');
    }

    if (rv.find("(unreachable)") != std::string::npos) {
        throw Error("Current working directory is unreachable.");
    }

    rv.erase(rv.begin() + unsigned(rv.find('\0')), rv.end());
#   elif defined(MACRO_PLATFORM_WINDOWS)
    //
    //                       WINDOWS IMPLEMENTATION
    //
    rv.resize(MAX_PATH, '\0');
    GetCurrentDirectoryA((DWORD)rv.size(), &rv.front());
    // there seems to be a bug in msvc's STL string library, I'll have to code
    // my own as a temporary work around
    rv.erase(rv.begin() + find_null_terminator(rv), rv.end());
#   endif
}

bool set_current_working_directory(const char * path) {
    const int SUCCESS = 1;
    const int FAILURE = 0;
    const int UNINIT  = -1;
    int rv = UNINIT;

#   if defined (MACRO_PLATFORM_LINUX)
    //
    //                  LINUX IMPLEMENTATION
    //
    // we cannot be sure that chdir is thread safe, therefore mutex
    static std::mutex cwd_change_mutex;
    std::lock_guard<std::mutex> lock(cwd_change_mutex); (void)lock;
    rv = (::chdir(path) == 0) ? SUCCESS : FAILURE;
#   elif defined (MACRO_PLATFORM_WINDOWS)
    rv = (SetCurrentDirectoryA(path) == TRUE) ? SUCCESS : FAILURE;
#   endif

    assert(rv != UNINIT);
    return rv == SUCCESS;
}

bool is_absolute_file_path(const char * path) {
#   if defined (MACRO_PLATFORM_LINUX)
    return path[0] == '/';
#   elif defined (MACRO_PLATFORM_WINDOWS)
    if (path[0] == '\0') return false;
    return path[1] == ':';
#   endif

}

std::string get_current_working_directory() {
    std::string rv;
    get_current_working_directory(rv);
    return rv;
}

bool set_current_working_directory(const std::string & path)
    { return set_current_working_directory(path.c_str()); }

bool is_absolute_file_path(const std::string & path)
    { return is_absolute_file_path(path.c_str()); }

DirectoryChangerRaii::DirectoryChangerRaii(const std::string & path):
    DirectoryChangerRaii(path.c_str())
{}

DirectoryChangerRaii::DirectoryChangerRaii(const char * path):
    m_old_directory(get_current_working_directory())
{
    std::string filepath = path;
    trim_to_parent_path(filepath);
    // make absolute, if not already
    if (!is_absolute_file_path(filepath))
        filepath = m_old_directory + '/' + filepath;
    if (!set_current_working_directory(filepath)) {
        throw Error("Failed to change working directory to: \"" +
                    filepath + "\".");
    }
}

DirectoryChangerRaii::~DirectoryChangerRaii() {
    if (!set_current_working_directory(m_old_directory)) {
        message_assert("Failed to change directory back (programming error.)", false);
    }
}

namespace {

#ifdef MACRO_PLATFORM_WINDOWS
int find_null_terminator(const std::string & str) {
    const char * c = str.data();
    while (*c) ++c;
    return int(c - str.data());
}
#endif

void trim_to_parent_path(std::string & path) noexcept {
    std::size_t selection;

    {
    std::size_t rslash = path.rfind('\\');
    std::size_t fslash = path.rfind('/' );

    if (rslash == std::string::npos) {
        selection = fslash;
    } else if (fslash == std::string::npos) {
        selection = rslash;
    } else {
        selection = std::max(rslash, fslash);
    }
    }

    if (selection == std::string::npos) {
        path.clear();
    } else {
        path.erase(path.begin() + std::string::difference_type(selection), path.end());
    }
}

} // end of <anonymous> namespace
