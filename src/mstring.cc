/*
 * Copyright (c) 2001,02,03,08,09,10 Bernd Walter Computer Technology
 * Copyright (c) 2008,09,10 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/string.cc $
 * $Date: 2025-06-22 18:40:52 +0200 (Sun, 22 Jun 2025) $
 * $Author: ticso $
 * $Rev: 49411 $
 */

#include "bwct.h"

namespace bwct
{
	void
	String::free_data() noexcept
	{
		if (buflen > directsize) {
			free(data);
		}
		data = NULL;
		free(udata);
		udata = NULL;
		buflen = directsize;
		ln = 0;
		sdata[0] = '\0';
	}

	char *
	String::get_data() noexcept
	{
		if (buflen <= directsize) {
			return sdata;
		} else {
			return data;
		}
	}

	const char *
	String::get_data() const noexcept
	{
		if (buflen <= directsize) {
			return sdata;
		} else {
			return data;
		}
	}

	void
	String::rebufsize(size_t len)
	{
		if (len <= buflen) {
			return;
		}

		size_t nlen = len;

		if (nlen > buflen) {
			if (nlen >= 1024) {
				nlen = (nlen + 1024) & 0xfffffc00;
			} else {
				nlen = 8;
				while(len > nlen) {
					nlen <<= 1;
				}
				nlen <<= 1;
			}
			abort_assert(nlen >= len);
			char* tmp;
			if (buflen <= directsize) {
				tmp = (char*)malloc(nlen);
				if (tmp == NULL) {
					throw std::bad_alloc();
				}
				strcpy(tmp, sdata);
			} else {
				tmp = (char*)realloc(data, nlen);
				if (tmp == NULL) {
					throw std::bad_alloc();
				}
			}
			data = tmp;
			buflen = nlen;
		}
	}

	void
	String::set_size(size_t len)
	{
		rebufsize(len + 1);
	}

	void
	String::bufsize(size_t len)
	{
		size_t nlen = len;

		free_data();
		if (len <= directsize) {
			return;
		}
		if (nlen >= 1024) {
			nlen = (nlen + 1024) & 0xfffffc00;
		} else {
			nlen = 8;
			while(len > nlen) {
				nlen <<= 1;
			}
			nlen <<= 1;
		}
		abort_assert(nlen >= len);
		data = (char*)malloc(nlen);
		if (data == NULL) {
			throw std::bad_alloc();
		}
		buflen = nlen;
	}

	String::String() noexcept
	{
		udata = NULL;
		buflen = directsize;
		sdata[0] = '\0';
		ln = 0;
	}

	String::String(const char* rhs)
	{
		data = NULL;
		udata = NULL;
		buflen = directsize;
		if (rhs == NULL) {
			log ("rhs == NULL");
			rhs = "(null)";
		}
		size_t rhslen = strlen(rhs);
		bufsize(rhslen + 1);
		strcpy(get_data(), rhs);
		ln = rhslen;
	}

	String::String(const String &rhs)
	{
		data = NULL;
		udata = NULL;
		buflen = directsize;
		bufsize(rhs.ln + 1);
		strcpy(get_data(), rhs.get_data());
		ln = rhs.ln;
	}

	String::String(String &&rhs) noexcept
	{
		data = NULL;
		udata = NULL;
		buflen = directsize;
		bufsize(directsize);
		get_data()[0]= '\0';
		std::swap(data, rhs.data);
		udata = rhs.udata;
		rhs.udata = NULL;
		ln = rhs.ln;
		rhs.ln = 0;
		buflen = rhs.buflen;
	}

	String::String(bool rhs) noexcept
	{
		data = NULL;
		udata = NULL;
		buflen = directsize;
		// no if/else as workaround for CLANG in case of uninitialized rhs
		if (rhs) {
			sdata[0] = '1';
		} else {
			sdata[0] = '0';
		}
		sdata[1] = '\0';
		ln = 1;
	}

