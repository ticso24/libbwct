/*
 * Copyright (c) 2001,02,03,04,08 Bernd Walter Computer Technology
 * Copyright (c) 2008 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/sarray.h $
 * $Date: 2025-06-10 19:26:00 +0200 (Tue, 10 Jun 2025) $
 * $Author: ticso $
 * $Rev: 49347 $
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
	template <bool IsConst>
	struct Iterator {
	private:
		T* pos;
	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type      = typename std::conditional_t<IsConst, const T, T>;
		using reference         = value_type&;
		using pointer           = value_type*;
		using difference_type   = std::ptrdiff_t;

		friend class SArray<T>;

		Iterator(const Iterator&) = default;
		Iterator& operator=(const Iterator&) = default;

		template<bool WasConst, class = std::enable_if_t<IsConst && !WasConst>>
		Iterator(const Iterator<WasConst>& rhs) : pos(rhs.pos) {}

		template<bool WasConst, class = std::enable_if_t<IsConst && !WasConst>>
		Iterator& operator=(const Iterator<WasConst>& rhs) {
			pos = rhs.pos;
			return (*this);
		}

		Iterator(T* rpos) {
			pos = rpos;
		}

		pointer operator->() const {
			return pos;
		}

		reference operator*() const {
			return *pos;
		}

		Iterator& operator--() {
			--pos;
			return *this;
		}

		Iterator operator--(int) {
			Iterator tmp = *this;
			--(*this);
			return tmp;
		}

		Iterator& operator++() {
			++pos;
			return *this;
		}

		Iterator operator++(int) {
			Iterator tmp = *this;
			++(*this);
			return tmp;
		}

		Iterator& operator+=(int a) {
			pos += a;
			return *this;
		}

		Iterator& operator-=(int a) {
			pos -= a;
			return *this;
		}

		reference operator[](int i) {
			return *(pos[i]);
		}

		friend Iterator operator+(Iterator a, int b) {
			a += b;
			return a;
		}

		friend Iterator operator-(Iterator a, int b) {
			a -= b;
			return a;
		}

		friend int operator-(const Iterator& a, const Iterator& b) {
			return a.pos - b.pos;
		}

		friend bool operator< (const Iterator& a, const Iterator& b) {
			return a.pos < b.pos;
		}

		friend bool operator== (const Iterator& a, const Iterator& b) {
			return a.pos == b.pos;
		}

		friend bool operator!= (const Iterator& a, const Iterator& b) {
			return a.pos != b.pos;
		}
	};

	using iterator = Iterator<false>;
	using const_iterator = Iterator<true>;

	iterator begin() {
		return iterator(elements);
	}
	iterator end() {
		return iterator(elements +  max + 1);
	}
	const_iterator begin() const {
		return const_iterator(elements);
	}
	const_iterator end() const {
		return const_iterator(elements +  max + 1);
	}
	const_iterator cbegin() const {
		return const_iterator(elements);
	}
	const_iterator cend() const {
		return const_iterator(elements +  max + 1);
	}
	auto rbegin() {
		return std::make_reverse_iterator(iterator(&elements[max]));
	}
	auto rend() {
		return std::make_reverse_iterator(iterator(&elements[-1]));
	}
	auto rbegin() const {
		return std::make_reverse_iterator(const_iterator(&elements[max]));
	}
	auto rend() const {
		return std::make_reverse_iterator(const_iterator(&elements[-1]));
	}
	auto crbegin() const {
		return std::make_reverse_iterator(const_iterator(&elements[max]));
	}
	auto crend() const {
		return std::make_reverse_iterator(const_iterator(&elements[-1]));
	}

	int max;
	SArray() noexcept;
	SArray(const SArray &src);
	SArray(SArray &&src) noexcept;
	SArray(std::initializer_list<T> ilist);
	~SArray() noexcept;
	void erase(const int i) noexcept;
	iterator erase(iterator pos);
	iterator erase(const_iterator pos);
	iterator erase(iterator first, iterator last);
	iterator erase(const_iterator first, const_iterator last);
	void insert(const int i);
	iterator insert(const_iterator pos, const T& value);
	iterator insert(const_iterator pos, int count, const T& value);
	template<class InputIt>
	iterator insert(const_iterator pos, InputIt first, InputIt last);
	iterator insert(const_iterator pos, std::initializer_list<T> ilist);
	const SArray& operator= (const SArray &src);
	const SArray& operator= (SArray &&src) noexcept;
	const SArray<T>& operator=(std::initializer_list<T> ilist);
	T& operator[](const int i);
	T& operator<<(T rh);
	T& operator<<(const SArray<T>& rh);
	const T& operator[](const int i) const;
	int getsize() noexcept;
	T& getlast();
	int indexof(T value) noexcept;
	void clear() noexcept;
	void pop_back() noexcept;
	void pop_front() noexcept;
	void push_back(T i);
	void emplace_back(T i);
	void push_front(T i);

	int size() const {
		return max + 1;
	}
};

template <class T>
SArray<T>::SArray () noexcept
{
	static_assert(std::is_scalar<T>::value, "type must be scalar");

	max = -1;
	num_elem = 0;
	elements = NULL;
}

template <class T>
SArray<T>::SArray (const SArray &src) : Base()
{
	static_assert(std::is_scalar<T>::value, "type must be scalar");

	max = src.max;
	num_elem = src.num_elem;
	elements = (T *)calloc(num_elem, sizeof(T));
	if (!elements)
		throw std::bad_alloc();
	bcopy(src.elements, elements, num_elem * sizeof(T));
}

template <class T>
SArray<T>::SArray(std::initializer_list<T> ilist)
{
	max = -1;
	num_elem = 0;
	elements = NULL;
	for (auto& x: ilist) {
		(*this).emplace_back(x);
	}
}

template <class T>
SArray<T>::SArray (SArray &&src) noexcept : Base()
{
	static_assert(std::is_scalar<T>::value, "type must be scalar");

	max = src.max;
	src.max = -1;
	num_elem = src.num_elem;
	src.num_elem = 0;
	elements = src.elements;
	src.elements = NULL;
}

template <class T>
SArray<T>::~SArray () noexcept
{
	check();
	free (elements);
}

template <class T>
const SArray<T>&
SArray<T>::operator= (const SArray &src)
{

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
SArray<T>::operator= (SArray &&src) noexcept {

	std::swap(elements, src.elements);
	std::swap(max, src.max);
	std::swap(num_elem, src.num_elem);
	return *this;
}

template <class T>
const SArray<T>& SArray<T>::operator=(std::initializer_list<T> ilist)
{
	clear();
	for (auto& x: ilist) {
		(*this).emplace_back(x);
	}
	return *this;
}

template <class T>
void
SArray<T>::pop_front() noexcept {

	abort_assert(max >= 0);
	if (0 != max)
		memmove(&elements[0], &elements[1], sizeof(T) * (max));
	max--;
}

template <class T>
void
SArray<T>::pop_back() noexcept {

	abort_assert(max >= 0);
	max--;
}

template <class T>
void
SArray<T>::erase(const int i) noexcept {

	abort_assert(i <= max);
	abort_assert(i >= 0);
	if (i != max)
		memmove(&elements[i], &elements[i + 1], sizeof(T) * (max - i));
	max--;
}

template <class T>
typename SArray<T>::iterator
SArray<T>::erase(iterator pos)
{
	int i = pos.pos - elements;
	erase(i);
	return iterator(&elements[i]);
}

template <class T>
typename SArray<T>::iterator
SArray<T>::erase(const_iterator pos)
{
	int i = pos.pos - elements;
	erase(i);
	return iterator(&elements[i]);
}

template <class T>
typename SArray<T>::iterator
SArray<T>::erase(iterator first, iterator last)
{
	int i = first.pos - elements;
	int j = last.pos - elements;
	const int num = j - i;
	if (i != max)
		memmove(&elements[i], &elements[i + num], sizeof(T*) * (max - i));
	max -= num;
	return iterator(&elements[i]);
}

template <class T>
typename SArray<T>::iterator
SArray<T>::erase(const_iterator first, const_iterator last)
{
	int i = first.pos - elements;
	int j = last.pos - elements;
	const int num = j - i;
	if (i != max)
		memmove(&elements[i], &elements[i + num], sizeof(T*) * (max - i));
	max -= num;
	return iterator(&elements[i]);
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
SArray<T>::push_front(T i) {
	setsize(max + 1);
	memmove(&elements[i + 1], &elements[i], sizeof(T) * (max - i + 1));
	max++;
	elements[0] = i;
}

template <class T>
void
SArray<T>::push_back(T i) {
	setsize(max + 1);
	elements[++max] = i;
}

template <class T>
void
SArray<T>::emplace_back(T i) {
	setsize(max + 1);
	elements[++max] = i;
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
		memmove(&elements[i + 1], &elements[i], sizeof(T) * (max - i + 1));
		max++;
		elements[i] = 0;
	}
	check();
}

template <class T>
typename SArray<T>::iterator
SArray<T>::insert(const_iterator pos, const T& value)
{
	int i = pos.pos - elements;
	setsize(max + 1);
	if (i <= max) {
		memmove(&elements[i + 1], &elements[i], sizeof(T) * (max - i + 1));
		max++;
	}
	elements[i] = value;
	return iterator(&elements[i]);
}

template <class T>
typename SArray<T>::iterator
SArray<T>::insert(const_iterator pos, int count, const T& value)
{
	int i = pos.pos - elements;
	setsize(max + count);
	if (i <= max) {
		memmove(&elements[i + count], &elements[i], sizeof(T) * (max - i + 1));
		max++;
	}
	for (int x = i; x < count; ++x) {
		elements[i] = value;
	}
	return iterator(&elements[i]);
}

template <class T>
template<class InputIt>
typename SArray<T>::iterator
SArray<T>::insert(const_iterator pos, InputIt first, InputIt last)
{
	int i = pos.pos - elements;
	for (auto x = first; x != last; ++x) {
		pos = insert(pos, *x);
		++pos;
	}
	return iterator(&elements[i]);
}

template <class T>
typename SArray<T>::iterator
SArray<T>::insert(const_iterator pos, std::initializer_list<T> ilist)
{
	int i = pos.pos - elements;
	for (auto x = ilist.begin(); x != ilist.end(); ++x) {
		pos = insert(pos, T(*x));
		++pos;
	}
	return iterator(&elements[i]);
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
SArray<T>::getsize() noexcept {
	return max;
}

template <class T>
void
SArray<T>::clear() noexcept {
	max = -1;
}

template <class T>
T&
SArray<T>::getlast() {
	return elements[max];
}

template <class T>
int
SArray<T>::indexof(T value) noexcept {
	for (int i = 0; i <= max; i++) {
		if (elements[i] == value)
			return i;
	}
	return -1;
}

#endif /* !_SARRAY */
