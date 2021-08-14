/*
 * Copyright (c) 2001,02,03,08 Bernd Walter Computer Technology
 * Copyright (c) 2008 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/tool.cc $
 * $Date: 2021-07-20 16:09:46 +0200 (Tue, 20 Jul 2021) $
 * $Author: ticso $
 * $Rev: 44547 $
 */

#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "bwct.h"

Mutex fetch_mtx;

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
	abort_assert(refcount == 0);
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
	refcount++;
	//syslog(LOG_DEBUG, "addref %s", tinfo().c_str());
}

void
Base::delref()
{
	check();
	//syslog(LOG_DEBUG, "delref %s", tinfo().c_str());
	if (--refcount == 0) {
		//syslog(LOG_DEBUG, "deleting %s by reference", tinfo().c_str());
		delete this;
	}
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
	a_ptr<char> tmp;
	tmp = new char[MAXHOSTNAMELEN + 1];
	if (gethostname(tmp.get(), MAXHOSTNAMELEN + 1) < 0)
		throw Error("gethostname failed:");
	String hostname(tmp.get());
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

static const uint32_t crctab[256] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

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

		char* argv[args.max + 2];
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
			execv(path.c_str(), (char**) &argv);

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
			result.printf("%s%c", result.c_str(), rnd);
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
			result.printf("%s%c", result.c_str(), rnd);
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
		char rndstr[2] = {'.', '\0'};
		for (int i = 0; i < 22; i++) {
			int rnd = getrandom() & 0x3f;
			rndstr[0] = Base64Code[rnd];
			salt += rndstr;
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
	if (lh.type == String::Type_Enum::xml) {
		return lh;
	}
	char buf[2];
	String ret;
	const char* plh;
	char tmp;

	buf[1] = '\0';
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
		case 0x09:	// just in case some broken browser complains...
		case 0x10:	// just in case some broken browser complains...
			ret += "&#";
			ret += (unsigned int) (unsigned char) tmp;
			ret += ";";
			break;
		default:
			buf[0] = tmp;
			ret += buf;
			break;
		}
	}
	ret.type = String::Type_Enum::xml;
	return ret;
}