	String::String(char rhs)
	{
		data = NULL;
		udata = NULL;
		buflen = directsize;
		bufsize(5); // -128 + \0
		sprintf(get_data(), "%lld", LL(rhs));
		ln = strlen(get_data());
	}

	String::String(short rhs)
	{
		data = NULL;
		udata = NULL;
		buflen = directsize;
		bufsize(7); // -32768 + \0
		sprintf(get_data(), "%lld", LL(rhs));
		ln = strlen(get_data());
	}

	String::String(int rhs)
	{
		data = NULL;
		udata = NULL;
		buflen = directsize;
		bufsize(32);
		sprintf(get_data(), "%lld", LL(rhs));
		ln = strlen(get_data());
	}

	String::String(long rhs)
	{
		data = NULL;
		udata = NULL;
		buflen = directsize;
		bufsize(32);
		sprintf(get_data(), "%lld", LL(rhs));
		ln = strlen(get_data());
	}

	String::String(long long rhs)
	{
		data = NULL;
		udata = NULL;
		buflen = directsize;
		bufsize(32);
		sprintf(get_data(), "%lld", LL(rhs));
		ln = strlen(get_data());
	}

	String::String(unsigned char rhs)
	{
		data = NULL;
		udata = NULL;
		buflen = directsize;
		bufsize(4); // 255 + \0
		sprintf(get_data(), "%lld", LL(rhs));
		ln = strlen(get_data());
	}

	String::String(unsigned short rhs)
	{
		data = NULL;
		udata = NULL;
		buflen = directsize;
		bufsize(6); // 65535 + \0
		sprintf(get_data(), "%lld", LL(rhs));
		ln = strlen(get_data());
	}

	String::String(unsigned int rhs)
	{
		data = NULL;
		udata = NULL;
		buflen = directsize;
		bufsize(32);
		sprintf(get_data(), "%lld", LL(rhs));
		ln = strlen(get_data());
	}

	String::String(unsigned long rhs)
	{
		data = NULL;
		udata = NULL;
		buflen = directsize;
		bufsize(32);
		sprintf(get_data(), "%lld", LL(rhs));
		ln = strlen(get_data());
	}

	String::String(unsigned long long rhs)
	{
		data = NULL;
		udata = NULL;
		buflen = directsize;
		bufsize(32);
		sprintf(get_data(), "%lld", LL(rhs));
		ln = strlen(get_data());
	}

	String::String(double rhs)
	{
		data = NULL;
		udata = NULL;
		buflen = directsize;
		bufsize(1024);
		sprintf(get_data(), "%.100G", rhs);
		ln = strlen(get_data());
	}

	String::String(const void *rhs)
	{
		data = NULL;
		udata = NULL;
		buflen = directsize;
		bufsize(32);
		sprintf(get_data(), "%p", rhs);
		ln = strlen(get_data());
	}

	String::String(File rhs)
	{
		data = NULL;
		udata = NULL;
		buflen = directsize;
		Stat st(rhs);
		bufsize(st.s.size + 1);
		get_data()[st.s.size] = '\0';
		ssize_t len;
		len = rhs.read(get_data(), st.s.size);
		if (len != (ssize_t)st.s.size) {
			TError("failed to read file");
		}
		ln = len;
	}

	String::String(const JSON& rhs)
	{
		data = NULL;
		udata = NULL;
		ln = 0;
		buflen = directsize;
		*this = rhs.get_str();
	}

	String::String(const Array<String>& rhs)
	{
		data = NULL;
		udata = NULL;
		buflen = directsize;

		size_t rhssize = 0;
		for (int i = 0; i <= rhs.max; i++) {
			rhssize += rhs[i].ln;
		}
		bufsize(rhssize + 1);
		char *d = get_data();
		d[0] = '\0';
		int size = 0;
		for (int i = 0; i <= rhs.max; i++) {
			strcpy(d + size, rhs[i].get_data());
			size += rhs[i].ln;
		}
		ln = size;
	}

	void
	String::clear() noexcept
	{
		free_data();
	}

