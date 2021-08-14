/*
 * Copyright (c) 2001,02,03,04,08 Bernd Walter Computer Technology
 * Copyright (c) 2008 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/tool.h $
 * $Date: 2021-07-16 15:09:29 +0200 (Fri, 16 Jul 2021) $
 * $Author: ticso $
 * $Rev: 44524 $
 */

#ifndef _TOOL
#define _TOOL

class Base;
class String;
class Mutex;

template <class T>
class Array;

template <class T>
class List;

#ifndef DEBUG
#define dbg_beep()
#define dbg_beepme()
#define wassert(test)
#define cassert(test)
#define cassertm(test, msg)
#endif /* !DEBUG */

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

#ifdef DEBUG
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

#endif /* DEBUG */

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
	void log(int priority, const String& str) const;
	void log(int priority, const char *str) const;
	void log(const String& str) const;
	void log(const char *str) const;
	virtual void check() const;
	Base();
	virtual ~Base();
	void addref();
	void delref();
	int getref() const
	{
		return refcount;
	}
	virtual String tinfo() const;
};

#include "mstring.h"

class Error : public std::exception {
public:
	String msg;
	Error(const char* msg, bool log = true) {
		(this)->msg = msg;
#ifdef DEBUG
		if (log) {
			syslog(LOG_INFO, "Error thrown: %s", msg);
		}
#endif
	}
	Error(const String& msg = "", bool log = true) {
		(this)->msg = msg;
#ifdef DEBUG
		if (log) {
			syslog(LOG_INFO, "Error thrown: %s", msg.c_str());
		}
#endif
	}
	~Error() throw() {
	}
	virtual const char* what () const throw() {
		return msg.c_str();
	}
};

#ifdef DEBUG
#define abort_assert(test)						\
if (!(test)) {								\
	syslog (LOG_EMERG,						\
	    "assertion (%s) failed: %s@%d in %s",			\
	    #test, __FILE__, __LINE__, __func__);			\
	abort();							\
}
#endif

#ifdef DEBUG
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
#endif /* DEBUG */

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
	const a_ptr& operator=(const a_ptr &src);
	a_ptr(const a_ptr &src);
	T& operator[](int i);
private:
	T* ptr;
public:
	a_ptr(T* nptr) {
		abort_assert(nptr != NULL);
		ptr = nptr;
	}
	a_ptr() {
		ptr = NULL;
	}
	bool isinit() const {
		return (ptr != NULL);
	}
	const T* operator->() const {
		abort_assert(isinit());
		return ptr;
	}
	T* operator->() {
		abort_assert(isinit());
		return ptr;
	}
	~a_ptr() {
		delete ptr;
		ptr = NULL;
	}
	T* get() {
		abort_assert(isinit());
		return ptr;
	}
	const T* get() const {
		abort_assert(isinit());
		return ptr;
	}
	void del() {
		delete ptr;
		ptr = NULL;
	}
	T* operator=(T* nptr) {
		abort_assert(nptr != NULL);
		delete ptr;
		ptr = nptr;
		return ptr;
	}
};

template <class T>
class a_refptr : public Base {
private:
	T* ptr;
public:
	a_refptr(const a_refptr &src) : Base () {
		ptr = src.ptr;
		if (ptr != NULL) {
			ptr->addref();
		}
	}
	a_refptr(a_refptr &&src) : Base () {
		ptr = src.ptr;
		src.ptr = NULL;
	}
	a_refptr(T* nptr) {
		abort_assert(nptr != NULL);
		ptr = nptr;
		ptr->addref();
	}
	a_refptr() {
		ptr = NULL;
	}
	~a_refptr() {
		if (ptr != NULL) {
			ptr->delref();
			ptr = NULL;
		}
	}
	const a_refptr& operator=(const a_refptr &src) {
		if (ptr != NULL) {
			ptr->delref();
		}
		ptr = src.ptr;
		if (ptr != NULL) {
			ptr->addref();
		}
		return *this;
	}
	const a_refptr& operator=(a_refptr &&src) {
		std::swap(ptr, src.ptr);
		return *this;
	}
	int isinit() const {
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
	T* get() {
		if (ptr == NULL) {
			ptr = new T;
			ptr->addref();
		}
		return ptr;
	}
	T& geto() {
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
	T* operator=(T* nptr) {
		abort_assert(nptr != NULL);
		T* tmp = ptr;
		ptr = nptr;
		ptr->addref();
		if (tmp != NULL) {
			tmp->delref();
		}
		return ptr;
	}
	void del() {
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

extern Mutex fetch_mtx;
void downloadURL(const String& URL, const String& path, bool cert_check = true);
String base64_encode(void* data, size_t length); // MIME (RFC 2045), RFC 3548 and RFC 4648 compliant

uint16_t fasthash(const String& key);

uint8_t nibbletobin(char rh);

uint32_t crc_hash(const void *key, uint32_t len, uint32_t hash);

double getload();

String get_strerror(int num);

String pw_crypt(const String& pw);
bool pw_crypt_compare(const String& pw, const String& hash);

String XML_ESC(const String& lh, bool text = false);

#endif /* !_TOOL */
