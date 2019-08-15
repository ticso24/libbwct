/*
 * Copyright (c) 2001,02,03,08,09,10 Bernd Walter Computer Technology
 * Copyright (c) 2008,09,10 FIZON GmbH
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#include <bwct/base.h>
#include <bwct/bwct.h>

#define initsize 8

void
String::rebufsize(size_t len)
{
	if (len <= buflen) {
		return;
	}

	size_t nlen = len;

	if (nlen >= 1024) {
		nlen =  (nlen + 1024) & ~1023;
	} else {
		nlen = 8;
		while(len > nlen) {
			nlen <<= 1;
		}
		nlen <<= 1;
	}
	abort_assert(nlen >= len);
	data = (char*)realloc(data, nlen);
	buflen = nlen;
	if (data == NULL) {
		throw std::bad_alloc();
	}
}

void
String::bufsize(size_t len)
{
	size_t nlen = len;

	free(data);
	if (nlen >= 1024) {
		nlen =  (nlen + 1024) & ~1023;
	} else {
		for (int i = 0; i < 10; i++) {
			nlen >>= 1;
			if (nlen == 0) {
				nlen = 1 << (i + 2);
				break;
			}
		}
	}
	abort_assert(nlen >= len);
	data = (char*)malloc(nlen);
	if (data == NULL) {
		throw std::bad_alloc();
	}
	buflen = nlen;
}

String::String()
{
	data = NULL;
	udata = NULL;
	type = Type_Enum::plain;
	bufsize(initsize);
	*data = '\0';
	ln = 0;
}

String::String(const Matrix<char>& rhs)
{
	data = NULL;
	udata = NULL;
	type = Type_Enum::plain;
	size_t rhslen = strlen(rhs.get());
	bufsize(rhslen + 1);
	strcpy(data, rhs.get());
	ln = rhslen;
}

String::String(const char* rhs)
{
	data = NULL;
	udata = NULL;
	type = Type_Enum::plain;
	if (rhs == NULL) {
		log ("rhs == NULL");
		rhs = "(null)";
	}
	size_t rhslen = strlen(rhs);
	bufsize(rhslen + 1);
	strcpy(data, rhs);
	ln = rhslen;
}

String::String(const String &rhs) : Base()
{
	data = NULL;
	udata = NULL;
	type = rhs.type;
	bufsize(rhs.ln + 1);
	strcpy(data, rhs.data);
	ln = rhs.ln;
}

String::String(String &&rhs) : Base()
{
	data = NULL;
	bufsize(initsize);
	*data = '\0';
	std::swap(data, rhs.data);
	udata = rhs.udata;
	rhs.udata = NULL;
	type = rhs.type;
	ln = rhs.ln;
	rhs.ln = 0;
	buflen = rhs.buflen;
}

String::String(bool rhs)
{
	data = NULL;
	udata = NULL;
	type = Type_Enum::plain;
	bufsize(2);
	// no if/else as workaround for CLANG in case of uninitialized rhs
	data[0] = '0';
	data[1] = '\0';
	if (rhs) {
		data[0] = '1';
	}
	ln = strlen(data);
}

String::String(char rhs)
{
	data = NULL;
	udata = NULL;
	type = Type_Enum::plain;
	bufsize(32);
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}

String::String(short rhs)
{
	data = NULL;
	udata = NULL;
	type = Type_Enum::plain;
	bufsize(32);
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}

String::String(int rhs)
{
	data = NULL;
	udata = NULL;
	type = Type_Enum::plain;
	bufsize(32);
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}

String::String(long rhs)
{
	data = NULL;
	udata = NULL;
	type = Type_Enum::plain;
	bufsize(32);
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}

String::String(long long rhs)
{
	data = NULL;
	udata = NULL;
	type = Type_Enum::plain;
	bufsize(32);
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}

String::String(unsigned char rhs)
{
	data = NULL;
	udata = NULL;
	type = Type_Enum::plain;
	bufsize(32);
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}

String::String(unsigned short rhs)
{
	data = NULL;
	udata = NULL;
	type = Type_Enum::plain;
	bufsize(32);
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}

String::String(unsigned int rhs)
{
	data = NULL;
	udata = NULL;
	type = Type_Enum::plain;
	bufsize(32);
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}

String::String(unsigned long rhs)
{
	data = NULL;
	udata = NULL;
	type = Type_Enum::plain;
	bufsize(32);
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}

String::String(unsigned long long rhs)
{
	data = NULL;
	udata = NULL;
	type = Type_Enum::plain;
	bufsize(32);
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}

String::String(double rhs)
{
	data = NULL;
	udata = NULL;
	type = Type_Enum::plain;
	bufsize(1024);
	sprintf(data, "%.100G", rhs);
	ln = strlen(data);
}

String::String(const void *rhs)
{
	data = NULL;
	udata = NULL;
	type = Type_Enum::plain;
	bufsize(32);
	sprintf(data, "%p", rhs);
	ln = strlen(data);
}

String::String(File rhs)
{
	data = NULL;
	udata = NULL;
	type = Type_Enum::plain;
	Stat st(rhs);
	bufsize(st.s.size + 1);
	data[st.s.size] = '\0';
	ssize_t len;
	len = rhs.read(data, st.s.size);
	if (len != (ssize_t)st.s.size) {
		TError("failed to read file");
	}
	ln = len;
}

String::String(const Array<String>& rhs)
{
	data = NULL;
	udata = NULL;
	type = Type_Enum::plain;

	size_t rhssize = 0;
	for (int i = 0; i <= rhs.max; i++) {
		rhssize += rhs[i].ln;
	}
	bufsize(rhssize + 1);
	data[0] = '\0';
	int size = 0;
	for (int i = 0; i <= rhs.max; i++) {
		strcpy(data + size, rhs[i].data);
		size += rhs[i].ln;
		if (i == 0) {
			type = rhs[0].type;
		}
		if (rhs[i].ln != 0) {
			if (type != rhs[i].type) {
				TError(String("string types different"));
			}
			type = rhs[i].type;
		}
	}
	ln = size;
}

const String&
String::operator= (const Matrix<char>& rhs)
{
	size_t rhslen = strlen(rhs.get());
	bufsize(rhslen + 1);
	strcpy(data, rhs.get());
	type = Type_Enum::plain;
	ln = rhslen;
	return *this;
}

const String&
String::operator= (const Array<String>& rhs)
{
	size_t rhssize = 0;
	for (int i = 0; i <= rhs.max; i++) {
		rhssize += rhs[i].ln;
	}
	bufsize(rhssize + 1);
	data[0] = '\0';
	int size = 0;
	for (int i = 0; i <= rhs.max; i++) {
		strcpy(data + size, rhs[i].data);
		size += rhs[i].ln;
		if (i == 0) {
			type = rhs[0].type;
		}
		if (rhs[i].ln != 0) {
			if (type != rhs[i].type) {
				TError(String("string types different"));
			}
			type = rhs[i].type;
		}
	}
	ln = size;
	return *this;
}

const String&
String::operator= (const String &rhs)
{
	char *tmpdata;
	tmpdata = data;
	data = NULL;
	type = rhs.type;
	bufsize(rhs.ln + 1);
	strcpy(data, rhs.data);
	ln = rhs.ln;
	free(tmpdata);
	return *this;
}

const String&
String::operator= (String &&rhs)
{
	std::swap(data, rhs.data);
	std::swap(udata, rhs.udata);
	std::swap(type, rhs.type);
	std::swap(ln, rhs.ln);
	std::swap(buflen, rhs.buflen);
	return *this;
}

const String&
String::operator= (char *rhs)
{
	*this = (const char*) rhs;
	return *this;
}

const String&
String::operator= (const char *rhs)
{
	if (rhs == NULL) {
		log ("rhs == NULL");
		rhs = "(null)";
	}
	type = Type_Enum::plain;
	char *tmpdata;
	tmpdata = data;
	data = NULL;
	size_t rhslen = strlen(rhs);
	bufsize(rhslen + 1);
	strcpy(data, rhs);
	ln = rhslen;
	free(tmpdata);
	return *this;
}

const String&
String::operator= (bool rhs)
{
	bufsize(2);
	// no if/else as workaround for CLANG in case of uninitialized rhs
	data[0] = '0';
	data[1] = '\0';
	if (rhs) {
		data[0] = '1';
	}
	ln = strlen(data);
	type = Type_Enum::plain;
	return *this;
}

const String&
String::operator= (unsigned int rhs)
{
	bufsize(32);
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
	type = Type_Enum::plain;
	return *this;
}

const String&
String::operator= (unsigned long rhs)
{
	bufsize(32);
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
	type = Type_Enum::plain;
	return *this;
}

const String&
String::operator= (unsigned long long rhs)
{
	bufsize(32);
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
	type = Type_Enum::plain;
	return *this;
}

const String&
String::operator= (double rhs)
{
	bufsize(1024);
	sprintf(data, "%.100G", rhs);
	ln = strlen(data);
	type = Type_Enum::plain;
	return *this;
}

const String&
String::operator= (const void *rhs)
{
	bufsize(32);
	sprintf(data, "%p", rhs);
	ln = strlen(data);
	type = Type_Enum::plain;
	return *this;
}

const String&
String::operator= (File rhs)
{
	String tmp(rhs);
	*this = std::move(tmp);
	return *this;
}

bool
String::operator< (const String &rhs) const {
	for (uint64_t i = 0; i <= ln && i <= rhs.ln; i++) {
		if (data[i] > rhs.data[i]) {
			return false;
		} else if (data[i] < rhs.data[i]) {
			return true;
		}
	}
	return false;
}

bool
String::operator> (const String &rhs) const {
	for (uint64_t i = 0; i <= ln && i <= rhs.ln; i++) {
		if (data[i] > rhs.data[i]) {
			return true;
		} else if (data[i] < rhs.data[i]) {
			return false;
		}
	}
	return false;
}

bool
String::operator<= (const String &rhs) const {
	return !(*this > rhs);
}

bool
String::operator>= (const String &rhs) const {
	return !(*this < rhs);
}

const String&
String::join (const Array<String>& rhs, const String &bind)
{
	size_t rhssize = 0;
	for (int i = 0; i <= rhs.max; i++) {
		rhssize += rhs[i].ln;
	}
	if (rhs.max < 0) {
		bufsize(2);
		data[0] = '\0';
		ln = 0;
		return *this;
	}
	bufsize(rhssize + bind.ln * rhs.max + 1);
	data[0] = '\0';
	size_t size = 0;
	for (int i = 0; i <= rhs.max; i++) {
		strcpy(data + size, rhs[i].data);
		size += rhs[i].ln;
		if (i < rhs.max) {
			strcpy(data + size, bind.data);
			size += bind.ln;
		}
		if (i == 0) {
			type = rhs[0].type;
		}
		if (rhs[i].ln != 0) {
			if (type != rhs[i].type) {
				TError(String("string types different"));
			}
			type = rhs[i].type;
		}
	}
	ln = strlen(data);
	return *this;
}

size_t
String::length() const
{
	return ln;
}

size_t
String::u_length() const
{
	String *tmp = const_cast<String*>(this);
	const uint32_t *ud = tmp->u_str();
	size_t len;
	for (len = 0; ud[len] != 0; len++);
	return len;
}

const uint32_t *
String::u_str()
{
	free(udata);
	udata = (uint32_t*)malloc((ln + 1) * 4);
	if (udata == NULL) {
		throw std::bad_alloc();
	}
	size_t pos = 0;
	for (size_t i = 0; i < ln; i++) {
		uint32_t c = 0;
		bool valid = false;
		int rem = ln - i;
		uint8_t d = data[i];
		if (d <= 127) {
			/// single byte encoding
			c = d;
			valid = true;
		} else if ((d & 0xe0) == 0xc0 && rem >= 2) {
			/// 2 byte encoding
			c |= (d & 0x1f) << 6;
			d = data[++i];
			if ((d & 0xc0) == 0x80) {
				c |= d & 0x3f;
				valid = true;
			}
		} else if ((d & 0xf0) == 0xe0 && rem >= 3) {
			/// 3 byte encoding
			c |= (d & 0x0f) << 12;
			d = data[++i];
			if ((d & 0xc0) == 0x80) {
				c |= (d & 0x3f) << 6;
				d = data[++i];
				if ((d & 0xc0) == 0x80) {
					c |= d & 0x3f;
					valid = true;
				}
			}
		} else if ((d & 0xf8) == 0xf0 && rem >= 4) {
			/// 4 byte encoding
			c |= (d & 0x07) << 18;
			d = data[++i];
			if ((d & 0xc0) == 0x80) {
				c |= (d & 0x3f) << 12;
				d = data[++i];
				if ((d & 0xc0) == 0x80) {
					c |= (d & 0x3f) << 6;
					d = data[++i];
					if ((d & 0xc0) == 0x80) {
						c |= d & 0x3f;
						valid = true;
					}
				}
			}
		}
		if (valid) {
			udata[pos++] = c;
		}
	}
	udata[pos++] = 0;

	return udata;
}

const char *
String::c_str() const
{
	return data;
}

String::~String()
{
	free(udata);
	free(data);
}

bool
String::operator== (const String &rhs) const
{
	if (ln != rhs.ln) {
		return false;
	}
	return (strcmp(data, rhs.data) == 0);
}

bool
String::operator== (const Matrix<char>& rhs) const
{
	return (strcmp(data, rhs.get()) == 0);
}

bool
String::operator== (const char *rhs) const
{
	if (rhs == NULL) {
		log ("rhs == NULL");
		rhs = "(null)";
	}
	return (strcmp(data, rhs) == 0);
}

bool
String::operator!= (const String &rhs) const
{
	if (ln != rhs.ln) {
		return true;
	}
	return (strcmp(data, rhs.data) != 0);
}

bool
String::operator!= (const Matrix<char>& rhs) const
{
	return (strcmp(data, rhs.get()) != 0);
}

bool
String::operator!= (const char *rhs) const
{
	if (rhs == NULL) {
		log ("rhs == NULL");
		rhs = "(null)";
	}
	return (strcmp(data, rhs) != 0);
}

String
String::operator+(const String &rhs) const {
	String nstr(*this);
	nstr += rhs;
	return nstr;
}

String
String::operator+(String &&rhs) const {
	String nstr(*this);
	nstr += std::move(rhs);
	return nstr;
}

String&
String::operator+= (const String &rhs)
{
	if (ln == 0) {
		type = rhs.type;
	}
	rebufsize(ln + rhs.ln + 1);
	strcpy(data + ln, rhs.data);
	ln += rhs.ln;
	return *this;
}

String&
String::operator+= (String &&rhs)
{
	if (ln == 0) {
		std::swap(type, rhs.type);
		std::swap(ln, rhs.ln);
		std::swap(data, rhs.data);
		std::swap(udata, rhs.udata);
		std::swap(buflen, rhs.buflen);
	} else {
		rebufsize(ln + rhs.ln + 1);
		strcpy(data + ln, rhs.data);
		ln += rhs.ln;
	}
	return *this;
}

String&
String::operator+= (const Matrix<char>& rhs)
{
	size_t rhslen = strlen(rhs.get());
	rebufsize(ln + rhslen + 1);
	strcpy(data + ln, rhs.get());
	ln += rhslen;
	return *this;
}

String&
String::operator+= (const Array<String>& rhs)
{
	String tmp;
	tmp = rhs;
	*this += tmp;
	return *this;
}

String&
String::operator+= (const char *rhs)
{
	if (rhs == NULL) {
		log ("rhs == NULL");
		rhs = "(null)";
	}
	size_t rhslen = strlen(rhs);
	rebufsize(ln + rhslen + 1);
	strcpy(data + ln, rhs);
	ln += rhslen;
	return *this;
}

String&
String::operator+=(bool rhs)
{
	return (*this) += String(rhs);
}

String&
String::operator+=(char rhs)
{
	return (*this) += String(rhs);
}

String&
String::operator+=(short rhs)
{
	return (*this) += String(rhs);
}

String&
String::operator+=(int rhs)
{
	return (*this) += String(rhs);
}

String&
String::operator+=(long rhs)
{
	return (*this) += String(rhs);
}

String&
String::operator+=(long long rhs)
{
	return (*this) += String(rhs);
}

String&
String::operator+=(unsigned char rhs)
{
	return (*this) += String(rhs);
}

String&
String::operator+=(unsigned short rhs)
{
	return (*this) += String(rhs);
}

String&
String::operator+=(unsigned int rhs)
{
	return (*this) += String(rhs);
}

String&
String::operator+=(unsigned long rhs)
{
	return (*this) += String(rhs);
}

String&
String::operator+=(unsigned long long rhs)
{
	return (*this) += String(rhs);
}

String&
String::operator+=(double rhs)
{
	return (*this) += String(rhs);
}

String&
String::operator+=(const void *rhs)
{
	return (*this) += String(rhs);
}

void
String::resize(size_t rhs)
{
	if (rhs < ln) {
		data[rhs] = 0;
		ln = rhs;
	}
}

bool
String::empty() const
{
	return (ln == 0);
}

size_t
String::begin() const
{
	return 0;
};

size_t
String::end() const
{
	return ln;
};

const char&
String::operator[](const size_t i) const
{
	cassert(i <= ln);
	return data[i];
};

char&
String::operator[](const size_t i)
{
	cassert(i <= ln);
	return data[i];
};

void
String::lower()
{
	for (size_t i = 0; i < ln; i++)
		data[i] = tolower(data[i]);
}

void
String::upper()
{
	for (size_t i = 0; i < ln; i++)
		data[i] = toupper(data[i]);
}

bool
String::strncmp(int64_t frompos, const String &rhs) const
{
	cassert(frompos >= 0);

	if (ln + frompos < rhs.ln) {
		return false;
	}
	return (::strncmp(data + frompos, rhs.data, rhs.ln) == 0);
}

bool
String::strncmp(const String &rhs, size_t len) const
{
	return (::strncmp(data, rhs.data, len) == 0);
}

bool
String::strncmp(const String &rhs) const
{
	if (ln < rhs.ln) {
		return false;
	}
	return (::strncmp(data, rhs.data, rhs.ln) == 0);
}

bool
String::strncmp(const char *rhs) const
{
	return (::strncmp(data, rhs, strlen(rhs)) == 0);
}

String
String::u_cut(size_t begin, ssize_t end) const
{
	String tmp;
	tmp = *this;
	const uint32_t *ud = tmp.u_str();
	ssize_t len;
	for (len = 0; ud[len] != 0; len++);

	if ((ssize_t)begin > len) {
		throw Error(String("String::cut begin beyound size"));
	}
	if (end > len) {
		throw Error(String("String::cut end beyound size"));
	}

	if (end < 0) {
		end = len;
	}

	String ret;
	ret.type = type;

	char d[] = {0, '\0'};

	for (ssize_t pos = begin; pos < end; pos++) {
		if (ud[pos] <= 0x7f) {
			d[0] = ud[pos];
			ret += d;
		} else if (ud[pos] <= 0x07ff) {
			d[0] = 0xc0 | ((ud[pos] >> 6) & 0x1f);
			ret += d;
			d[0] = 0x80 | ((ud[pos] >> 0) & 0x3f);
			ret += d;
		} else if (ud[pos] <= 0xffff) {
			d[0] = 0xe0 | ((ud[pos] >> 12) & 0x0f);
			ret += d;
			d[0] = 0x80 | ((ud[pos] >> 6) & 0x3f);
			ret += d;
			d[0] = 0x80 | ((ud[pos] >> 0) & 0x3f);
			ret += d;
		} else if (ud[pos] <= 0x1fffff) {
			d[0] = 0xf0 | ((ud[pos] >> 18) & 0x07);
			ret += d;
			d[0] = 0x80 | ((ud[pos] >> 12) & 0x3f);
			ret += d;
			d[0] = 0x80 | ((ud[pos] >> 6) & 0x3f);
			ret += d;
			d[0] = 0x80 | ((ud[pos] >> 0) & 0x3f);
			ret += d;
		} else if (ud[pos] <= 0x3ffffff) {
			d[0] = 0xf8 | ((ud[pos] >> 24) & 0x03);
			ret += d;
			d[0] = 0x80 | ((ud[pos] >> 18) & 0x3f);
			ret += d;
			d[0] = 0x80 | ((ud[pos] >> 12) & 0x3f);
			ret += d;
			d[0] = 0x80 | ((ud[pos] >> 6) & 0x3f);
			ret += d;
			d[0] = 0x80 | ((ud[pos] >> 0) & 0x3f);
			ret += d;
		} else if (ud[pos] <= 0x7fffffff) {
			d[0] = 0xfc | ((ud[pos] >> 30) & 0x01);
			ret += d;
			d[0] = 0x80 | ((ud[pos] >> 24) & 0x3f);
			ret += d;
			d[0] = 0x80 | ((ud[pos] >> 18) & 0x3f);
			ret += d;
			d[0] = 0x80 | ((ud[pos] >> 12) & 0x3f);
			ret += d;
			d[0] = 0x80 | ((ud[pos] >> 6) & 0x3f);
			ret += d;
			d[0] = 0x80 | ((ud[pos] >> 0) & 0x3f);
			ret += d;
		}
	}

	return ret;
}

String
String::cut(size_t begin, ssize_t end) const
{
	String ret;
	ret.type = type;

	if (begin > ln) {
		throw Error(String("String::cut begin beyond size"));
	}
	if (end > (ssize_t)ln) {
		throw Error(String("String::cut end beyond size"));
	}
	ret = data + begin;
	if (end >= 0) {
		ret.data[end - begin + 1] = '\0';
		ret.ln = strlen(ret.data);
	}
	return ret;
}

Array<String>
String::CSVsplit() const
{
	Array<String> ret;
	bool quote = false;
	String tmpres;
	const char *pos;
	int i = 0;

	for (pos = data; *pos != '\0'; pos++) {
		if (*pos == '"') {
			quote = !quote;
		} else {
			if (*pos == ';' && !quote) {
				ret[i++] = std::move(tmpres);
				tmpres = "";
			} else {
				tmpres.rebufsize(tmpres.ln + 2);
				tmpres.data[tmpres.ln] = *pos;
				tmpres.ln++;
				tmpres.data[tmpres.ln] = '\0';
			}
		}
	}
	ret[i++] = std::move(tmpres);

	return ret;
}

Array<String>
String::strsplit(String delim) const
{
	Array<String> ret;
	String tmp;
	char *match;
	char *start = data;
	char *oldstart;
	size_t length;

	do {
		match = strstr(start, delim.data);
		oldstart = start;
		if (match != NULL) {
			start = &match[delim.ln];
			length = match - oldstart;
		} else {
			length = data + ln - oldstart;
		}
		tmp.bufsize(length + 1);
		for (size_t i = 0; i < length; i++) {
			tmp.data[i] = oldstart[i];
		}
		tmp.data[length] = '\0';
		tmp.type = type;
		tmp.ln = length;
		ret[ret.max + 1] = std::move(tmp);
	} while (match != NULL);

	return ret;
}

Array<String>
String::split(String delim) const
{
	Array<String> ret;
	int i;
	char *res;
	char *tmp;
	String copy(*this);

	i = 0;
	tmp = copy.data;
	do {
		res = strsep(&tmp, delim.c_str());
		if (res != NULL) {
			ret[i] = res;
			ret[i].type = type;
			i++;
		}
	} while (res != NULL);

	return ret;
}

bool
String::contains(const String& rhs) const
{
	if (strstr(data, rhs.data) != NULL) {
		return true;
	}
	return false;
}

String
String::trim() const
{
	String ret;
	ssize_t first = 0;
	ssize_t last = ln - 1;

	for (ssize_t i = 0; i < (ssize_t)ln; i++) {
		if (data[i] == ' ') {
			first = i + 1;
		} else {
			break;
		}
	}
	for (ssize_t i = ln - 1; i >= 0; i--) {
		if (data[i] == ' ') {
			last = i - 1;
		} else {
			break;
		}
	}

	if (first > (ssize_t)ln - 1 || last < 0) {
		return ret;
	}

	ret = cut(first, last);
	return ret;
}

String
String::trimstart(const char value) const
{
	String ret;
	ssize_t first = 0;
	ssize_t last = ln - 1;

	for (ssize_t i = 0; i < (ssize_t)ln; i++) {
		if (data[i] == value) {
			first = i + 1;
		} else {
			break;
		}
	}

	if (first > (ssize_t)ln - 1 || last < 0) {
		return ret;
	}

	ret = cut(first, last);
	return ret;
}

String
String::trimend(const char value) const
{
	String ret;
	ssize_t first = 0;
	ssize_t last = ln - 1;

	for (ssize_t i = ln - 1; i >= 0; i--) {
		if (data[i] == value) {
			last = i - 1;
		} else {
			break;
		}
	}

	if (first > (ssize_t)ln - 1 || last < 0) {
		return ret;
	}

	ret = cut(first, last);
	return ret;
}

bool
String::replacefirst(const String &search, const String &replace)
{
	char *match;
	String tmp;

	match = strstr(data, search.data);
	if (match != NULL) {
		*match = '\0';
		tmp = data;
		tmp += replace;
		tmp += (match + search.ln);
		*this = tmp;
		return true;
	}
	return false;
}

int
String::replace(const String &search, const String &replace)
{
	char *match;
	char *remaining;
	String tmp;
	int matches = 0;

	match = strstr(data, search.data);
	remaining = data;
	while (match != NULL) {
		matches++;
		*match = '\0';
		tmp += remaining;
		tmp += replace;
		remaining = match + search.ln;
		match = strstr(remaining, search.data);
	}
	if (matches > 0) {
		tmp += remaining;
		*this = tmp;
	}
	return matches;
}

double
String::getd() const
{
	double ret;

	ret = strtod(data, NULL);
	return ret;
}

long long
String::getll() const
{
	long long ret;

	ret = atoll(data);
	return ret;
}

#if 0
String
String::re_subst(const String& re)
{
	String ret;
#ifdef no_yet
	regex_t rx;
	int res;
	Array<String> splitre;
	const size_t err_bufsize = 1024;
	regmatch_t pmatch;

	splitre = re.split("/");
	cassert(splitre.max == 2);
	cassert(splitre[0] == "s");

	String lh = splitre[1];
	String rh = splitre[2];

	res = regcomp(&rx, lh.c_str(), REG_EXTENDED);
	if (res != 0) {
		a_ptr<char> error;
		error = new char[err_bufsize];
		regerror(res, &rx, error.get(), err_bufsize);
		//regfree(&rx);
		throw Error(String(error.get()));
	}
	pmatch.rm_so = 0;
	pmatch.rm_eo = ln;
	res = regexec(&rx, data, 0, &pmatch, 0);
	if (res == REG_NOMATCH) {
		regfree(&rx);
		return (*this);
	}

	if (res != 0) {
		a_ptr<char> error;
		error = new char[err_bufsize];
		regerror(res, &rx, error.get(), err_bufsize);
		regfree(&rx);
		throw Error(String(error.get()));
	}

	char buf[2];
	buf[1] = '\0';
	const char *rp;
	const char *wp;
	VarPattern *pat;
	if (pat->matches[0].rm_so > 0) {
		for (int i = 0; i < pat->matches[0].rm_so; i++) {
			buf[0] = wp[i];
			ret += buf;
		}
	}
	for (rp = rh.c_str(); *rp; rp++) {
		if ((*rp == '\\') && ((rp[1] == '&') || (rp[1] == '\\'))) {
			buf[0] = rp[1];
			ret += buf;
			rp++;

		} else if ((*rp == '&') ||
		    ((*rp == '\\') && isdigit((unsigned char)rp[1]))) {
			int n;
			const char *subbuf;
			int sublen;
			char errstr[3];

			if (*rp == '&') {
				n = 0;
				errstr[0] = '&';
				errstr[1] = '\0';
			} else {
				n = rp[1] - '0';
				errstr[0] = '\\';
				errstr[1] = rp[1];
				errstr[2] = '\0';
				rp++;
			}

			if (n > pat->nsub) {
				regfree(&rx);
				throw Error(S + "No subexpression " + &errstr[0]);

			} else if ((pat->matches[n].rm_so == -1) && (pat->matches[n].rm_eo == -1)) {
				regfree(&rx);
				throw Error(S + "No match for subexpression " + &errstr[0]);

			} else {
				subbuf = wp + pat->matches[n].rm_so;
				sublen = pat->matches[n].rm_eo - pat->matches[n].rm_so;
			}

			if (sublen > 0) {
				for (int i = 0; i < sublen; i++) {
					buf[0] = subbuf[i];
					ret += buf;
				}
			}
		} else {
			buf[0] = *rp;
			ret += buf;
		}
	}
	wp += pat->matches[0].rm_eo;
	if (*wp) {
		ret += wp;
	}

	regfree(&rx);

#endif
	return ret;
}
#endif

#if 0
bool
String::re_comp(const String& re) const
{
	regex_t rx;
	int res;
	regmatch_t pmatch;
	const size_t err_bufsize = 1024;

	//log(S + "compare " + *this + " with " + re);
	res = regcomp(&rx, re.c_str(), REG_EXTENDED);
	if (res != 0) {
		a_ptr<char> error;
		error = new char[err_bufsize];
		regerror(res, &rx, error.get(), err_bufsize);
		//regfree(&rx);
		throw Error(String(error.get()));
	}
	pmatch.rm_so = 0;
	pmatch.rm_eo = ln;
	res = regexec(&rx, data, 0, &pmatch, 0);
	if (res != 0 && res != REG_NOMATCH) {
		a_ptr<char> error;
		error = new char[err_bufsize];
		regerror(res, &rx, error.get(), err_bufsize);
		regfree(&rx);
		throw Error(String(error.get()));
	}
	regfree(&rx);

	if (res == REG_NOMATCH) {
		return false;
	}
	return true;
}
#endif

String
String::printf(String format, ...)
{
	va_list ap;
	free(data);
	va_start(ap, format);
	vasprintf(&data, format.c_str(), ap);
	va_end(ap);
	ln = strlen(data);
	buflen = ln + 1;
	return *this;
}

bool
String::is_numeric()
{
	for (size_t i = 0; i < ln; i++) {
		if (data[i] < '0' || data[i] > '9') {
			return false;
		}
	}
	return true;
}

void
String::reverse()
{
	char tmp[ln + 1];
	for (int64_t i = ln - 1, j = 0; i >= 0; i--, j++) {
		tmp[j] = data[i];
	}
	tmp[ln] = '\0';
	strcpy(data, tmp);
}

#if 0
// XXX Wrapper object to handle const/non-const char** in iconv
class iconv_wrap
{
	char** data;
	public:
		iconv_wrap(char** d)
		{
			data = d;
		}
		operator char** () const
		{
			return data;
		}
		operator const char** () const
		{
			return const_cast<const char**>(data);
		}
};
#endif

#if 0
const String&
String::convert(const String& from, const String& to)
{
	if (ln != 0) {
		size_t buflen = ln * 4 + 4;
		a_ptr<char> tmpbuf;
		tmpbuf = new char[buflen];
		iconv_t ic = iconv_open(to.c_str(), from.c_str());
		ssize_t res;
		iconv_wrap inbuf(&data);
		size_t inbuflen = ln;
		char *outbuf = tmpbuf.get();
		size_t outbuflen = buflen - 1;
		while(inbuflen > 0) {
			// those Linux idiots are so clever to return -1 on error with an unsigned size_t.
			res = (ssize_t)iconv(ic, inbuf, &inbuflen, &outbuf, &outbuflen);
			if (res < 0) {
				abort_assert(errno != E2BIG);
				if (errno == EILSEQ || errno == EINVAL) {
					// input encoding error
					// skip a byte and continue with the rest
					log("iconv conversion error");
					char** inbuf_help = inbuf;
					(*inbuf_help)++;
					inbuflen--;
				} else {
					iconv_close(ic);
					TError(S + "unknown conversion failure");
				}
			}
		}
		iconv_close(ic);
		*outbuf = '\0';
		*this = tmpbuf.get();
	}
	return *this;
}
#endif

bool
String::test_utf() const
{
	if (ln != 0) {
		for (uint64_t i = 0; i <= ln; i++) {
			int continuation = 0;
			if ((data[i] & 0xe0) == 0xc0) {
				// 2 byte encoding
				continuation = 1;
			} else if ((data[i] & 0xf0) == 0xe0) {
				// 3 byte encoding
				continuation = 2;
			} else if ((data[i] & 0xf8) == 0xf0) {
				// 4 byte encoding
				continuation = 3;
			} else if (data[i] & 0x80) {
				return false;
			}
			for (int j = 0; j < continuation; j++) {
				// test for continuation characters
				i++;
				if (i > ln || ((data[i] & 0xc0) != 0x80)) {
					return false;
				}
			}
		}
	}
	return true;
}

