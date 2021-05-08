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

#pragma once

#include <string>

/** @note Implementation is platform specific
 *  @return Process's current working directory.
 */
std::string get_current_working_directory();

/** @note Implementation is platform specific
 *  @param path Set to process's current working directory.
 */
void get_current_working_directory(std::string & path);

/** Attempts to set the process's currect working directory.
 *  @note This function is thread safe (using one mutex for this global
 *        resource)
 *  @param path relative/absolute new current working directory
 *  @return Returns true if the currect working directory was successfully
 *          changed.
 */
bool set_current_working_directory(const std::string & path);

/** @copydoc bool set_current_working_directory(const std::string&) */
bool set_current_working_directory(const char * path);

/** Makes a quick platform specific check on whether a path is given as an
 *  absolute path.
 *  @note   Does not check if the path exist or not!
 *  @param  path Path to test.
 *  @return Returns true if absolute path, false otherwise.
 */
bool is_absolute_file_path(const std::string & path);

/** @copydoc bool file_path_is_absolute(const std::string&) */
bool is_absolute_file_path(const char * path);

/** Automatic directory changing class. This RAII object will change the
 *  process' working directory until this object is destoryed.
 */
class DirectoryChangerRaii {
public:
    DirectoryChangerRaii(const std::string & path);
    DirectoryChangerRaii(const char * path);
    ~DirectoryChangerRaii();

private:
    std::string m_old_directory;
};
