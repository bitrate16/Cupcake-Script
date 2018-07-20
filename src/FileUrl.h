/*
	Copcake script interpreter.
    Copyright C 2018  bitrate16 bitrate16@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    at your option any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
/*
 * Utility that helps working with file paths.
 * Allows accessing files by direct path, 
 * getting absolute path from relative, 
 * combine two paths, 
 * pat the cat, e.t.c.
 */


#ifndef FILEURL_H
#define FILEURL_H

#include "string.h"
#include <cstdio>
#include <stdio.h>

struct FileUrl {
	// Default allocator: malloc
	char *path;
	
	FileUrl(const char *path);
	FileUrl(string *path);
	FileUrl(string &path);
	
	FileUrl(FileUrl *f, const char *path);
	FileUrl(FileUrl *f, string *path);
	FileUrl(FileUrl *f, string &path);
	
	// Concate two paths
	FileUrl(FileUrl *f, FileUrl *p);
	
	~FileUrl();
	
	// Performs attempt to open file in given mode
	FILE *open(const char *mode);
	// Checks if file exists on disk
	bool exists();
	// Returns true if file is regular file
	bool isFile();
	
	// Returns directory of current file or itself, if it is directory.
	// Null if no parent directory.
	FileUrl *getDirectory();
	// Returns real path of this file.
	FileUrl *getRealPath();
	
	// Returns real path of the file by it's name
	static FileUrl *getRealPath(const char *path);
	// Returns directory wrehe program been executed
	static FileUrl *getExecutableDirectory();
};

#endif