/*
 * Copyright (c) 2001,02 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#include "config.h"
#include "bsd.h"
#include "tool.h"

String::String() {
	data = (char*)malloc(1);
	if (data == NULL)
		throw std::bad_alloc();
	*data = '\0';
	ln = 0;
}

String::String(const Matrix<char>& rhs) {
	data = (char*)malloc(strlen(rhs.get()) + 1);
	if (data == NULL)
		throw std::bad_alloc();
	strcpy(data, rhs.get());
	ln = strlen(data);
}

String::String(const char* rhs) {
	data = (char*)malloc(strlen(rhs) + 1);
	if (data == NULL)
		throw std::bad_alloc();
	strcpy(data, rhs);
	ln = strlen(data);
}

String::String(const String &rhs) {
	data = (char*)malloc(rhs.ln + 1);
	if (data == NULL)
		throw std::bad_alloc();
	strcpy(data, rhs.data);
	ln = rhs.ln;
	cassert(ln == (int)strlen(data));
}

String::String(char rhs) {
	data = (char*)malloc(32);
	if (data == NULL)
		throw std::bad_alloc();
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}

String::String(short rhs) {
	data = (char*)malloc(32);
	if (data == NULL)
		throw std::bad_alloc();
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}

String::String(int rhs) {
	data = (char*)malloc(32);
	if (data == NULL)
		throw std::bad_alloc();
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}

String::String(long rhs) {
	data = (char*)malloc(32);
	if (data == NULL)
		throw std::bad_alloc();
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}

String::String(long long rhs) {
	data = (char*)malloc(32);
	if (data == NULL)
		throw std::bad_alloc();
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}

String::String(unsigned char rhs) {
	data = (char*)malloc(32);
	if (data == NULL)
		throw std::bad_alloc();
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}

String::String(unsigned short rhs) {
	data = (char*)malloc(32);
	if (data == NULL)
		throw std::bad_alloc();
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}

String::String(unsigned int rhs) {
	data = (char*)malloc(32);
	if (data == NULL)
		throw std::bad_alloc();
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}

String::String(unsigned long rhs) {
	data = (char*)malloc(32);
	if (data == NULL)
		throw std::bad_alloc();
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}

String::String(unsigned long long rhs) {
	data = (char*)malloc(32);
	if (data == NULL)
		throw std::bad_alloc();
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
}


String::String(const void *rhs) {
	data = (char*)malloc(32);
	if (data == NULL)
		throw std::bad_alloc();
	sprintf(data, "%p", rhs);
	ln = strlen(data);
}

const String&
String::operator= (const Matrix<char>& rhs) {
	if (data != NULL)
		free (data);
	data = (char*)malloc(strlen(rhs.get()) + 1);
	if (data == NULL)
		throw std::bad_alloc();
	strcpy(data, rhs.get());
	ln = strlen(data);
	return *this;
}

const String&
String::operator= (const String &rhs) {
	if (data != NULL)
		free (data);
	data = (char*)malloc(rhs.ln + 1);
	if (data == NULL)
		throw std::bad_alloc();
	strcpy(data, rhs.data);
	ln = rhs.ln;
	cassert(ln == (int)strlen(data));
	return *this;
}

const String&
String::operator= (const char *rhs) {
	if (data != NULL)
		free (data);
	data = (char*)malloc(strlen(rhs) + 1);
	if (data == NULL)
		throw std::bad_alloc();
	strcpy(data, rhs);
	ln = strlen(data);
	return *this;
}

const String&
String::operator= (unsigned int rhs) {
	if (data != NULL)
		free (data);
	data = (char*)malloc(32);
	if (data == NULL)
		throw std::bad_alloc();
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
	return *this;
}

const String&
String::operator= (unsigned long rhs) {
	if (data != NULL)
		free (data);
	data = (char*)malloc(32);
	if (data == NULL)
		throw std::bad_alloc();
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
	return *this;
}

const String&
String::operator= (unsigned long long rhs) {
	if (data != NULL)
		free (data);
	data = (char*)malloc(32);
	if (data == NULL)
		throw std::bad_alloc();
	sprintf(data, "%lld", LL(rhs));
	ln = strlen(data);
	return *this;
}

const String&
String::operator= (const void *rhs) {
	if (data != NULL)
		free (data);
	data = (char*)malloc(32);
	if (data == NULL)
		throw std::bad_alloc();
	sprintf(data, "%p", rhs);
	ln = strlen(data);
	return *this;
}

int
String::length() const {
	cassert(ln == (int)strlen(data));
	return ln;
}

const char *
String::c_str() const {
	return data;
}

String::~String() {
	if (data != NULL)
		free (data);
}

bool
String::operator== (const String &rhs) const {
	return (strcmp(data, rhs.data) == 0);
}

bool
String::operator== (const Matrix<char>& rhs) const {
	return (strcmp(data, rhs.get()) == 0);
}

bool
String::operator== (const char *rhs) const {
	return (strcmp(data, rhs) == 0);
}

bool
String::operator!= (const String &rhs) const {
	return (strcmp(data, rhs.data) != 0);
}

bool
String::operator!= (const Matrix<char>& rhs) const {
	return (strcmp(data, rhs.get()) != 0);
}

bool
String::operator!= (const char *rhs) const {
	return (strcmp(data, rhs) != 0);
}

String&
String::operator+= (const String &rhs) {
	data = (char*)realloc(data, ln + rhs.ln + 1);
	if (data == NULL)
		throw std::bad_alloc();
	strcpy(data + ln, rhs.data);
	ln += rhs.ln;
	return *this;
}

String&
String::operator+= (const Matrix<char>& rhs) {
	data = (char*)realloc(data, ln + strlen(rhs.get()) + 1);
	if (data == NULL)
		throw std::bad_alloc();
	strcpy(data + ln, rhs.get());
	ln += strlen(rhs.get());
	return *this;
}

String&
String::operator+= (const char *rhs) {
	data = (char*)realloc(data, ln + strlen(rhs) + 1);
	if (data == NULL)
		throw std::bad_alloc();
	strcpy(data + ln, rhs);
	ln += strlen(rhs);
	return *this;
}

String&
String::operator+=(char rhs) {
	return (*this) += String(rhs);
}

String&
String::operator+=(short rhs) {
	return (*this) += String(rhs);
}

String&
String::operator+=(int rhs) {
	return (*this) += String(rhs);
}

String&
String::operator+=(long rhs) {
	return (*this) += String(rhs);
}

String&
String::operator+=(long long rhs) {
	return (*this) += String(rhs);
}

String&
String::operator+=(unsigned char rhs) {
	return (*this) += String(rhs);
}

String&
String::operator+=(unsigned short rhs) {
	return (*this) += String(rhs);
}

String&
String::operator+=(unsigned int rhs) {
	return (*this) += String(rhs);
}

String&
String::operator+=(unsigned long rhs) {
	return (*this) += String(rhs);
}

String&
String::operator+=(unsigned long long rhs) {
	return (*this) += String(rhs);
}

String&
String::operator+=(const void *rhs) {
	return (*this) += String(rhs);
}


void
String::resize(int rhs) {
	if (rhs < ln) {
		data[rhs] = 0;
		ln = rhs;
	}
}

bool
String::empty() const {
	return (ln == 0);
}

const char *
String::operator[] (const int i) const {
	cassert(i <= (int)ln);
	return &data[i];
}

void
String::lower() {
	for (int i = 0; i < ln; i++)
		data[i] = tolower(data[i]);
}

void
String::upper() {
	for (int i = 0; i < ln; i++)
		data[i] = toupper(data[i]);
}

void
Base::log(const String& str) {
	syslog(LOG_DEBUG, "%s %s", str.c_str(), tinfo().c_str());
}

String
Base::tinfo() const {
	String ret;
	ret << "(" << typeid(*this).name() << "@" << this << ")";
	return ret;
}

void
Base::addref() {
	check();
	refcount++;
	syslog(LOG_DEBUG, "%s refno=%d", tinfo().c_str(), refcount);
}

void
Base::delref() {
	check();
	syslog(LOG_DEBUG, "%s refno=%d", tinfo().c_str(), refcount - 1);
	if (!(--refcount))
		delete this;
}

uint64_t
genid() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	static uint64_t lastid;
	uint64_t ret = (LL(tv.tv_sec) << 32) + tv.tv_usec;
	if (ret == lastid)
		ret++;
	lastid = ret;
	return ret;
}

String
tohex(char *data, int size) {
	String ret;
	int nibble;
	char val[3];
	val[2] = '\0';
	for (i = 0; i < size; i++) {
		nibble = (data[i] & 0xf0) >> 4;
		val[0] = (nibble > 9) ? 'a' + nibble - 10 : '0' + nibble;
		nibble = data[i] & 0x0f;
		val[1] = (nibble > 9) ? 'a' + nibble - 10 : '0' + nibble;
		ret += val;
	}
}

String
genmd5id() {
	uint64_t id = genid(); 
	MD5_CTX ctx;
	MD5Init(&ctx);
	MD5Update(&ctx, (char*)&id, sizeof(id));
	char digest[16];
	MD5Final(digest, &ctx);
	return tohex(digest, 16);
}

uint64_t
getdate() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return LL(tv.tv_sec);
}

uint64_t
gettimesec(void) {
        struct timeval tv;
	gettimeofday (&tv, NULL);
	return LL(tv.tv_sec);
}

String
sgethostname() {
	Matrix<char> tmp(1025);
	if (gethostname(tmp.get(), 1025) < 0)
		throw Error("gethostname failed:");
	String hostname(tmp);
	return hostname;
}

