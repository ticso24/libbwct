/*
 * Copyright (c) 2001,02,03,04,08 Bernd Walter Computer Technology
 * Copyright (c) 2008 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/array.h $
 * $Date: 2022-05-11 00:54:26 +0200 (Wed, 11 May 2022) $
 * $Author: ticso $
 * $Rev: 45533 $
 */

#ifndef _ARRAY
#define _ARRAY

#include "tool.h"

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
	void empty();
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
	T& operator<<(const Array<T> &rh);
	T& operator<<(Array<T> &&rh);
	int exists(const int i);
	int getsize();
	T& getlast();
	T merge(const T& separator);
	void sort();
	void sort(int (*func) (const T**, const T**));
	void shuffle();
	int indexof(T &value) const;
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
	try {
		*this = src;
	} catch (...) {
		empty();
		throw;
	}
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
	empty();
}

template <class T>
void Array<T>::empty() {
	check();
	for (int i = 0; i <= max; i++) {
		delete elements[i];
		elements[i] = NULL;
	}
	free (elements);
	elements = NULL;
	num_elem = 0;
	max = -1;
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
Array<T>::shuffle()
{
	Array<T> tmp = std::move(*this);
	empty();
	while(tmp.max >= 0) {
		int64_t i = getrandom() % (tmp.max + 1);
		(*this)[max + 1] = std::move(tmp[i]);
		tmp.del(i);
	}
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
			new_num = i + 1024;
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
	setsize(max + 1);
	max++;
	elements[max] = new T();
	*elements[max] = rh;
	return *elements[max];
}

template <class T>
T&
Array<T>::operator<<(T&& rh) {
	setsize(max + 1);
	max++;
	elements[max] = new T();
	*elements[max] = std::move(rh);
	return *elements[max];
}

template <class T>
T&
Array<T>::operator<<(const Array<T>& rh) {
	setsize(max + rh.max + 1);
	for (int64_t i = 0; i <= rh.max; i++) {
		(*this)[max + 1] = rh[i];
	}
	return *elements[max];
}

template <class T>
T&
Array<T>::operator<<(Array<T>&& rh) {
	setsize(max + rh.max + 1);
	for (int64_t i = 0; i <= rh.max; i++) {
		// steal the pointer from the source object
		max++;
		elements[max] = rh.elements[i];
		rh.elements[i] = NULL;
	}
	return *elements[max];
}

template <class T>
int
Array<T>::exists(const int i) {
	cassert(i >= 0);
	if (i > max)
		return 0;
	return (elements[i] != NULL);
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
Array<T>::indexof(T &value) const {
	for (int i = 0; i <= max; i++) {
		if (elements[i] != NULL && *elements[i] == value)
			return i;
	}
	return -1;
}

extern template class Array<String>;

#endif /* !_ARRAY */
