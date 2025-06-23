/*
 * Copyright (c) 2001,02,03,08 Bernd Walter Computer Technology
 * Copyright (c) 2008 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/tool.cc $
 * $Date: 2025-06-22 18:40:52 +0200 (Sun, 22 Jun 2025) $
 * $Author: ticso $
 * $Rev: 49411 $
 */

#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "bwct.h"

namespace bwct
{
	void
	print_rusage()
	{
		struct rusage ru;
		int res = getrusage(RUSAGE_SELF, &ru);
		if (res < 0) {
			throw Error(S + "getrusage failed " + get_strerror(errno));
		}
		String msg;
		msg += S + "ru_maxrss: " + (ru.ru_maxrss / 1024) + " ";
		msg += S + "ru_ixrss: " + (ru.ru_ixrss / 1024) + " ";
		msg += S + "ru_idrss: " + (ru.ru_idrss / 1024) + " ";
		msg += S + "ru_isrss: " + (ru.ru_isrss / 1024) + " ";

		syslog(LOG_DEBUG, "%s", msg.c_str());
	}

	void
	Base::check() const
	{
	}

	Base::~Base() noexcept
	{

		check();
	//	log("destroy");
	}

	void
	Base::log(int priority, const String& str) const noexcept
	{
		syslog(priority, "%s %s", str.c_str(), tinfo().c_str());
	}

	void
	Base::log(int priority, const char *str) const noexcept
	{
		syslog(priority, "%s %s", str, tinfo().c_str());
	}

	void
	Base::log(const String& str) const noexcept
	{
		log(LOG_DEBUG, str);
	}

	void
	Base::log(const char *str) const noexcept
	{
		log(LOG_DEBUG, str);
	}

	String
	Base::tinfo() const
	{
		String ret;
		ret << "(" << typeid(*this).name() << "@" << this << ")";
		return ret;
	}

	uint64_t
	genid()
	{
		struct timespec tp;
		clock_gettime(CLOCK_REALTIME_FAST, &tp);
		static uint64_t lastid;
		uint64_t ret = (LL(tp.tv_sec) << 32) + (tp.tv_nsec / 1000);
		if (ret == lastid)
			ret++;
		lastid = ret;
		return ret;
	}

	String
	tohex(char *data, int size)
	{
		String ret;
		for (int i = 0; i < size; i++) {
			int nibble;
			nibble = (data[i] & 0xf0) >> 4;
			ret.push_back((nibble > 9) ? 'a' + nibble - 10 : '0' + nibble);
			nibble = data[i] & 0x0f;
			ret.push_back((nibble > 9) ? 'a' + nibble - 10 : '0' + nibble);
		}
		return ret;
	}

#ifdef HAVE_OPENSSL
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
#endif

	String
	get_strhash(MD5_Hash hash)
	{
		String ret;
		for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
			char nibble;
			nibble = hash.buf[i] >> 4;
			if (nibble > 9) {
				ret.push_back('a' + nibble - 10);
			} else {
				ret.push_back('0' + nibble);
			}
			nibble = hash.buf[i] & 0x0f;
			if (nibble > 9) {
				ret.push_back('a' + nibble - 10);
			} else {
				ret.push_back('0' + nibble);
			}
		}
		return ret;
	}

	String
	get_base64hash(MD5_Hash hash)
	{
		return base64_encode(hash.buf, MD5_DIGEST_LENGTH);
	}

#ifdef HAVE_OPENSSL
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
#endif

	String
	get_base64hash(SHA1_Hash hash)
	{
		return base64_encode(hash.buf, SHA_DIGEST_LENGTH);
	}

	String
	get_strhash(SHA1_Hash hash)
	{
		String ret;
		for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
			char nibble;
			nibble = hash.buf[i] >> 4;
			if (nibble > 9) {
				ret.push_back('a' + nibble - 10);
			} else {
				ret.push_back('0' + nibble);
			}
			nibble = hash.buf[i] & 0x0f;
			if (nibble > 9) {
				ret.push_back('a' + nibble - 10);
			} else {
				ret.push_back('0' + nibble);
			}
		}
		return ret;
	}