	const String&
	String::operator= (const Array<String>& rhs)
	{
		size_t rhssize = 0;
		for (int i = 0; i <= rhs.max; i++) {
			rhssize += rhs[i].ln;
		}
		if (rhssize == 0) {
			free_data();
			return *this;
		}
		bufsize(rhssize + 1);
		char* d = get_data();
		d[0] = '\0';
		int size = 0;
		for (int i = 0; i <= rhs.max; i++) {
			strcpy(d + size, rhs[i].get_data());
			size += rhs[i].ln;
		}
		ln = size;
		return *this;
	}

	const String&
	String::operator= (const String &rhs)
	{
		free_data();
		if (rhs.ln == 0) {
			return *this;
		}
		bufsize(rhs.ln + 1);
		strcpy(get_data(), rhs.get_data());
		ln = rhs.ln;
		return *this;
	}

	const String&
	String::operator= (String &&rhs) noexcept
	{
		if (rhs.ln <= directsize) {
			free_data();
			if (rhs.ln == 0) {
				return *this;
			}
			bufsize(rhs.ln + 1);
			strcpy(get_data(), rhs.get_data());
			ln = rhs.ln;
		} else {
			std::swap(data, rhs.data);
			std::swap(udata, rhs.udata);
			std::swap(ln, rhs.ln);
			std::swap(buflen, rhs.buflen);
		}
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
		size_t rhslen = strlen(rhs);
		bufsize(rhslen + 1);
		strcpy(get_data(), rhs);
		ln = rhslen;
		return *this;
	}

	const String&
	String::operator= (bool rhs)
	{
		bufsize(directsize);
		// no if/else as workaround for CLANG in case of uninitialized rhs
		if (rhs) {
			sdata[0] = '1';
		} else {
			sdata[0] = '0';
		}
		sdata[1] = '\0';
		ln = 1;
		return *this;
	}

	const String&
	String::operator= (unsigned int rhs)
	{
		bufsize(32);
		sprintf(get_data(), "%lld", LL(rhs));
		ln = strlen(get_data());
		return *this;
	}

	const String&
	String::operator= (unsigned long rhs)
	{
		bufsize(32);
		sprintf(get_data(), "%lld", LL(rhs));
		ln = strlen(get_data());
		return *this;
	}

	const String&
	String::operator= (unsigned long long rhs)
	{
		bufsize(32);
		sprintf(get_data(), "%lld", LL(rhs));
		ln = strlen(get_data());
		return *this;
	}

	const String&
	String::operator= (double rhs)
	{
		bufsize(1024);
		sprintf(get_data(), "%.100G", rhs);
		ln = strlen(get_data());
		return *this;
	}

	const String&
	String::operator= (const void *rhs)
	{
		bufsize(32);
		sprintf(get_data(), "%p", rhs);
		ln = strlen(get_data());
		return *this;
	}

	const String&
	String::operator= (File rhs)
	{
		String tmp(rhs);
		*this = std::move(tmp);
		return *this;
	}

	const String&
	String::operator= (const JSON& rhs)
	{
		*this = rhs.get_str();
		return *this;
	}

	const String&
	String::operator=(const std::string& rhs) {
		const char* rstr = rhs.c_str();
		const auto rlen = rhs.length();

		bufsize(rlen + 1);
		strcpy(get_data(), rstr);
		ln = rlen;
		return *this;
	}

	bool
	String::operator< (const String &rhs) const noexcept {
		const char* d = get_data();
		const char* d2 = rhs.get_data();
		for (uint64_t i = 0; i <= ln && i <= rhs.ln; i++) {
			if (d[i] > d2[i]) {
				return false;
			} else if (d[i] < d2[i]) {
				return true;
			}
		}
		return false;
	}

	bool
	String::operator> (const String &rhs) const noexcept {
		const char* d = get_data();
		const char* d2 = rhs.get_data();
		for (uint64_t i = 0; i <= ln && i <= rhs.ln; i++) {
			if (d[i] > d2[i]) {
				return true;
			} else if (d[i] < d2[i]) {
				return false;
			}
		}
		return false;
	}

