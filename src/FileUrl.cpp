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


#include "FileUrl.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "cstdlib"
#include "ptr_wrapper.h"

#ifdef __WIN32__

#include <direct.h>
#define SeparatorChar '\\'
#define GetCurrentDir _getcwd

char *_getRealPath(const char *path) {
	char *pszFileName = NULL;

	int size = GetFullPathName(path, 0, NULL, &pszFileName);
	char *np = malloc((size + 1) * sizeof(char));
	GetFullPathName(path, size + 1, np, &pszFileName);
	
	return np;
};

#else
	
#include <unistd.h>
#define SeparatorChar '/'
#define GetCurrentDir getcwd

char *_getRealPath(const char *path) {
	/*char actualpath[PATH_MAX + 1];

	realpath(path, actualpath);

	int len = 0;
	while (actualpath[len]) ++len;
	
	char *np = malloc((len + 1) * sizeof(char));
	
	len = 0;
	while (actualpath[len]) 
		np[i] = actualpath[i++];
	
	return np;*/
	return realpath(path, NULL);
};

#endif

void normalize_separator(FileUrl *f) {
	int len = 0;
	while (f->path[len]) {
		if (f->path[len] == '\\')
			f->path[len] = SeparatorChar;
		if (f->path[len] == '/')
			f->path[len] = SeparatorChar;
		
		++len;
	}
};

FileUrl::FileUrl(const char *path) {
	string temp = string(path);
	this->path  = temp.toCString();
	normalize_separator(this);
};

FileUrl::FileUrl(string *path) {
	string temp = string(path);
	this->path  = temp.toCString();
	normalize_separator(this);
};

FileUrl::FileUrl(string &path) {
	string temp = string(path);
	this->path  = temp.toCString();
	normalize_separator(this);
};

FileUrl::FileUrl(FileUrl *f, const char *path) {
	string temp = string(f->path);
	temp += SeparatorChar;
	temp += path;
	this->path  = temp.toCString();
	normalize_separator(this);
};

FileUrl::FileUrl(FileUrl *f, string *path) {
	string temp = string(f->path);
	temp += SeparatorChar;
	temp += path;
	this->path  = temp.toCString();
	normalize_separator(this);
};

FileUrl::FileUrl(FileUrl *f, string &path) {
	string temp = string(f->path);
	temp += SeparatorChar;
	temp += path;
	this->path  = temp.toCString();
	normalize_separator(this);
};

FileUrl::FileUrl(FileUrl *f, FileUrl *p) {
	string temp = string(f->path);
	temp += SeparatorChar;
	temp += p->path;
	this->path  = temp.toCString();
	normalize_separator(this);
};

FileUrl::~FileUrl() {
	free(path);
};


FILE *FileUrl::open(const char *mode) {
	return fopen(path, mode);
};

bool FileUrl::exists() {
	struct stat buffer;   
	return (stat (path, &buffer) == 0);
};

bool FileUrl::isFile() {
	struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
};

FileUrl *FileUrl::getDirectory() {
	if (!isFile())
		return this;
	
	int offset = 0;
	int len = 0;
	while (path[len]) ++len;
	
	while (offset < len)
		if (path[len - offset - 1] == '/' || path[len - offset - 1] == '\\')
			break;
		else
			++offset;
	
	if (offset == len)
		return NULL;
	
	ptr_wrapper wrapper(malloc((len - offset) * sizeof(char)), PTR_ALLOC);
	char *np = (char*) wrapper.ptr;
	
	for (int i = 0; i < len - offset - 1; ++i)
		np[i] = path[i];
	
	np[len - offset - 1] = '\0';
	
	return new FileUrl(np);
};

FileUrl *FileUrl::getRealPath() {
	ptr_wrapper wrapper(_getRealPath(path), PTR_ALLOC);
	return new FileUrl((char*) wrapper.ptr);
};


FileUrl *FileUrl::getRealPath(const char *path) {
	ptr_wrapper wrapper(_getRealPath(path), PTR_ALLOC);
	return new FileUrl((char*) wrapper.ptr);
};

FileUrl *FileUrl::getExecutableDirectory() {
	char buff[FILENAME_MAX];
	GetCurrentDir(buff, FILENAME_MAX);
	return new FileUrl(buff);
};

