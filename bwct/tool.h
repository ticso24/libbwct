/*
 * Copyright (c) 2001,02 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#ifndef _TOOL
#define _TOOL

#include "config.h"

class Base;

template <class T>
class Array;

template <class T>
class List;

#ifndef DEBUG
#define beep()
#define beepme()
#define wassert(test)
#define cassert(test)
#define timer()
#endif /* !DEBUG */

#ifdef DEBUG
#define beep() {							\
	syslog (LOG_DEBUG, "beep: %s@%d in %s",				\
	    __FILE__, __LINE__, __func__);				\
}

#define beepme() {							\
	syslog (LOG_DEBUG, "beep: %s %s@%d in %s",			\
	    tinfo().c_str(), __FILE__, __LINE__, __func__);		\
}

#define timer() {							\
	struct timeval tv;						\
	gettimeofday(&tv, NULL);					\
	syslog(LOG_DEBUG, "%s@%d in %s time: %ld %ld",			\
	    __FILE__, __LINE__, __func__, tv.tv_sec, tv.tv_usec);	\
}

#define wassert(test)							\
if (!(test)) {								\
	syslog (LOG_CRIT,						\
	    "assertion \"%s\" failed: %s@%d in %s",			\
	    #test, __FILE__, __LINE__, __func__);			\
}

#define cassert(test)							\
if (!(test)) {								\
	syslog (LOG_EMERG,						\
	    "assertion \"%s\" failed: %s@%d in %s",			\
	    #test, __FILE__, __LINE__, __func__);			\
	abort();							\
}
#endif /* DEBUG */

#ifdef FREEDEBUG
#define free(ptr);							\
	syslog (LOG_DEBUG,						\
	    "free(%s) called: %s@%d ind %s ptr=%p",			\
		#ptr, __FILE__, __LINE__, __func__, ptr);		\
	free(ptr);
#endif /* FREEDEBUG */

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

class Ftask;

template <class T>
class Matrix {
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
	if (newsize <= size)
		return;
	T *newdata = new T[num];
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

class String {
private:
	int ln;
	char *data;
public:
	String();
	String(const char *rhs);
	String(const Matrix<char>& rhs);
	String(const String &rhs);
	String(char rhs);
	String(short rhs);
	String(int rhs);
	String(long rhs);
	String(long long rhs);
	String(unsigned char rhs);
	String(unsigned short rhs);
	String(unsigned int rhs);
	String(unsigned long rhs);
	String(unsigned long long rhs);
	String(const void *rhs);
	const String& operator= (const String &rhs);
	const String& operator= (const Matrix<char>& rhs);
	const String& operator= (const char *rhs);
	const String& operator= (unsigned int rhs);
	const String& operator= (unsigned long rhs);
	const String& operator= (unsigned long long rhs);
	const String& operator= (const void *rhs);
	int length() const;
	const char *c_str() const;
	~String();
	bool operator== (const String &rhs) const;
	bool operator== (const Matrix<char>& rhs) const;
	bool operator== (const char *rhs) const;
	bool operator!= (const String &rhs) const;
	bool operator!= (const Matrix<char>& rhs) const;
	bool operator!= (const char *rhs) const;
	template <class T>
	String operator+(const T &rhs) const;
	template <class T>
	String& operator<<(const T &rhs);
	String& operator+=(const String &rhs);
	String& operator+=(const Matrix<char>& rhs);
	String& operator+=(const char *rhs);
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
	String& operator+=(const void *rhs);
	void resize(int rhs);
	bool empty() const;
	const char *operator[] (const int i) const;
	void lower();
	void upper();
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

class Error : public std::exception {
public:
	String msg;
	Error(const String& msg = "") {
		(this)->msg = msg;
#ifdef DEBUG
		syslog(LOG_INFO, "Error thrown: %s", msg.c_str());
#endif
	}
	~Error() throw() {
	}
	virtual const char* what () const throw() {
		return msg.c_str();
	}
};

class Base {
private:
	int state;
	int refcount;
public:
	friend class Ftask;
	void log(const String& str);
	void check() const {

		cassert(refcount >= 0);
	}
	Base () {

		refcount = 0;
//		log("create");
	}
	virtual ~Base () {

		check();
		cassert(refcount == 0);
		refcount = -1;
//		log("destroy");
	}
	void addref();
	void delref();
	virtual String tinfo() const;
};

template <class T>
class SArray {
private:
	int num_elem;
	T *elements;
	
public:
	int max;
	SArray();
	SArray(const SArray &src);
	void del(const int i);
	const T& operator= (const SArray &src);
	~SArray();
	T& operator[](const int i);
};

template <class T>
SArray<T>::SArray () {

	max = -1;
	num_elem = 8;
	elements = (T *)calloc(num_elem, sizeof(T));
	if (!elements)
		throw std::bad_alloc();
}

template <class T>
SArray<T>::~SArray () {

	free (elements);
}

template <class T>
void
SArray<T>::del(const int i) {

	if (i > max)
		return;
	if (i != max)
		memmove(&elements[i], &elements[i + 1], sizeof(T) * max - i);
	max--;
}

template <class T>
T&
SArray<T>::operator[](int i) {
	int old_num;
	T *tmp;
	
	cassert(i >= 0);
	if (i > max) {
		max = i;
		if (i >= num_elem) {
			old_num = num_elem;
			if (i >= 1024) {
				num_elem = (i + 1024) & ~1023;
			} else {
				while (i >= num_elem)
					num_elem *= 2;
			}
			tmp = (T *)realloc(elements, num_elem * sizeof(T));
			if (!tmp)
				throw std::bad_alloc();
			elements = tmp;
			bzero((char*)&elements[old_num],
			    sizeof(T) * (num_elem - old_num));
		}
	}
	return elements[i];
};

template <class T>
class Array {
private:
	int num_elem;
	T **elements;
	void setsize(const int i);
public:
	int max;
	Array();
	Array(const Array &src);
	void del(const int i);
	void insert(const int i);
	void clear(const int i);
	T* cutptr(const int i);
	void pasteptr(const int i, T* ptr);
	const T& operator=(const Array &src);
	~Array();
	T& operator[](const int i);
	int exists(const int i);
};

template <class T>
Array<T>::Array() {
	max = -1;
	num_elem = 8;
	elements = (T **)calloc(num_elem, sizeof(T*));
	if (!elements)
		throw std::bad_alloc();
}

template <class T>
Array<T>::~Array() {
	for (int i = 0; i < num_elem; i++)
		delete elements[i];
	free (elements);
}

template <class T>
void
Array<T>::clear(const int i) {
	cassert(i >= 0);
	if (i > max)
		return;
	delete elements[i];
	elements[i] = NULL;
}

template <class T>
void
Array<T>::insert(const int i) {
	cassert(i >= 0);
	if (i - 1 > max)
		return;
	setsize(i);
	if (i <= max) {
		memmove(&elements[i + 1], &elements[i],
		    sizeof(T*) * (max - i + 1));
		max++;
	}
}

template <class T>
void
Array<T>::del(const int i) {
	cassert(i >= 0);
	if (i > max)
		return;
	delete elements[i];
	if (i != max)
		memmove(&elements[i], &elements[i + 1], sizeof(T*) * (max - i));
	max--;
}

template <class T>
void
Array<T>::setsize(const int i) {
	if (i >= num_elem) {
		int old_num = num_elem;
		if (i >= 1024) {
			num_elem = (i + 1024) & ~1023;
		} else {
			while (i >= num_elem)
				num_elem *= 2;
		}
		T **tmp = (T **)realloc(elements, num_elem * sizeof(T*));
		if (!tmp)
			throw std::bad_alloc();
		elements = tmp;
		bzero((char*)&elements[old_num],
		    sizeof(T*) * (num_elem - old_num));
	}
}

template <class T>
T*
cutptr(const int i) {
	cassert(i >= 0);
	T *tmp = elements[i];
	elements[i] = NULL;
	return tmp;
}

template <class T>
void
pasteptr(const int i, T* ptr) {
	cassert(i >= 0);
	if (i > max) {
		max = i;
		setsize(i);
	}
	delete elements[i];
	elements[i] = ptr;
}

template <class T>
T&
Array<T>::operator[](int i) {
	cassert(i >= 0);
	if (i > max) {
		max = i;
		setsize(i);
	}
	if (!elements[i])
		elements[i] = new T();
	return *elements[i];
};

template <class T>
int
Array<T>::exists(const int i) {
	cassert(i >= 0);
	if (i > max)
		return 0;
	return (elements[i] != 0);
}

template <class T>
class List {
private:
	struct elements {
		T *obj;
		struct elements *next;
	};
	struct elements *e0, *el;
	int numobj;
	
public:
	List();
	List(List &src);
	~List();
	int empty() {
		return !numobj;
	}
	void clear() {
		for (; !empty(); --(*this));
	}
	T& operator[](int i) ;
	void operator+=(const T& obj);
	void operator-=(int i);
	void operator--();

};

template <class T>
List<T>::List() {

	e0 = el = 0;
	numobj = 0;
};

template <class T>
List<T>::~List() {

	clear();
	cassert(numobj == 0);
};

template <class T>
T&
List<T>::operator[](int i) {

	cassert (i == 0);
	cassert (e0);
	return *e0->obj;
};

template <class T>
void
List<T>::operator+=(const T& obj) {
	struct elements *ne;

	ne = new struct elements;
	ne->obj = new T(obj);
	ne->next = 0;
	if (el == 0)
		e0 = ne;
	else
		el->next=ne;
	el = ne;
	numobj++;
};

template <class T>
void
List<T>::operator--() {
	struct elements *en;

	if (e0) {
		en = e0->next;
		delete e0->obj;
		delete e0;
		e0=en;
		if (e0 == 0)
			el = 0;
		numobj--;
		cassert(numobj >= 0);
	}
};

template <class T>
void
List<T>::operator-=(int i) {
	struct elements *en;

	cassert (i == 0);
	if (e0) {
		en = e0->next;
		delete e0->obj;
		delete e0;
		e0=en;
		if (e0 == 0)
			el = 0;
		numobj--;
		cassert(numobj >= 0);
	}
};

template <class T>
class a_ptr {
protected:
	const a_ptr& operator=(const a_ptr &src);
	a_ptr(const a_ptr &src);
	T& operator[](int i);
private:
	T* ptr;
public:
	a_ptr(T* nptr) {
		ptr = nptr;
	}
	a_ptr() {
		ptr = NULL;
	}
	int isinit() const {
		return (ptr != NULL);
	}
	const T* operator->() const {
		cassert(isinit());
		return ptr;
	}
	T* operator->() {
		cassert(isinit());
		return ptr;
	}
	~a_ptr() {
		delete ptr;
	}
	T* get() {
		cassert(isinit());
		return ptr;
	}
	T* operator=(T* nptr) {
		delete ptr;
		ptr = nptr;
		return ptr;
	}
};

uint64_t gettimesec(void);
String sgethostname();
uint64_t genid();
String tohex(char *data, int size) {
String genmd5id();
uint64_t getdate();

#endif /* !_TOOL */