#ifdef HAVE_OPENSSL
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	String
	get_strhmac256(const String& key, const String& data)
	{
		String ret;

		unsigned char* res;
		unsigned int len = EVP_MAX_MD_SIZE;
		res = (unsigned char*)malloc(sizeof(char) * len);

		HMAC_CTX ctx;
		HMAC_CTX_init(&ctx);

		HMAC_Init_ex(&ctx, key.c_str(), key.length(), EVP_sha256(), NULL);
		HMAC_Update(&ctx, (unsigned char*)data.c_str(), data.length());
		HMAC_Final(&ctx, res, &len);
		HMAC_CTX_cleanup(&ctx);

		for (unsigned int h = 0; h != len; h++) {
			String hex;
			hex.printf("%02x", (unsigned int)res[h]);
			ret += hex;
		}
		free(res);

		return ret;
	}
#else
	String
	get_strhmac256(const String& key, const String& data)
	{
		String ret;

		unsigned char* res;
		unsigned int len = EVP_MAX_MD_SIZE;
		res = (unsigned char*)malloc(sizeof(char) * len);

		HMAC_CTX *ctx;
		ctx = HMAC_CTX_new();

		HMAC_Init_ex(ctx, key.c_str(), key.length(), EVP_sha256(), NULL);
		HMAC_Update(ctx, (unsigned char*)data.c_str(), data.length());
		HMAC_Final(ctx, res, &len);
		HMAC_CTX_free(ctx);

		for (unsigned int h = 0; h != len; h++) {
			String hex;
			hex.printf("%02x", (unsigned int)res[h]);
			ret += hex;
		}
		free(res);

		return ret;
	}
