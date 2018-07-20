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
 * Custom realisation of string type for C++. 
 * Used by whole interpreter. 
 * Overrides standard operatins over strings, characters.
 * Supports parsing string into integers/doubles
 * Supports cinverting integers/doubles into string values.
 * Uses wchar_t for storing unicode characters.
 */
 

#ifndef STRING_H
#define STRING_H	

#include <wchar.h>

struct string {
	
	int      length;
	int        size;
	wchar_t *buffer;
	
	string();
	
	string(int size);
	
	string(const char *data);
	
	string(const wchar_t *data);
	
	string(string *str);
	
	string(const string &str);
	
	~string();
	
	// Accessor --> read
	wchar_t operator[](int i);
	
	// Mutator --> change
	const wchar_t &operator[](int i) const;
	
	bool operator==(const string &s) const;
	
	bool operator==(const char *s) const;
	
	bool operator==(const wchar_t *s) const;
	
	bool operator!=(const string &s) const;
	
	bool operator!=(const char *s) const;
	
	bool operator!=(const wchar_t *s) const;
	
	string &operator=(const char *s);
	
	string &operator=(const wchar_t *s);
	
	string &operator=(const string &s);
	
	string &operator+=(const char c);
	
	string &operator+=(const wchar_t c);
	
	string &operator+=(const wint_t c);
	
	string &operator+=(const int i);
	
	string &operator+=(const string &s);
	
	string &operator+=(const char *s);
	
	string &operator+=(const wchar_t *s);
	
	string operator+(const char c);
	
	string operator+(const wchar_t c);
	
	string operator+(const int i);
	
	string operator+(const string &s);
	
	string operator+(const char *s);
	
	string operator+(const wchar_t *s);
	
	void reverse();
	
	int compare(string *s);
	
	int compare(string &s);
	
	int compare(const char *s);
	
	int compare(const wchar_t *s);
	
	static wchar_t toUpper(wchar_t c);
	
	static wchar_t toLower(wchar_t c);
	
	string toUpperCase();
	
	string toLowerCase();
	
	static string fromCString(const char *s);
	
	static string fromString(const wchar_t *s);
	
	static string toString(int i);
	
	static string toString(long l);
	
	static string toString(long long l, int b);
	
	static string toString(double d);
	
	long long toInt(int base, int def);

	double toDouble(double def);
	
	void print(void);
	
	wchar_t *toCharSequence();
	
	char *toCString();
	
	string *copy();
	
	void dispose(void);
};

#endif
