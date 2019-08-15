/*
 * Copyright (c) 2001,02,03,04,08 Bernd Walter Computer Technology
 * Copyright (c) 2008 FIZON GmbH
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#ifndef _TOOL
#define _TOOL

#include <pthread.h>

class Base;
class String;
class Mutex;

template <class T>
class Array;

template <class T>
class Matrix;

template <class T>
class List;

#ifndef DEBUG
#define dbg_beep()
#define dbg_beepme()
#define wassert(test)
#define cassert(test)
#define timer()
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

#define timer() {							\
	struct timeval tv;						\
	gettimeofday(&tv, NULL);					\
	syslog(LOG_DEBUG, "%s@%d in %s time: %ld %06ld",		\
	    __FILE__, __LINE__, __func__, tv.tv_sec, tv.tv_usec);	\
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

#include <bwct/mstring.h>

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
class Matrix : public Base {
private:
	T *data;
	int num;
public:
	Matrix(const int i);
	~Matrix();
	Matrix& operator=(const Matrix& rhs);
	T& operator[](const int i);
	T *get();
	const T *get() const;
	size_t size();
	void setsize(size_t newsize);
};

template <class T>
void
Matrix<T>::setsize(size_t newsize) {
	if (newsize <= num)
		return;
	T *newdata = new T[newsize];
	for (int i = 0; i < MIN(newsize, num); i++)
		newdata[i] = data[i];
	data = newdata;
}

template <class T>
size_t
Matrix<T>::size() {
	return num;
}

template <class T>
Matrix<T>::Matrix(const int i) {
	num = i;
	data = new T[num];
}

template <class T>
Matrix<T>::~Matrix() {
	delete[]data;
}

template <class T>
T&
Matrix<T>::operator[](const int i) {
	cassert(i < num);
	return data[i];
}

template <class T>
T*
Matrix<T>::get() {
	return data;
}

template <class T>
const T*
Matrix<T>::get() const {
	return data;
}

/* simplified array for scalar data types */
template <class T>
class SArray : public Base {
private:
	int num_elem;
	T *elements;
	void setsize(const int i);

public:
	int max;
	SArray();
	SArray(const SArray &src);
	SArray(SArray &&src);
	~SArray();
	void del(const int i);
	void insert(const int i);
	const SArray& operator= (const SArray &src);
	const SArray& operator= (SArray &&src);
	T& operator[](const int i);
	T& operator<<(T rh);
	const T& operator[](const int i) const;
	int getsize();
	T& getlast();
	int indexof(T value);
	int begin() const {
		return 0;
	}
	int end() const {
		return max + 1;
	}
};

template <class T>
SArray<T>::SArray () {
	static_assert(std::is_scalar<T>::value, "type must be scalar");

	max = -1;
	num_elem = 0;
	elements = NULL;
}

template <class T>
SArray<T>::SArray (const SArray &src) : Base() {
	static_assert(std::is_scalar<T>::value, "type must be scalar");

	max = src.max;
	num_elem = src.num_elem;
	elements = (T *)calloc(num_elem, sizeof(T));
	if (!elements)
		throw std::bad_alloc();
	bcopy(src.elements, elements, num_elem * sizeof(T));
}

template <class T>
SArray<T>::SArray (SArray &&src) : Base() {
	static_assert(std::is_scalar<T>::value, "type must be scalar");

	max = src.max;
	src.max = -1;
	num_elem = src.num_elem;
	src.num_elem = 0;
	elements = src.elements;
	src.elements = NULL;
}

template <class T>
SArray<T>::~SArray () {
	check();
	free (elements);
}

template <class T>
const SArray<T>&
SArray<T>::operator= (const SArray &src) {

	free(elements);
	max = src.max;
	num_elem = src.num_elem;
	elements = (T *)calloc(num_elem, sizeof(T));
	if (!elements)
		throw std::bad_alloc();
	bcopy(src.elements, elements, num_elem * sizeof(T));
	return *this;
}

template <class T>
const SArray<T>&
SArray<T>::operator= (SArray &&src) {

	std::swap(elements, src.elements);
	std::swap(max, src.max);
	std::swap(num_elem, src.num_elem);
	return *this;
}

template <class T>
void
SArray<T>::del(const int i) {

	abort_assert(i <= max);
	abort_assert(i >= 0);
	if (i != max)
		memmove(&elements[i], &elements[i + 1], sizeof(T) * (max - i));
	max--;
}

template <class T>
void
SArray<T>::setsize(const int i) {
	if (i >= num_elem) {
		int new_num = num_elem;
		if (i >= 1024) {
			new_num = (i + 1024) & ~1023;
		} else {
			new_num = (new_num > 0) ? new_num : 8;
			while (i >= new_num) {
				new_num *= 2;
			}
		}
		T *tmp = (T *)realloc(elements, new_num * sizeof(T));
		if (!tmp)
			throw std::bad_alloc();
		elements = tmp;
		bzero((char*)&elements[num_elem],
		    sizeof(T) * (new_num - num_elem));
		num_elem = new_num;
	}
}

