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
 * This class defines utility for converting VM 
 * objects to core types: integer/double/string.
 */

#ifndef OBJECT_CONVERTER_H
#define OBJECT_CONVERTER_H

#include "VirtualObject.h"
#include "../string.h"

// Interface for converting VirtualObjects to Integer
struct IntCastObject {
	virtual long toInt();
};

// Interface for converting VirtualObjects to Double
struct DoubleCastObject {
	virtual double toDouble();
};

// Interface for converting VirtualObjects to String
struct StringCastObject {
	virtual string toString();
};

long objectIntValue(VirtualObject*);

double objectDoubleValue(VirtualObject*);

string objectStringValue(VirtualObject*);

#endif