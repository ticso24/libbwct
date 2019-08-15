/*
 * Copyright (c) 2001,02,03,08 Bernd Walter Computer Technology
 * Copyright (c) 2008 FIZON GmbH
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#include <bwct/bwct.h>
#include <sys/types.h>
//#include <machine/atomic.h>

Mutex fetch_mtx;

void
Base::check() const
{
	if (refcount < 0) {
		syslog(LOG_DEBUG, "refcount == %d %s", refcount, tinfo().c_str());
		abort();
	}
}

Base::Base ()
{

	refcount = 0;
//	log("create");
}

Base::~Base()
{

	check();
	// TODO we can't throw exceptions in a destructor
//	cassert(refcount == 0);
	refcount = -10;
//	log("destroy");
}

void
Base::log(int priority, const String& str) const
{
	syslog(priority, "%s %s", str.c_str(), tinfo().c_str());
}

void
Base::log(int priority, const char *str) const
{
	syslog(priority, "%s %s", str, tinfo().c_str());
}

void
Base::log(const String& str) const
{
	log(LOG_DEBUG, str);
}

void
Base::log(const char *str) const
{
	log(LOG_DEBUG, str);
}

String
Base::tinfo() const
{
	String ret;
	ret << "(" << typeid(*this).name() << "@" << this << " refcount=" << refcount << ")";
	return ret;
}

void
Base::addref()
{
	check();
	//atomic_add_int((volatile u_int*)&refcount, 1);
	refcount++;
	//syslog(LOG_DEBUG, "addref %s", tinfo().c_str());
}

void
Base::delref()
{
	check();
	int lastref;
	lastref = refcount;
	refcount--;
	//lastref = (int)atomic_fetchadd_int((volatile u_int*)&refcount, (u_int)-1);
	//syslog(LOG_DEBUG, "delref %s", tinfo().c_str());
	if (lastref == 1) {
		//syslog(LOG_DEBUG, "deleting %s by reference", tinfo().c_str());
		delete this;
	}
}

uint64_t
genid()
{
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
tohex(char *data, int size)
{
	String ret;
	char val[3];
	val[2] = '\0';
	for (int i = 0; i < size; i++) {
		int nibble;
		nibble = (data[i] & 0xf0) >> 4;
		val[0] = (nibble > 9) ? 'a' + nibble - 10 : '0' + nibble;
		nibble = data[i] & 0x0f;
		val[1] = (nibble > 9) ? 'a' + nibble - 10 : '0' + nibble;
		ret += val;
	}
	return ret;
}

MD5_Hash
getMD5(void* data, size_t length)
{
	class MD5_Hash hash;
	MD5_CTX context;
	MD5_Init(&context);
	MD5_Update(&context, data, length);
	MD5_Final(hash.buf, &context);
	return hash;
}

MD5_Hash
getMD5(const String& data)
{
	class MD5_Hash hash;
	MD5_CTX context;
	MD5_Init(&context);
	MD5_Update(&context, data.c_str(), data.length());
	MD5_Final(hash.buf, &context);
	return hash;
}

String
get_strhash(MD5_Hash hash)
{
	String ret;
	char tmp[2];
	tmp[1] = '\0';
	for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
		char nibble;
		nibble = hash.buf[i] >> 4;
		if (nibble > 9) {
			tmp[0] = 'a' + nibble - 10;
		} else {
			tmp[0] = '0' + nibble;
		}
		ret += tmp;
		nibble = hash.buf[i] & 0x0f;
		if (nibble > 9) {
			tmp[0] = 'a' + nibble - 10;
		} else {
			tmp[0] = '0' + nibble;
		}
		ret += tmp;
	}
	return ret;
}

String
get_base64hash(MD5_Hash hash)
{
	return base64_encode(hash.buf, MD5_DIGEST_LENGTH);
}

SHA1_Hash
getSHA1(void* data, size_t length)
{
	class SHA1_Hash hash;
	SHA_CTX context;
	SHA1_Init(&context);
	SHA1_Update(&context, data, length);
	SHA1_Final(hash.buf, &context);
	return hash;
}

SHA1_Hash
getSHA1(const String& data)
{
	class SHA1_Hash hash;
	SHA_CTX context;
	SHA1_Init(&context);
	SHA1_Update(&context, data.c_str(), data.length());
	SHA1_Final(hash.buf, &context);
	return hash;
}

String
get_base64hash(SHA1_Hash hash)
{
	return base64_encode(hash.buf, SHA_DIGEST_LENGTH);
}

String
get_strhash(SHA1_Hash hash)
{
	String ret;
	char tmp[2];
	tmp[1] = '\0';
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
		char nibble;
		nibble = hash.buf[i] >> 4;
		if (nibble > 9) {
			tmp[0] = 'a' + nibble - 10;
		} else {
			tmp[0] = '0' + nibble;
		}
		ret += tmp;
		nibble = hash.buf[i] & 0x0f;
		if (nibble > 9) {
			tmp[0] = 'a' + nibble - 10;
		} else {
			tmp[0] = '0' + nibble;
		}
		ret += tmp;
	}
	return ret;
}

uint64_t
getdate()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return LL(tv.tv_sec);
}

uint64_t
gettimesec(void)
{
	struct timeval tv;
	gettimeofday (&tv, NULL);
	return LL(tv.tv_sec);
}

String
sgethostname()
{
	Matrix<char> tmp(1025);
	if (gethostname(tmp.get(), 1025) < 0)
		throw Error("gethostname failed:");
	String hostname(tmp);
	return hostname;
}

static char
getbase64(uint8_t index)
{
	char ret = '=';
	if (index >= 64) {
		throw Error(S + "invalid base64 index" + index);
	}
	switch(index) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
			ret = 'A' + index;
			break;
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 40:
		case 41:
		case 42:
		case 43:
		case 44:
		case 45:
		case 46:
		case 47:
		case 48:
		case 49:
		case 50:
		case 51:
			ret = 'a' + (index - 26);
			break;
		case 52:
		case 53:
		case 54:
		case 55:
		case 56:
		case 57:
		case 58:
		case 59:
		case 60:
		case 61:
			ret = '0' + (index - 52);
			break;
		case 62:
			ret = '+';
			break;
		case 63:
			ret = '/';
			break;
	}
	return ret;
}

String
base64_encode(void* data, size_t length)
{
	String ret;
	char* ldata = (char*)data;
	char outbuf[] = "====";
	uint8_t input[3];
	int linelen = 0;
	for (size_t i = 0; i < length;) {
		if (i < length) {
			input[0] = ldata[i++];
			if (i >= length) {
				outbuf[0] = getbase64(input[0] >> 2);
				outbuf[1] = getbase64((input[0] & 0x3) << 4);
				outbuf[2] = '=';
				outbuf[3] = '=';
			}
		}
		if (i < length) {
			input[1] = ldata[i++];
			if (i >= length) {
				outbuf[0] = getbase64(input[0] >> 2);
				outbuf[1] = getbase64(((input[0] & 0x3) << 4) | input[1] >> 4);
				outbuf[2] = getbase64((input[1] & 0xf) << 2);
				outbuf[3] = '=';
			}
		}
		if (i < length) {
			input[2] = ldata[i++];
			outbuf[0] = getbase64(input[0] >> 2);
			outbuf[1] = getbase64(((input[0] & 0x3) << 4) | input[1] >> 4);
			outbuf[2] = getbase64(((input[1] & 0xf) << 2) | input[2] >> 6);
			outbuf[3] = getbase64(input[2] & 0x3f);
		}
		if ((linelen += 4) > 60) {
			linelen = 4;
			ret += "\n";
		}
		ret += outbuf;
		strcpy(outbuf, "====");
	}
	return ret;
}

uint16_t
fasthash(const String& key)
{
	uint16_t ret = 0;
	const char* v = key.c_str();
	while(*v != '\0') {
		char extra = (ret & 0x8000) ? 1 : 0;
		ret = ((ret << 1) ^ *v) | extra;
	}
	return ret;
}

uint8_t
nibbletobin(char rh)
{
	if (rh >= '0' && rh <= '9') {
		return (rh - '0');
	}
	if (rh >= 'a' && rh <= 'f') {
		return (rh - 'a' + 10);
	}
	if (rh >= 'A' && rh <= 'F') {
		return (rh - 'A' + 10);
	}
	return 0;
}

uint32_t
crc_hash(const void *key, uint32_t len, uint32_t hash)
{
	uint32_t  i;
	const uint8_t *k = (const uint8_t*)key;
	for (hash = len, i = 0; i < len; ++i) {
		hash = (hash >> 8) ^ crctab[(hash & 0xff) ^ k[i]];
	}
	return hash;
}

template class Array<String>;