template <class T>
void
SArray<T>::insert(const int i) {
	cassert(i >= 0);
	check();
	if (i - 1 > max)
		return;
	setsize(max + 1);
	if (i <= max) {
		memmove(&elements[i + 1], &elements[i], sizeof(T*) * (max - i + 1));
		max++;
		elements[i] = 0;
	}
	check();
}

template <class T>
const T&
SArray<T>::operator[](int i) const {
	cassert(i >= 0);
	cassert(i <= max);
	return elements[i];
};

template <class T>
T&
SArray<T>::operator[](int i) {

	cassert(i >= 0);
	if (i > max) {
		setsize(i);
		max = i;
	}
	return elements[i];
};

template <class T>
T&
SArray<T>::operator<<(T rh) {
	(*this)[max + 1] = rh;
	return elements[max];
}

template <class T>
int
SArray<T>::getsize() {
	return max;
}

template <class T>
T&
SArray<T>::getlast() {
	return elements[max];
}

template <class T>
int
SArray<T>::indexof(T value) {
	for (int i = 0; i <= max; i++) {
		if (elements[i] == value)
			return i;
	}
	return -1;
}

template <class T>
class Array : public Base {
private:
	int num_elem;
protected:
	T **elements;
	void setsize(const int i);
public:
	int max;
	Array();
	Array(const Array &src);
	Array(Array &&src);
	~Array();
	virtual void check() const;
	void del(const int i);
	void insert(const int i);
	void clear(const int i);
	T* cutptr(const int i);
	void pasteptr(const int i, T* ptr);
	bool operator==(const Array &lh) const;
	bool operator!=(const Array &lh) const;
	const Array<T>& operator=(const Array &lh);
	const Array<T>& operator=(Array &&lh);
	T& operator[](const int i);
	const T& operator[](const int i) const;
	T& operator<<(const T &rh);
	T& operator<<(T &&rh);
	int exists(const int i);
	int getsize();
	T& getlast();
	T merge(const T& separator);
	void sort();
	void sort(int (*func) (const T**, const T**));
	int indexof(T &value);
	int begin() const {
		return 0;
	}
	int end() const {
		return max + 1;
	}
};

template <class T>
Array<T>::Array() {
	static_assert(!std::is_scalar<T>::value, "type must not be scalar");

	max = -1;
	num_elem = 0;
	elements = NULL;
	check();
}

template <class T>
Array<T>::Array(const Array &src) : Base()
{
	static_assert(!std::is_scalar<T>::value, "type must not be scalar");

	max = -1;
	num_elem = 0;
	elements = NULL;
	*this = src;
}

template <class T>
Array<T>::Array(Array &&src) : Base()
{
	static_assert(!std::is_scalar<T>::value, "type must not be scalar");

	max = src.max;
	src.max = -1;
	num_elem = src.num_elem;
	src.num_elem = 0;
	elements = src.elements;
	src.elements = NULL;
}

template <class T>
Array<T>::~Array() {
	check();
	for (int i = 0; i <= max; i++)
		delete elements[i];
	free (elements);
}

template <class T>
void
Array<T>::check() const
{
#if 0
	for (int i = 0; i <= max; i++) {
		for (int j = i; j <= max; j++) {
			if (i != j) {
				if (elements[i] != NULL) {
					abort_assert(elements[i] != elements[j]);
				}
			}
		}
	}
#endif
	Base::check();
}

template <class T>
void
Array<T>::clear(const int i) {
	cassert(i >= 0);
	check();
	if (i > max)
		return;
	delete elements[i];
	elements[i] = NULL;
}

template <class T>
void
Array<T>::insert(const int i) {
	cassert(i >= 0);
	check();
	if (i - 1 > max)
		return;
	setsize(max + 1);
	if (i <= max) {
		memmove(&elements[i + 1], &elements[i], sizeof(T*) * (max - i + 1));
		max++;
		elements[i] = NULL;
	}
	check();
}

template <class T>
void
Array<T>::sort()
{
	if (max < 1) {
		return;
	}
	auto helper = [] (const T** a, const T** b) {
		int ret = 0;
		if (**a < **b) {
			ret = -1;
		} else if (**a > **b) {
			ret = +1;
		}
		return ret;
	};
	sort(helper);
}

template <class T>
void
Array<T>::sort(int (*func) (const T**, const T**))
{
	::qsort(elements, max + 1, sizeof(T*), (int (*)(const void*, const void*))func);
	check();
}

