/*
 * Copyright (c) 2001,02,03,04,08 Bernd Walter Computer Technology
 * Copyright (c) 2008 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/sarray.h $
 * $Date: 2021-08-13 12:57:59 +0200 (Fri, 13 Aug 2021) $
 * $Author: ticso $
 * $Rev: 44740 $
 */

#ifndef _SARRAY
#define _SARRAY

#include "tool.h"

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
	T& operator<<(const SArray<T>& rh);
	const T& operator[](const int i) const;
	int getsize();
	T& getlast();
	int indexof(T value);
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
			new_num = i + 1024;
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
T&
SArray<T>::operator<<(const SArray<T>& rh) {
	setsize(max + rh.max + 1);
	for (int64_t i = 0; i <= rh.max; i++) {
		(*this)[max + 1] = rh[i];
	}
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

#endif /* !_SARRAY */
