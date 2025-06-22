/*
 * Copyright (c) 2001,02,03,04,08 Bernd Walter Computer Technology
 * Copyright (c) 2008 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/tool.h $
 * $Date: 2025-06-22 18:40:52 +0200 (Sun, 22 Jun 2025) $
 * $Author: ticso $
 * $Rev: 49411 $
 */

#ifndef _TOOL
#define _TOOL

#include <pthread_np.h>

namespace bwct
{

	class Base;
	class String;
	class Mutex;

	template <class T>
	class Array;

	template <class T>
	class SArray;

	template <class T>
	class List;

#ifndef BWCT_DEBUG
	#define dbg_beep()
	#define dbg_beepme()
	#define wassert(test)
	#define cassert(test)
	#define cassertm(test, msg)
#endif /* !BWCT_DEBUG */

	#define dbg_stacksize() {									\
		pthread_attr_t attr;									\
		pthread_attr_init(&attr);								\
		pthread_attr_get_np(pthread_self(), &attr);						\
		size_t guardsize;									\
		pthread_attr_getguardsize(&attr, &guardsize);						\
		size_t stacksize;									\
		void* stackbase;									\
		pthread_attr_getstack(&attr, &stackbase, &stacksize);					\
		pthread_attr_destroy(&attr);								\
		void* stackpointer = &stackbase;							\
		size_t stackleft = (size_t)((char*)stackpointer - (char*)stackbase);			\
		if (stackleft < 1000)									\
			abort();									\
		syslog (LOG_DEBUG, "dbg_stacksize: %p stacksize=%lld guardsize=%lld stackleft=%lld stackbase=%p stackpointer=%p %s@%d in %s",			\
		    pthread_self(), (long long)stacksize, (long long)guardsize, (long long)stackleft, stackbase, stackpointer, __FILE__, __LINE__, __func__);	\
	}

	#define dbg_stacksizeme() {									\
		pthread_attr_t attr;									\
		pthread_attr_init(&attr);								\
		pthread_attr_get_np(pthread_self(), &attr);						\
		size_t guardsize;									\
		pthread_attr_getguardsize(&attr, &guardsize);						\
		size_t stacksize;									\
		void* stackbase;									\
		pthread_attr_getstack(&attr, &stackbase, &stacksize);					\
		pthread_attr_destroy(&attr);								\
		void* stackpointer = &stackbase;							\
		size_t stackleft = (size_t)((char*)stackpointer - (char*)stackbase);			\
		if (stackleft < 1000)									\
			abort();									\
		syslog (LOG_DEBUG, "dbg_stacksize: %p stacksize=%lld guardsize=%lld stackleft=%lld stackbase=%p stackpointer=%p %s %s@%d in %s",			\
		    pthread_self(), (long long)stacksize, (long long)guardsize, (long long)stackleft, stackbase, stackpointer, tinfo().c_str(), __FILE__, __LINE__, __func__);	\
	}