template <class T>
void
Array<T>::del(const int i) {
	abort_assert(i <= max);
	abort_assert(i >= 0);
	check();
	delete elements[i];
	if (i != max)
		memmove(&elements[i], &elements[i + 1], sizeof(T*) * (max - i));
	elements[max] = NULL;
	max--;
	check();
}

template <class T>
void
Array<T>::setsize(const int i) {
	if (i >= num_elem) {
		int new_num = num_elem;
		if (i >= 1024) {
			new_num = (i + 1024) & ~1023;
		} else {
			new_num = (new_num > 0) ? new_num : 8;
			while (i >= new_num) {
				new_num *= 2;
			}
		}
		T **tmp = (T **)realloc(elements, new_num * sizeof(T*));
		if (!tmp)
			throw std::bad_alloc();
		elements = tmp;
		bzero((char*)&elements[num_elem],
		    sizeof(T*) * (new_num - num_elem));
		num_elem = new_num;
		check();
	}
}

template <class T>
bool
Array<T>::operator==(const Array &lh) const
{
	for (int i = 0; i <= max; i++) {
		if (elements[i] != NULL && lh.elements[i] != NULL) {
			if (*elements[i] != *lh.elements[i]) {
				return false;
			}
		} else if (elements[i] != NULL || lh.elements[i] != NULL) {
			return false;
		}
	}
	return true;
}

template <class T>
bool
Array<T>::operator!=(const Array &lh) const
{
	for (int i = 0; i <= max; i++) {
		if (elements[i] != NULL && lh.elements[i] != NULL) {
			if (*elements[i] == *lh.elements[i]) {
				return false;
			}
		} else if (elements[i] == NULL && lh.elements[i] == NULL) {
			return false;
		}
	}
	return true;
}

template <class T>
const Array<T>&
Array<T>::operator=(const Array &lh)
{
	while (max >= 0) {
		delete elements[max];
		elements[max] = NULL;
		max--;
	}
	setsize(lh.max);
	for (int i = 0; i <= lh.max; i++) {
		(*this)[i] = lh[i];
	}
	return *this;
}

template <class T>
const Array<T>&
Array<T>::operator=(Array &&lh)
{
	std::swap(num_elem, lh.num_elem);
	std::swap(max, lh.max);
	std::swap(elements, lh.elements);
	return *this;
}

template <class T>
T*
Array<T>::cutptr(const int i) {
	cassert(i >= 0);
	T *tmp = elements[i];
	elements[i] = NULL;
	return tmp;
}

template <class T>
void
Array<T>::pasteptr(const int i, T* ptr) {
	cassert(i >= 0);
	if (i > max) {
		max = i;
		setsize(i);
	}
	delete elements[i];
	elements[i] = ptr;
}

template <class T>
const T&
Array<T>::operator[](int i) const {

	cassert(i >= 0);
	cassert(i <= max);
	check();
	Array<T> *me = const_cast<Array<T>*>(this);
	if (!elements[i])
		me->elements[i] = new T();
	return *me->elements[i];
};

template <class T>
T&
Array<T>::operator[](int i) {
	cassert(i >= 0);
	if (i > max) {
		setsize(i);
		max = i;
	}
	if (!elements[i])
		elements[i] = new T();
	return *elements[i];
};

template <class T>
T&
Array<T>::operator<<(const T& rh) {
	(*this)[max + 1] = rh;
	return *elements[max];
}

template <class T>
T&
Array<T>::operator<<(T&& rh) {
	setsize(max + 1);
	max++;
	elements[max] = new T(std::move(rh));
	return *elements[max];
}

template <class T>
int
Array<T>::exists(const int i) {
	cassert(i >= 0);
	if (i > max)
		return 0;
	return (elements[i] != 0);
}

template <class T>
int
Array<T>::getsize() {
	return max;
}

template <class T>
T&
Array<T>::getlast() {
	if (!elements[max])
		elements[max] = new T();
	return *elements[max];
}

template <class T>
T
Array<T>::merge(const T& separator) {
	T ret;
	bool first = true;

	for (int i = 0; i <= max; i++) {
		if (first) {
			first = false;
		} else {
			ret += separator;
		}
		ret += (*this)[i];
	}
	return ret;
}

template <class T>
int
Array<T>::indexof(T &value) {
	for (int i = 0; i <= max; i++) {
		if (elements[i] != NULL && *elements[i] == value)
			return i;
	}
	return -1;
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
		std::swap(ptr, src.ptr);
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
uint64_t getdate();

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

extern Mutex fetch_mtx;
String base64_encode(void* data, size_t length); // MIME (RFC 2045), RFC 3548 and RFC 4648 compliant

uint16_t fasthash(const String& key);

uint8_t nibbletobin(char rh);

extern template class Array<String>;

uint32_t crc_hash(const void *key, uint32_t len, uint32_t hash);

#endif /* !_TOOL */