	bool
	String::operator<= (const String &rhs) const noexcept {
		return !(*this > rhs);
	}

	bool
	String::operator>= (const String &rhs) const noexcept {
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
			free_data();
			return *this;
		}
		bufsize(rhssize + bind.ln * rhs.max + 1);
		char* d = get_data();
		d[0] = '\0';
		size_t size = 0;
		for (int i = 0; i <= rhs.max; i++) {
			strcpy(d + size, rhs[i].get_data());
			size += rhs[i].ln;
			if (i < rhs.max) {
				strcpy(d + size, bind.get_data());
				size += bind.ln;
			}
		}
		ln = strlen(d);
		return *this;
	}

	void
	String::push_back(char c)
	{
		rebufsize(ln + 2);
		auto p = get_data();
		p[ln] = c;
		p[ln + 1] = '\0';
		++ln;
	}

	void
	String::push_front(char c)
	{
		rebufsize(ln + 2);
		auto p = get_data();
		memmove(p + 1, p, ln + 1);
		*p = c;
	}

	void
	String::pop_front()
	{
		auto p = get_data();
		memmove(p, p + 1, ln);
		--ln;
	}

	void
	String::pop_back()
	{
		get_data()[ln] = '\0';
		--ln;
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
		char* da = get_data();
		for (size_t i = 0; i < ln; i++) {
			uint32_t c = 0;
			bool valid = false;
			int rem = ln - i;
			uint8_t d = da[i];
			if (d <= 127) {
				/// single byte encoding
				c = d;
				valid = true;
			} else if ((d & 0xe0) == 0xc0 && rem >= 2) {
				/// 2 byte encoding
				c |= (d & 0x1f) << 6;
				d = da[++i];
				if ((d & 0xc0) == 0x80) {
					c |= d & 0x3f;
					valid = true;
				}
			} else if ((d & 0xf0) == 0xe0 && rem >= 3) {
				/// 3 byte encoding
				c |= (d & 0x0f) << 12;
				d = da[++i];
				if ((d & 0xc0) == 0x80) {
					c |= (d & 0x3f) << 6;
					d = da[++i];
					if ((d & 0xc0) == 0x80) {
						c |= d & 0x3f;
						valid = true;
					}
				}
			} else if ((d & 0xf8) == 0xf0 && rem >= 4) {
				/// 4 byte encoding
				c |= (d & 0x07) << 18;
				d = da[++i];
				if ((d & 0xc0) == 0x80) {
					c |= (d & 0x3f) << 12;
					d = da[++i];
					if ((d & 0xc0) == 0x80) {
						c |= (d & 0x3f) << 6;
						d = da[++i];
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
	String::c_str() const noexcept
	{
		return get_data();
	}

	String::~String() noexcept
	{
		if (buflen > directsize) {
			free(data);
		}
		free(udata);
	}

	bool
	String::operator== (const String &rhs) const noexcept
	{
		if (ln != rhs.ln) {
			return false;
		}
		return (strcmp(get_data(), rhs.get_data()) == 0);
	}

	bool
	String::operator== (const char *rhs) const noexcept
	{
		if (rhs == NULL) {
			log ("rhs == NULL");
			rhs = "(null)";
		}
		return (strcmp(get_data(), rhs) == 0);
	}

	bool
	String::operator!= (const String &rhs) const noexcept
	{
		if (ln != rhs.ln) {
			return true;
		}
		return (strcmp(get_data(), rhs.get_data()) != 0);
	}

	bool
	String::operator!= (const char *rhs) const noexcept
	{
		if (rhs == NULL) {
			log ("rhs == NULL");
			rhs = "(null)";
		}
		return (strcmp(get_data(), rhs) != 0);
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

	void
	String::add_memory(const void* data, size_t len)
	{
		if (len == 0) {
			return;
		}
		for (size_t i = 0; i <= len; i++) {
			if (*(char*)data == '\0') {
				len = i;
				break;
			}
		}
		rebufsize(ln + len + 1);
		memcpy(this->get_data() + ln, data, len);
		ln += len;
		this->get_data()[ln] = '\0';
	}

	String&
	String::operator+= (const String &rhs)
	{
		if (rhs.ln == 0) {
			return *this;
		}
		rebufsize(ln + rhs.ln + 1);
		strcpy(get_data() + ln, rhs.get_data());
		ln += rhs.ln;
		return *this;
	}

	String&
	String::operator+= (String &&rhs)
	{
		if (rhs.ln == 0) {
			return *this;
		}
		if (ln == 0 && buflen == directsize) {
			std::swap(ln, rhs.ln);
			std::swap(data, rhs.data);
			std::swap(udata, rhs.udata);
			std::swap(buflen, rhs.buflen);
		} else {
			rebufsize(ln + rhs.ln + 1);
			strcpy(get_data() + ln, rhs.get_data());
			ln += rhs.ln;
		}
		return *this;
	}

	String&
	String::operator+= (const Array<String>& rhs)
	{
		String tmp;
		tmp = rhs;
		*this += std::move(tmp);
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
		strcpy(get_data() + ln, rhs);
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
			get_data()[rhs] = '\0';
			ln = rhs;
		}
	}

	bool
	String::empty() const noexcept
	{
		return (ln == 0);
	}

	const char&
	String::operator[](const size_t i) const
	{
		cassert(i <= ln);
		return get_data()[i];
	};

	char&
	String::operator[](const size_t i)
	{
		cassert(i <= ln);
		return get_data()[i];
	};

	const char&
	String::at(const size_t i) const
	{
		cassert(i <= ln);
		return get_data()[i];
	};

	char&
	String::at(const size_t i)
	{
		cassert(i <= ln);
		return get_data()[i];
	};

	void
	String::lower() noexcept
	{
		for (size_t i = 0; i < ln; i++)
			get_data()[i] = tolower(get_data()[i]);
	}

	void
	String::upper() noexcept
	{
		for (size_t i = 0; i < ln; i++)
			get_data()[i] = toupper(get_data()[i]);
	}

	bool
	String::strncmp(int64_t frompos, const String &rhs) const
	{
		cassert(frompos >= 0);

		if (ln + frompos < rhs.ln) {
			return false;
		}
		return (::strncmp(get_data() + frompos, rhs.get_data(), rhs.ln) == 0);
	}

	bool
	String::strncmp(const String &rhs, size_t len) const noexcept
	{
		return (::strncmp(get_data(), rhs.get_data(), len) == 0);
	}

	bool
	String::strncmp(const String &rhs) const noexcept
	{
		if (ln < rhs.ln) {
			return false;
		}
		return (::strncmp(get_data(), rhs.get_data(), rhs.ln) == 0);
	}

	bool
	String::strncmp(const char *rhs) const noexcept
	{
		return (::strncmp(get_data(), rhs, strlen(rhs)) == 0);
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

		for (ssize_t pos = begin; pos < end; pos++) {
			if (ud[pos] <= 0x7f) {
				ret.push_back(ud[pos]);
			} else if (ud[pos] <= 0x07ff) {
				ret.push_back(0xc0 | ((ud[pos] >> 6) & 0x1f));
				ret.push_back(0x80 | ((ud[pos] >> 0) & 0x3f));
			} else if (ud[pos] <= 0xffff) {
				ret.push_back(0xe0 | ((ud[pos] >> 12) & 0x0f));
				ret.push_back(0x80 | ((ud[pos] >> 6) & 0x3f));
				ret.push_back(0x80 | ((ud[pos] >> 0) & 0x3f));
			} else if (ud[pos] <= 0x1fffff) {
				ret.push_back(0xf0 | ((ud[pos] >> 18) & 0x07));
				ret.push_back(0x80 | ((ud[pos] >> 12) & 0x3f));
				ret.push_back(0x80 | ((ud[pos] >> 6) & 0x3f));
				ret.push_back(0x80 | ((ud[pos] >> 0) & 0x3f));
			} else if (ud[pos] <= 0x3ffffff) {
				ret.push_back(0xf8 | ((ud[pos] >> 24) & 0x03));
				ret.push_back(0x80 | ((ud[pos] >> 18) & 0x3f));
				ret.push_back(0x80 | ((ud[pos] >> 12) & 0x3f));
				ret.push_back(0x80 | ((ud[pos] >> 6) & 0x3f));
				ret.push_back(0x80 | ((ud[pos] >> 0) & 0x3f));
			} else if (ud[pos] <= 0x7fffffff) {
				ret.push_back(0xfc | ((ud[pos] >> 30) & 0x01));
				ret.push_back(0x80 | ((ud[pos] >> 24) & 0x3f));
				ret.push_back(0x80 | ((ud[pos] >> 18) & 0x3f));
				ret.push_back(0x80 | ((ud[pos] >> 12) & 0x3f));
				ret.push_back(0x80 | ((ud[pos] >> 6) & 0x3f));
				ret.push_back(0x80 | ((ud[pos] >> 0) & 0x3f));
			}
		}

		return ret;
	}

	String
	String::cut(size_t begin, ssize_t end) const
	{
		String ret;

		if (begin > ln) {
			throw Error(String("String::cut begin beyond size"));
		}
		if (end > (ssize_t)ln) {
			throw Error(String("String::cut end beyond size"));
		}
		ret = get_data() + begin;
		if (end >= 0) {
			ret.get_data()[end - begin + 1] = '\0';
			ret.ln = strlen(ret.get_data());
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

		for (pos = get_data(); *pos != '\0'; pos++) {
			if (*pos == '"') {
				quote = !quote;
			} else {
				if (*pos == ';' && !quote) {
					ret[i++] = std::move(tmpres);
					tmpres = "";
				} else {
					tmpres.rebufsize(tmpres.ln + 2);
					tmpres.get_data()[tmpres.ln] = *pos;
					tmpres.ln++;
					tmpres.get_data()[tmpres.ln] = '\0';
				}
			}
		}
		ret[i++] = std::move(tmpres);

		return ret;
	}

	Array<String>
	String::linesplit() const
	{
		Array<String> ret;

		String buf = *this;
		String tmp;
		bool moredata = false;
		char *pos;
		char *lastpos;
		for (pos = lastpos = buf.get_data(); pos < (buf.get_data() + buf.ln); pos++) {
			if (pos[0] == '\n') {
				pos[0] = '\0';
				if (pos[1] == '\r') {
					pos[1] = '\0';
					pos++;
				}
				tmp = lastpos;
				lastpos = pos + 1;
				ret << tmp;
				moredata = false;
			} else if (pos[0] == '\r') {
				pos[0] = '\0';
				if (pos[1] == '\n') {
					pos[1] = '\0';
					pos++;
				}
				tmp = lastpos;
				lastpos = pos + 1;
				ret << tmp;
				moredata = false;
			} else {
				moredata = true;
			}
		}
		if (moredata) {
			tmp = lastpos;
			ret << lastpos;
		}
		return ret;
	}

	Array<String>
	String::strsplit(String delim) const
	{
		Array<String> ret;
		String tmp;
		char *match;
		char *start = (char*)get_data();
		char *oldstart;
		size_t length;

		do {
			match = strstr(start, delim.get_data());
			oldstart = start;
			if (match != NULL) {
				start = &match[delim.ln];
				length = match - oldstart;
			} else {
				length = get_data() + ln - oldstart;
			}
			tmp.bufsize(length + 1);
			for (size_t i = 0; i < length; i++) {
				tmp.get_data()[i] = oldstart[i];
			}
			tmp.get_data()[length] = '\0';
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
		tmp = copy.get_data();
		do {
			res = strsep(&tmp, delim.c_str());
			if (res != NULL) {
				ret[i] = res;
				i++;
			}
		} while (res != NULL);

		return ret;
	}

	Array<String>
	String::split(String delim, int64_t len) const
	{
		Array<String> ret;
		int i;
		char *res;
		char *tmp;
		String copy(*this);

		i = 0;
		tmp = copy.get_data();
		do {
			if (i + 1 == len && tmp != NULL) {
				ret[i] = tmp;
				res = NULL;
			} else {
				res = strsep(&tmp, delim.c_str());
				if (res != NULL) {
					ret[i] = res;
					i++;
				}
			}
		} while (res != NULL);

		return ret;
	}

	bool
	String::contains(const String& rhs) const noexcept
	{
		if (strstr(get_data(), rhs.get_data()) != NULL) {
			return true;
		}
		return false;
	}

	String
	String::trim() const noexcept
	{
		String ret;
		ssize_t first = 0;
		ssize_t last = ln - 1;

		for (ssize_t i = 0; i < (ssize_t)ln; i++) {
			if (get_data()[i] == ' ') {
				first = i + 1;
			} else {
				break;
			}
		}
		for (ssize_t i = ln - 1; i >= 0; i--) {
			if (get_data()[i] == ' ') {
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
	String::trimstart(const char value) const noexcept
	{
		String ret;
		ssize_t first = 0;
		ssize_t last = ln - 1;

		for (ssize_t i = 0; i < (ssize_t)ln; i++) {
			if (get_data()[i] == value) {
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
	String::trimend(const char value) const noexcept
	{
		String ret;
		ssize_t first = 0;
		ssize_t last = ln - 1;

		for (ssize_t i = ln - 1; i >= 0; i--) {
			if (get_data()[i] == value) {
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

		match = strstr(get_data(), search.get_data());
		if (match != NULL) {
			*match = '\0';
			tmp = get_data();
			tmp += replace;
			tmp += (match + search.ln);
			*this = std::move(tmp);
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

		match = strstr(get_data(), search.get_data());
		remaining = get_data();
		while (match != NULL) {
			matches++;
			*match = '\0';
			tmp += remaining;
			tmp += replace;
			remaining = match + search.ln;
			match = strstr(remaining, search.get_data());
		}
		if (matches > 0) {
			tmp += remaining;
			*this = std::move(tmp);
		}
		return matches;
	}

	double
	String::getd() const noexcept
	{
		double ret;

		ret = strtod(get_data(), NULL);
		return ret;
	}

	long long
	String::getll() const noexcept
	{
		long long ret;
		char* res;

		ret = strtoll(get_data(), &res, 10);
		if (res == get_data()) {
			//TError(S + "non numeric input data in" + *this);
			log(S + "non numeric input data in " + *this);
		}
		return ret;
	}

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
			aa_ptr<char> error;
			error = new char[err_bufsize];
			regerror(res, &rx, error.get(), err_bufsize);
			//regfree(&rx);
			throw Error(String(error.get()));
		}
		pmatch.rm_so = 0;
		pmatch.rm_eo = ln;
		res = regexec(&rx, get_data(), 0, &pmatch, 0);
		if (res == REG_NOMATCH) {
			regfree(&rx);
			return (*this);
		}

		if (res != 0) {
			aa_ptr<char> error;
			error = new char[err_bufsize];
			regerror(res, &rx, error.get(), err_bufsize);
			regfree(&rx);
			throw Error(String(error.get()));
		}

		const char *rp;
		const char *wp;
		VarPattern *pat;
		if (pat->matches[0].rm_so > 0) {
			for (int i = 0; i < pat->matches[0].rm_so; i++) {
				ret.push_back(wp[i]);
			}
		}
		for (rp = rh.c_str(); *rp; rp++) {
			if ((*rp == '\\') && ((rp[1] == '&') || (rp[1] == '\\'))) {
				ret.push_back(rp[i]);
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
						ret.push_back(subbuf[i]);
					}
				}
			} else {
				ret.push_back(*rp);
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
			aa_ptr<char> error;
			error = new char[err_bufsize];
			regerror(res, &rx, error.get(), err_bufsize);
			//regfree(&rx);
			throw Error(String(error.get()));
		}
		pmatch.rm_so = 0;
		pmatch.rm_eo = ln;
		res = regexec(&rx, get_data(), 0, &pmatch, 0);
		if (res != 0 && res != REG_NOMATCH) {
			aa_ptr<char> error;
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

	String
	String::printf(String format, ...)
	{
		va_list ap;
		free_data();
		char *tmp;
		va_start(ap, format);
		int res = vasprintf(&tmp, format.c_str(), ap);
		va_end(ap);
		if (res < 0) {
			throw std::bad_alloc();
		}
		ln = strlen(tmp);
		if ((ln + 1) <= directsize) {
			buflen = directsize;
			strcpy(sdata, tmp);
			free(tmp);
		} else {
			data = tmp;
			buflen = ln + 1;
		}
		return *this;
	}

	bool
	String::is_numeric() noexcept
	{
		for (size_t i = 0; i < ln; i++) {
			if (get_data()[i] < '0' || get_data()[i] > '9') {
				return false;
			}
		}
		return true;
	}

	void
	String::reverse()
	{
		String tmp;
		for (int64_t i = ln - 1, j = 0; i >= 0; i--, j++) {
			tmp.push_back(get_data()[i]);
		}
		(*this) = std::move(tmp);
	}

	bool
	String::test_utf() const
	{
		if (ln != 0) {
			for (uint64_t i = 0; i <= ln; i++) {
				int continuation = 0;
				if ((get_data()[i] & 0xe0) == 0xc0) {
					// 2 byte encoding
					continuation = 1;
				} else if ((get_data()[i] & 0xf0) == 0xe0) {
					// 3 byte encoding
					continuation = 2;
				} else if ((get_data()[i] & 0xf8) == 0xf0) {
					// 4 byte encoding
					continuation = 3;
				} else if (get_data()[i] & 0x80) {
					return false;
				}
				for (int j = 0; j < continuation; j++) {
					// test for continuation characters
					i++;
					if (i > ln || ((get_data()[i] & 0xc0) != 0x80)) {
						return false;
					}
				}
			}
		}
		return true;
	}

	SArray<uint8_t>
	String::hex_to_bytes() const
	{
		SArray<uint8_t> ret;

		for (size_t i = 0; (i + 1) < ln; i += 2) {
			uint8_t b = 0;
			char c;
			c = get_data()[i];
			if (c >= '0' && c <= '9') {
				b |= c - '0';
			} else if (c >= 'a' &&  c <= 'f') {
				b |= c - 'a' + 10;
			} else if (c >= 'A' &&  c <= 'F') {
				b |= c - 'A' + 10;
			}
			b <<= 4;
			c = get_data()[i + 1];
			if (c >= '0' && c <= '9') {
				b |= c - '0';
			} else if (c >= 'a' &&  c <= 'f') {
				b |= c - 'a' + 10;
			} else if (c >= 'A' &&  c <= 'F') {
				b |= c - 'A' + 10;
			}
			ret << b;
		}

		return ret;
	}

	String
	String::tinfo() const
	{
		String ret;
		ret << "(" << typeid(*this).name() << "@" << this << ")";
		return ret;
	}

	void
	String::log(int priority, const String& str) const noexcept
	{
		syslog(priority, "%s %s", str.c_str(), tinfo().c_str());
	}

	void
	String::log(int priority, const char *str) const noexcept
	{
		syslog(priority, "%s %s", str, tinfo().c_str());
	}

	void
	String::log(const String& str) const noexcept
	{
		log(LOG_DEBUG, str);
	}

	void
	String::log(const char *str) const noexcept
	{
		log(LOG_DEBUG, str);
	}

	std::ostream&
	operator<< (std::ostream& out, const String& rh)
	{
		out.write(rh.get_data(), rh.ln);
		return out;
	}

}