#ifdef BWCT_DEBUG
	#define dbg_beep() {							\
		syslog (LOG_DEBUG, "dbg_beep: %p %s@%d in %s",			\
		    pthread_self(), __FILE__, __LINE__, __func__);		\
	}

	#define dbg_beepme() {								\
		syslog (LOG_DEBUG, "dbg_beep: %p %s %s@%d in %s",			\
		    pthread_self(), tinfo().c_str(), __FILE__, __LINE__, __func__);	\
	}

	#define wassert(test)							\
	if (!(test)) {								\
		syslog (LOG_CRIT,						\
		    "assertion (%s) failed: %s@%d in %s",			\
		    #test, __FILE__, __LINE__, __func__);			\
	}

#endif /* BWCT_DEBUG */

#ifdef FREEDEBUG
	#define free(ptr);							\
		syslog (LOG_DEBUG,						\
		    "free(%s) called: %s@%d ind %s ptr=%p",			\
			#ptr, __FILE__, __LINE__, __func__, ptr);		\
		free(ptr);
#endif /* FREEDEBUG */

	#define TError(str)							\
		throw Error(tinfo() + ": " + str + " : " +			\
		    __FILE__ + "@" + __LINE__ + " in " + __func__)

	#define XError(str)							\
		throw Error(S + str + " : " +					\
		    __FILE__ + "@" + __LINE__ + " in " + __func__)


	#define LL(v) ((long long)(v))

	#ifndef MIN
	# define MIN(a, b) ((a < b) ? a : b)
	#endif

	#ifndef MAX
	# define MAX(a, b) ((a < b) ? b : a)
	#endif

	void print_rusage();

	template<typename T>
	inline void palign(T& ptr, int len) {
		ptr = (T)((unsigned long)ptr + (len-1) & ~(len-1));
	}

	/* TODO: make use of OS internal for speedup */
	inline void cbswap64(uint64_t& data) {
		char tmp;
		char *tmp2;

		tmp2 = (char*)&data;
		tmp = tmp2[0]; tmp2[0] = tmp2[7]; tmp2[7] = tmp;
		tmp = tmp2[1]; tmp2[1] = tmp2[6]; tmp2[6] = tmp;
		tmp = tmp2[2]; tmp2[2] = tmp2[5]; tmp2[5] = tmp;
		tmp = tmp2[3]; tmp2[3] = tmp2[4]; tmp2[4] = tmp;
	}

	uint64_t getrandom();
	String getrandomAlNum(size_t length);
	String getrandomAlpha(size_t length);

	class Base {
	private:
		volatile int refcount;
	public:
		void log(int priority, const String& str) const noexcept;
		void log(int priority, const char *str) const noexcept;
		void log(const String& str) const noexcept;
		void log(const char *str) const noexcept;
		virtual void check() const;
		Base() noexcept { refcount = 0; };
		Base(const Base& rh) noexcept { refcount = 0; };
		Base(Base&& rh) noexcept { refcount = 0; };
		Base& operator=(const Base& rh) noexcept { return *this; };
		Base& operator=(Base&& rh) noexcept { return *this; };
		virtual ~Base() noexcept;
		void addref() noexcept;
		void delref() noexcept;
		int getref() const noexcept
		{
			return refcount;
		}
		virtual String tinfo() const;
	};

}

#include "mstring.h"

namespace bwct
{

	class Error : public std::exception {
	public:
		String msg;
		Error(const char* msg, bool log = true) {
			(this)->msg = msg;
#ifdef BWCT_DEBUG
			if (log) {
				syslog(LOG_INFO, "Error thrown: %s", msg);
			}
#endif
		}
		Error(const String& msg = "", bool log = true) {
			(this)->msg = msg;
#ifdef BWCT_DEBUG
			if (log) {
				syslog(LOG_INFO, "Error thrown: %s", msg.c_str());
			}
#endif
		}
		Error(const Error& rh) {
			msg = rh.msg;
		}
		Error(Error&& rh) noexcept {
			msg = std::move(rh.msg);
		}
		Error& operator= (const Error& rh) {
			msg = rh.msg;
			return *this;
		}
		Error& operator= (Error&& rh) {
			msg = std::move(rh.msg);
			return *this;
		}
		~Error() throw() {
		}
		virtual const char* what () const throw() {
			return msg.c_str();
		}
	};

#ifdef BWCT_DEBUG
	#define abort_assert(test)						\
	if (!(test)) {								\
		syslog (LOG_EMERG,						\
		    "assertion (%s) failed: %s@%d in %s",			\
		    #test, __FILE__, __LINE__, __func__);			\
		abort();							\
	}
#endif

#ifdef BWCT_DEBUG
#ifdef ASSERT_CORE
	#define cassert(test)							\
	if (!(test)) {								\
		syslog (LOG_EMERG,						\
		    "assertion (%s) failed: %s@%d in %s",			\
		    #test, __FILE__, __LINE__, __func__);			\
		abort();							\
	}
	#define cassertm(test, msg)						\
	if (!(test)) {								\
		syslog (LOG_EMERG,						\
		    "assertion (%s) [%s] failed: %s@%d in %s",			\
		    #test, msg, __FILE__, __LINE__, __func__);			\
		abort();							\
	}
#else
	#define cassert(test)							\
	if (!(test)) {								\
		String inf = tinfo();						\
		syslog (LOG_EMERG,						\
		    "assertion (%s) failed: %s@%d in %s for %s",		\
		    #test, __FILE__, __LINE__, __func__, inf.c_str());		\
		String err = String ("assertion (") + #test			\
		    + ") failed " + __FILE__ + "@" + __LINE__			\
		    + " in " + __func__ + inf;					\
		throw Error(err);						\
	}
	#define cassertm(test, msg)						\
	if (!(test)) {								\
		String inf = tinfo();						\
		syslog (LOG_EMERG,						\
		    "assertion (%s) [%s] failed: %s@%d in %s for %s",		\
		    #test, msg, __FILE__, __LINE__, __func__, inf.c_str());	\
		String err = String ("assertion (") + #test			\
		    + ") failed " + __FILE__ + "@" + __LINE__			\
		    + " in " + __func__ + inf;					\
		throw Error(err);						\
	}
#endif /* ASSERT_CORE */
#endif /* BWCT_DEBUG */

	#define xassert(test)							\
	if (!(test)) {								\
		syslog (LOG_EMERG,						\
		    "assertion (%s) failed: %s@%d in %s",			\
		    #test, __FILE__, __LINE__, __func__);			\
		String err = String ("assertion (") + #test			\
		    + ") failed " + __FILE__ + "@" + __LINE__			\
		    + " in " + __func__;					\
		throw Error(err);						\
	}

	template <class T>
	class a_ptr : public Base {
	protected:
		T& operator[](int i) noexcept;
	private:
		T* ptr;
	public:
		a_ptr() noexcept {
			ptr = NULL;
		}
		a_ptr(const a_ptr& rh) = delete;
		a_ptr(a_ptr&& rh) noexcept {
			ptr = rh.ptr;
			rh.ptr = NULL;
		}
		a_ptr(T* nptr) noexcept {
			abort_assert(nptr != NULL);
			ptr = nptr;
		}
		a_ptr& operator=(const a_ptr& rh) = delete;
		a_ptr& operator=(a_ptr&& rh) noexcept {
			ptr = rh.ptr;
			rh.ptr = NULL;
		}
		bool isinit() const noexcept {
			return (ptr != NULL);
		}
		const T* operator->() const noexcept {
			abort_assert(isinit());
			return ptr;
		}
		T* operator->() noexcept {
			abort_assert(isinit());
			return ptr;
		}
		~a_ptr() noexcept {
			delete ptr;
			ptr = NULL;
		}
		T* get() noexcept {
			abort_assert(isinit());
			return ptr;
		}
		const T* get() const noexcept {
			abort_assert(isinit());
			return ptr;
		}
		void del() noexcept {
			delete ptr;
			ptr = NULL;
		}
		T* operator=(T* nptr) noexcept {
			abort_assert(nptr != NULL);
			delete ptr;
			ptr = nptr;
			return ptr;
		}
	};

	template <class T>
	class aa_ptr : public Base {
	protected:
		T& operator[](int i) noexcept;
	private:
		T* ptr;
	public:
		aa_ptr() noexcept {
			ptr = NULL;
		}
		aa_ptr(const aa_ptr& src) = delete;
		aa_ptr(aa_ptr&& src) noexcept {
			ptr = src.ptr;
			src.ptr = NULL;
		}
		aa_ptr(T* nptr) noexcept {
			abort_assert(nptr != NULL);
			ptr = nptr;
		}
		aa_ptr& operator=(const aa_ptr& src) = delete;
		aa_ptr& operator=(aa_ptr&& src) noexcept {
			ptr = src.ptr;
			src.ptr = NULL;
		}
		bool isinit() const noexcept {
			return (ptr != NULL);
		}
		const T* operator->() const noexcept {
			abort_assert(isinit());
			return ptr;
		}
		T* operator->() noexcept {
			abort_assert(isinit());
			return ptr;
		}
		~aa_ptr() noexcept {
			delete[] ptr;
			ptr = NULL;
		}
		T* get() noexcept {
			abort_assert(isinit());
			return ptr;
		}
		const T* get() const noexcept {
			abort_assert(isinit());
			return ptr;
		}
		void del() noexcept {
			delete[] ptr;
			ptr = NULL;
		}
		T* operator=(T* nptr) noexcept {
			abort_assert(nptr != NULL);
			delete[] ptr;
			ptr = nptr;
			return ptr;
		}
	};

	template <class T>
	class a_refptr : public Base {
	private:
		T* ptr;
	public:
		a_refptr(const a_refptr &src) noexcept : Base () {
			ptr = src.ptr;
			if (ptr != NULL) {
				ptr->addref();
			}
		}
		a_refptr(a_refptr &&src) noexcept : Base () {
			ptr = src.ptr;
			src.ptr = NULL;
		}
		a_refptr(T* nptr) noexcept {
			abort_assert(nptr != NULL);
			ptr = nptr;
			ptr->addref();
		}
		a_refptr() noexcept {
			ptr = NULL;
		}
		~a_refptr() noexcept {
			if (ptr != NULL) {
				ptr->delref();
				ptr = NULL;
			}
		}
		const a_refptr& operator=(const a_refptr &src) noexcept {
			if (ptr != NULL) {
				ptr->delref();
			}
			ptr = src.ptr;
			if (ptr != NULL) {
				ptr->addref();
			}
			return *this;
		}
		const a_refptr& operator=(a_refptr &&src) noexcept {
			std::swap(ptr, src.ptr);
			return *this;
		}
		int isinit() const noexcept {
			return (ptr != NULL);
		}
		const T* operator->() const {
			if (ptr == NULL) {
				T** tmp = const_cast<T**>(&ptr);
				*tmp = new T;
				ptr->addref();
			}
			return ptr;
		}
		T* operator->() {
			if (ptr == NULL) {
				ptr = new T;
				ptr->addref();
			}
			return ptr;
		}
		T* get() noexcept {
			if (ptr == NULL) {
				ptr = new T;
				ptr->addref();
			}
			return ptr;
		}
		T& geto() noexcept {
			if (ptr == NULL) {
				ptr = new T;
				ptr->addref();
			}
			return *ptr;
		}
		const T* get() const {
			if (ptr == NULL) {
				T** tmp = const_cast<T**>(&ptr);
				*tmp = new T;
				ptr->addref();
			}
			return ptr;
		}
		T* operator=(T* nptr) noexcept {
			abort_assert(nptr != NULL);
			T* tmp = ptr;
			ptr = nptr;
			ptr->addref();
			if (tmp != NULL) {
				tmp->delref();
			}
			return ptr;
		}
		void del() noexcept {
			if (ptr != NULL) {
				ptr->delref();
			}
			ptr = NULL;
		}
	};

	uint64_t gettimesec(void);
	String sgethostname();
	uint64_t genid();
	String tohex(char *data, int size);

	class MD5_Hash {
	public:
		unsigned char buf[MD5_DIGEST_LENGTH];
	};

	MD5_Hash getMD5(void* data, size_t length);
	MD5_Hash getMD5(const String& data);
	String get_strhash(MD5_Hash hash);
	String get_base64hash(MD5_Hash hash);

	class SHA1_Hash {
	public:
		unsigned char buf[SHA_DIGEST_LENGTH];
	};

	SHA1_Hash getSHA1(void* data, size_t length);
	SHA1_Hash getSHA1(const String& data);
	String get_strhash(SHA1_Hash hash);
	String get_base64hash(SHA1_Hash hash);
	String get_strhmac256(const String& key, const String& data);

	String base64_encode(void* data, size_t length); // MIME (RFC 2045), RFC 3548 and RFC 4648 compliant

	uint16_t fasthash(const String& key) noexcept;

	uint8_t nibbletobin(char rh) noexcept;

	constexpr uint32_t crc_hash(const char *key, uint32_t len) noexcept
	{
		const uint32_t crctab[256] = {
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

		uint32_t hash = len;
		for (uint32_t i = 0; i < len; ++i) {
			hash = (hash >> 8) ^ crctab[(hash & 0xff) ^ (uint8_t)key[i]];
		}
		return hash;
	}

	constexpr uint32_t cstrhash (const char* key) {
		uint32_t len = 0;
		for (; key[len] != '\0'; ++len); // neither strlen() nor std::strlen() are constexpr.
		return crc_hash(key, len);
	}

	double getload();

	void call_external(Array<String>& args, bool dontwait = false);

	String get_strerror(int num);

	String pw_crypt(const String& pw);
	bool pw_crypt_compare(const String& pw, const String& hash);

	String XML_ESC(const String& lh, bool text = false);

}

#endif /* !_TOOL */