#endif
#endif

	uint64_t
	gettimesec(void)
	{
		struct timespec tp;
		clock_gettime(CLOCK_REALTIME_FAST, &tp);
		return LL(tp.tv_sec);
	}

	String
	sgethostname()
	{
		char tmp[MAXHOSTNAMELEN + 1];
		if (gethostname(tmp, MAXHOSTNAMELEN + 1) < 0)
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
	fasthash(const String& key) noexcept
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
	nibbletobin(char rh) noexcept
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

	double
	getload()
	{
		const int e = 1;
		double avenrun[e];

		if (getloadavg(avenrun, e) == -1) {
			throw Error(S + "no load average information available");
		}
		return  avenrun[0];
	}

	void
	call_external(Array<String>& args, bool dontwait)
	{
		struct sigaction sa;
		struct sigaction osa;
		sa.sa_handler = SIG_DFL;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		if (sigaction(SIGCHLD, &sa, &osa) != 0) {
			throw Error(S + "sigaction failed " + get_strerror(errno));
		}
		try {

			String path = args[0];

			String x;

			std::unique_ptr<char*[]> argv(new char*[args.max + 2]);
			for (int64_t i = 0; i <= args.max; i++) {
				argv[i] = (char*) args[i].c_str();
				x += args[i] + " ";
				argv[i + 1] = NULL;
			}

			{
				String logstr;
				logstr = S + "exec: " + x;
				syslog(LOG_DEBUG, "%s", logstr.c_str());
			}

			pid_t child = fork();
			if (child == 0) { // are we the child?
				closefrom(3);
				execv(path.c_str(), argv.get());

				// if we are still here something with exec went wrong
				_exit(-1);
			} else if (child != -1) { // are we the parent?
				// wait for child to complete
				int status;
				pid_t res;
				do {
					res = wait4(child, &status, dontwait ? WNOHANG : 0, NULL);
				} while (res == -1 && (errno == EAGAIN || errno == EINTR));
				if (res == -1) {
					throw Error(S + "waiting for child failed " + get_strerror(errno));
				}
				if (status != 0) {
					throw Error(S + "child returned status " + status);
				}
			} else {
				throw Error(S + "fork failed " + get_strerror(errno));
			}
		} catch (...) {
			sigaction(SIGCHLD, &osa, NULL);
			throw;
		}
		sigaction(SIGCHLD, &osa, NULL);
	}

	String
	get_strerror(int num)
	{
		String ret;
		char ebuf[NL_TEXTMAX];

		if (strerror_r(num, ebuf, sizeof(ebuf)) != 0) {
			ret = "invalid errno";
		}
		ret = ebuf;
		return ret;
	}

	static int randomdev = -1;

	uint64_t
	getrandom()
	{
		uint64_t ret;

		if (randomdev < 0) {
			randomdev = open("/dev/random", O_RDONLY);
			if (randomdev < 0) {
				throw Error(S + "open /dev/random failed " + get_strerror(errno));
			}
		}
		read(randomdev, &ret, sizeof(ret));
		return ret;
	}

	String
	getrandomAlNum(size_t length)
	{
		String result;
		while (result.length() < length) {
			uint64_t rnd = getrandom() % 122;
			if ((rnd >= '0' && rnd <= '9') ||
			    (rnd >= 'a' && rnd <= 'z') ||
			    (rnd >= 'A' && rnd <= 'Z')) {
				String tmp = result;
				result.printf("%s%c", tmp.c_str(), rnd);
			}
		}
		return result;
	}

	String
	getrandomAlpha(size_t length)
	{
		String result;
		while (result.length() < length) {
			uint64_t rnd = getrandom() % 122;
			if ((rnd >= 'a' && rnd <= 'z') ||
			    (rnd >= 'A' && rnd <= 'Z')) {
				String tmp = result;
				result.printf("%s%c", tmp.c_str(), rnd);
			}
		}
		return result;
	}

	static Mutex crypt_mtx;
	const static u_int8_t Base64Code[] =
	    "./ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

	String
	pw_crypt(const String& pw)
	{
		String ret;

		String salt = "$2b$06$"; // select blowfish with cost 6 (range 4-31)
		{
			// generate random base64 data to be used as salt
			for (int i = 0; i < 22; i++) {
				int rnd = getrandom() & 0x3f;
				salt.push_back(Base64Code[rnd]);
			}
		}

		crypt_mtx.lock();
		char* tmphash;
		tmphash = crypt(pw.c_str(), salt.c_str());
		if (tmphash != NULL) {
			ret = tmphash;
		}
		crypt_mtx.unlock();
		xassert(tmphash != NULL);
		return ret;
	}

	bool
	pw_crypt_compare(const String& pw, const String& hash)
	{
		String newhash;
		crypt_mtx.lock();
		char* tmphash;
		tmphash = crypt(pw.c_str(), hash.c_str());
		if (tmphash != NULL) {
			newhash = tmphash;
		}
		crypt_mtx.unlock();
		xassert(tmphash != NULL);
		return (hash == newhash);
	}

	String
	XML_ESC(const String &lh, bool text)
	{
		String ret;
		const char* plh;
		char tmp;

		plh = lh.c_str();
		for (size_t i = 0; i < lh.length(); i++) {
			tmp = plh[i];
			switch (tmp) {
			case ' ':
				if (text) {
					if (plh[i + 1] == ' ' || (i != 0 && plh[i -1] == ' ')) {
						ret += "&nbsp;";
					} else {
						ret += " ";
					}
				} else {
					ret += " ";
				}
				break;
			case '<':
				ret += "&lt;";
				break;
			case '>':
				ret += "&gt;";
				break;
			case 0x01:	// (SOH) Start Of Header - Firefox complains even if it is escaped - sighXXL
			case 0x02:	// just in case some broken browser complains...
			case 0x03:	// just in case some broken browser complains...
			case 0x04:	// just in case some broken browser complains...
			case 0x05:	// ENQ enquiry - some Browser complain even if it is escaped - sighXXL
			case 0x06:	// (ACK) Acknowledge - Firefox complains even if it is escaped - sighXXL
			case 0x07:	// just in case some broken browser complains...
			case 0x08:	// just in case some broken browser complains...
			case 0x0c:	// just in case some broken browser complains...
			case 0x0e:	// just in case some broken browser complains...
			case 0x0f:	// just in case some broken browser complains...
			case 0x09:	// just in case some broken browser complains...
			case 0x10:	// just in case some broken browser complains...
			case 0x11:	// (HT) Horizontal Tab - Firefox complains even if it is escaped - sighXXL
			case 0x12:	// just in case some broken browser complains...
			case 0x13:	// Device Control 3 - all supported browsers complain
			case 0x14:	// just in case some broken browser complains...
			case 0x15:	// just in case some broken browser complains...
			case 0x16:	// just in case some broken browser complains...
			case 0x17:	// just in case some broken browser complains...
			case 0x18:	// just in case some broken browser complains...
			case 0x19:	// just in case some broken browser complains...
			case 0x1a:	// just in case some broken browser complains...
			case 0x1b:	// just in case some broken browser complains...
			case 0x1c:	// just in case some broken browser complains...
			case 0x1d:	// just in case some broken browser complains...
			case 0x1e:	// just in case some broken browser complains...
			case 0x1f:	// just in case some broken browser complains...
			case '\v':	// (VT) vertical tab - Firefox complains if not escaped - sigh
				break;
			case '"':	// double quotes
			case '&':	// ampersant
			case '\'':	// single quotes
			case '\n':	// (NL) newline
			case '\r':	// (CR) carriage return
				ret += "&#";
				ret += (unsigned int) (unsigned char) tmp;
				ret += ";";
				break;
			default:
				ret.push_back(tmp);
				break;
			}
		}
		return ret;
	}

}
