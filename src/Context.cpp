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



#include "Context.h"
#include <cstdlib>
#include "FileUrl.h"

Context::Context() {
	scope            = NULL;
	executer         = NULL;
	script_dir_path  = NULL;
	script_file_path = NULL;
};

Context::~Context() {
	delete script_dir_path;
	delete script_file_path;
};