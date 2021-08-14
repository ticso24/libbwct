/*
 * Copyright (c) 2001,02,03,04,08,09,10 Bernd Walter Computer Technology
 * Copyright (c) 2008,09,10 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/string.h $
 * $Date: 2019-09-16 14:32:28 +0200 (Mon, 16 Sep 2019) $
 * $Author: ticso $
 * $Rev: 40426 $
 */

#ifndef _STRING
#define _STRING

#define S String()
#define M_STR(str) #str
#define MSTR(str) M_STR(str)

class JSON;

class String : public Base {
private:
	size_t ln;
	size_t buflen;
	char *data;
	uint32_t *udata;
	void rebufsize(size_t len);
	void bufsize(size_t len);
public:
	String();
	String(const char *rhs);
	String(const String &rhs);
	String(String &&rhs);
	String(char rhs);
	String(short rhs);
	String(int rhs);
	String(long rhs);
	String(long long rhs);
	String(bool rhs);
	String(unsigned char rhs);
	String(unsigned short rhs);
	String(unsigned int rhs);
	String(unsigned long rhs);
	String(unsigned long long rhs);
	String(double rhs);
	String(const void *rhs);
	String(File rhs);
	String(const JSON&  rhs);
	String(const Array<String>& rhs);
	void add_memory(void* data, size_t len);
	void getURL(const String& URL, const String& source);
	const String& operator= (const String &rhs);
	const String& operator= (String &&rhs);
	const String& operator= (const Array<String>& rhs);
	const String& operator= (char *rhs);
	const String& operator= (const char *rhs);
	const String& operator= (bool rhs);
	const String& operator= (unsigned int rhs);
	const String& operator= (unsigned long rhs);
	const String& operator= (unsigned long long rhs);
	const String& operator= (double rhs);
	const String& operator= (const void *rhs);
	const String& operator= (File rhs);
	const String& operator= (const JSON& rhs);
	template <class T>
	const String& operator=(const T &rhs);
	const String& join (const Array<String>& rhs, const String &bind);
	size_t length() const;
	size_t u_length() const;
	const char *c_str() const;
	const uint32_t *u_str();
	~String();
	bool operator== (const String &rhs) const;
	bool operator== (const char *rhs) const;
	bool operator!= (const String &rhs) const;
	bool operator!= (const char *rhs) const;
	template <class T>
	String operator+(const T &rhs) const;
	String operator+(const String &rhs) const;
	String operator+(String &&rhs) const;
	template <class T>
	String& operator<<(const T &rhs);
	bool operator< (const String &rhs) const;
	bool operator<= (const String &rhs) const;
	bool operator> (const String &rhs) const;
	bool operator>= (const String &rhs) const;
	String& operator+=(const String &rhs);
	String& operator+=(String &&rhs);
	String& operator+=(const Array<String>& rhs);
	String& operator+=(const char *rhs);
	String& operator+=(bool rhs);
	String& operator+=(char rhs);
	String& operator+=(short rhs);
	String& operator+=(int rhs);
	String& operator+=(long rhs);
	String& operator+=(long long rhs);
	String& operator+=(unsigned char rhs);
	String& operator+=(unsigned short rhs);
	String& operator+=(unsigned int rhs);
	String& operator+=(unsigned long rhs);
	String& operator+=(unsigned long long rhs);
	String& operator+=(double rhs);
	String& operator+=(const void *rhs);
	String trim() const;
	String trimstart(const char value) const;
	String trimend(const char value) const;
	void resize(size_t rhs);
	bool empty() const;
	void lower();
	void upper();
	bool strncmp(const String &rhs, size_t len) const;
	bool strncmp(const String &rhs) const;
	bool strncmp(const char *rhs) const;
	bool strncmp(int64_t frompos, const String &rhs) const;
	bool replacefirst(const String &search, const String &replace);
	bool contains(const String &rhs) const;
	int replace(const String &search, const String &replace);
	String cut(size_t begin, ssize_t end = -1) const;
	String u_cut(size_t begin, ssize_t end = -1) const;
	Array<String> split(String delim) const;
	Array<String> split(String delim, int64_t len) const;
	Array<String> strsplit(String delim) const;
	Array<String> CSVsplit() const;
	Array<String> linesplit() const;
	long long getll() const;
	double getd() const;
#if 0
	String re_subst(const String& re);
	bool re_comp(const String& re) const;
#endif
	String printf(String format, ...);
	bool is_numeric();
	void reverse();
	enum class Type_Enum {
		plain,
		xml
	};
	Type_Enum type;
	size_t begin() const;
	size_t end() const;
	const char& operator[](const size_t i) const;
	char& operator[](const size_t i);
	bool test_utf() const;
};

template <class T>
String&
String::operator<<(const T &rhs) {
	*this += String(rhs);
	return *this;
}

template <class T>
String
String::operator+(const T &rhs) const {
	String nstr(*this);
	nstr += rhs;
	return nstr;
}

template <class T>
const String&
String::operator=(const T& rhs) {
	const char* rstr = rhs.c_str();

	if (rstr == NULL) {
		log ("rstr == NULL");
		rstr = "(null)";
	}
	char *tmpdata;
	tmpdata = data;
	data = NULL;
	bufsize(strlen(rstr) + 1);
	strcpy(data, rstr);
	ln = strlen(data);
	if (tmpdata != NULL) {
		free (tmpdata);
	}
	return *this;
}

#endif /* !_STRING */
