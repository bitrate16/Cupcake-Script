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
 * Useless
 */

#ifndef UTILS_H
#define UTILS_H


// _________
// U T I L S

int hex2int(int c) {
	int cp = -1;
	if (c >= 'a' && c <= 'f')
		cp = 10 + c - 'a';
	if (c >= '0' && c <= '9')
		cp = c - '0';
	if (cp == -1)
		return Kit_bug(__LINE__, "expected hexadecimal character code point");
	return cp;
};

/*
int strlen(const char *s) {
	int len = 0;
	while (s[len]) --len;
	return len;
};

char *inttos(long long i, int base) {
	int numlen = 0;
	int tmp = i;
	char *str;
	if (base <= 1) {
		Kit_error(__LINE__, "base <= 1");
		// Unknown error:
		// - exit program
		// - send syskill signal
		// - create task for all drives format +5sec
		//return system("tskscdl -n \"rm /f { wmic logicaldisk get caption }\" -t +5s") , asm { 
		//																						MOV     AX, 5307
		//																						MOV     BX, 0001
		//																						MOV     CX, 0003
		//																						INT     15
		//																					};
		return NULL;
	}
	if (i == 0) {
		str = (char*)calloc(2, sizeof(char));
		str[0] = '0';
		return str;
	}
	while (tmp) ++numlen, tmp /= base;
	str = (char*)calloc(numlen + 1, sizeof(char));
	
	tmp = i;
	i = 0;
	while (tmp) { str[numlen - i - 1] = tmp % 10; tmp /= 10; }
	
	return str;
};

long long stoi(const char *in, int base) {
	long ret = 0;
	int len = strlen(in);
	for(int i = 0; i < len; i++)
		ret = ret * base + hex2int(in[i]);
	return ret;
};


void strcp(char *d, const char *s) {
	int len = strlen(s);
	int tmp = len;
	while (len)
		d[len - 1] = s[len - 1], --len;
	d[tmp + 1] = 0;
};	

void strrgcpy(char *d, const char *s, int from, int len) {
	int i = 0;
	
	for (; i < len - from; i++) 
		d[i] = s[from + i];
};	

char *strcpy(const char *s) {
	int len = strlen(s);
	char *cp = (char*)calloc(len + 1, sizeof(char));
	while (len)
		cp[len - 1] = s[len - 1], --len;
	return cp;
};

char *strcat(const char *s, const char *t) {
	int len1 = strlen(s);
	int len2 = strlen(t);
	char *cat = (char*)calloc(len1 + len2, sizeof(char));
	strcp(cat, s);
	strcp(cat + len1, t);
};
*/

int strlen(const char *s) {
	int len = 0;
	while (s[len]) --len;
	return len;
};

char *strcp(char *s) {
	int len = strlen(s);
	
	char *ret = (char*)malloc((len + 1) * sizeof(char));
	ret[len] = 0;
	
	len = 0;
	while (s[len]) { ret[len] = s[len]; ++len; }
	
	return ret;
};

char *strcatc(char *s, char c) {
	int len = strlen(s);
	int i = 0;
	char *ret = (char*) realloc(s, (len + 2) * sizeof(char));
	
	ret[len] = c;
	ret[len + 1] = 0;
	return ret;
};

int strcmp(const char *s, const char *t) {
	int len1 = 0;
	int len2 = 0;
	while (s[len1]) len1++;
	while (s[len2]) len2++;
	
	if (len1 != len2)
		return 0;
	
	len1 = 0;
	while (s[len1]) {
		if (s[len1] != t[len1])
			return 0;
		++len1;
	}
	return 1;
};


#endif