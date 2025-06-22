/*
 * Copyright (c) 2001,02,03,04,08 Bernd Walter Computer Technology
 * Copyright (c) 2008 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/array.h $
 * $Date: 2025-06-10 19:26:00 +0200 (Tue, 10 Jun 2025) $
 * $Author: ticso $
 * $Rev: 49347 $
 */

#ifndef _ARRAY
#define _ARRAY

#include "tool.h"
#include <iterator>

template <class T>
class Array {
private:
	int num_elem;
protected:
	T **elements;
	void setsize(const int i);
public:
	template <bool IsConst>
	struct Iterator {
		using iterator_category = std::random_access_iterator_tag;
		using value_type      = typename std::conditional_t<IsConst, const T, T>;
		using reference         = value_type&;
		using pointer           = value_type*;
		using difference_type   = std::ptrdiff_t;

		friend class Array<T>;

	private:
		T** pos;

	public:
		Iterator(const Iterator&) = default;
		Iterator& operator=(const Iterator&) = default;

		template<bool WasConst, class = std::enable_if_t<IsConst && !WasConst>>
		Iterator(const Iterator<WasConst>& rhs) : pos(rhs.pos) {}

		template<bool WasConst, class = std::enable_if_t<IsConst && !WasConst>>
		Iterator& operator=(const Iterator<WasConst>& rhs) {
			pos = rhs.pos;
			return (*this);
		}

		Iterator(T** rpos) {
			pos = rpos;
		}

		pointer operator->() const {
			return *pos;
		}

		reference operator*() const {
			return **pos;
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

	auto begin() {
		return iterator(elements);
	}
	auto end() {
		return iterator(&elements[max + 1]);
	}
	auto begin() const {
		return const_iterator(elements);
	}
	auto end() const {
		return const_iterator(&elements[max + 1]);
	}
	auto cbegin() const {
		return const_iterator(elements);
	}
	auto cend() const {
		return const_iterator(&elements[max + 1]);
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
	Array() noexcept;
	Array(const Array &src);
	Array(Array &&src) noexcept;
	Array(std::initializer_list<T> ilist);
	~Array() noexcept;
	void clear() noexcept;
	void erase(const int i) noexcept;
	iterator erase(iterator pos);
	iterator erase(const_iterator pos);
	iterator erase(iterator first, iterator last);
	iterator erase(const_iterator first, const_iterator last);
	void insert(const int i);
	iterator insert(const_iterator pos, const T& value);
	iterator insert(const_iterator pos, T&& value );
	iterator insert(const_iterator pos, int count, const T& value);
	template<class InputIt>
	iterator insert(const_iterator pos, InputIt first, InputIt last);
	iterator insert(const_iterator pos, std::initializer_list<T> ilist);
	void clear(const int i);
	T* cutptr(const int i);
	void pasteptr(const int i, T* ptr);
	bool operator==(const Array &lh) const noexcept;
	bool operator!=(const Array &lh) const noexcept;
	const Array<T>& operator=(const Array &lh);
	const Array<T>& operator=(Array &&lh) noexcept;
	const Array<T>& operator=(std::initializer_list<T> ilist);
	T& operator[](const int i);
	const T& operator[](const int i) const;
	void push_back(const T &rh);
	void push_back(T &&rh);
	template <typename... Args>
	void emplace_back(Args&&... args);
	void push_front(const T &rh);
	void push_front(T &&rh);
	template <typename... Args>
	void emplace_front(Args&&... args);
	void pop_front();
	void pop_back();
	T& operator<<(const T &rh);
	T& operator<<(T &&rh);
	T& operator<<(const Array<T> &rh);
	T& operator<<(Array<T> &&rh);
	bool exists(const int i) noexcept;
	int getsize() noexcept;
	T& getlast();
	T merge(const T& separator);
	int size() const {
		return max + 1;
	}
	void sort() noexcept;
	void sort(int (*func) (const T**, const T**)) noexcept;
	void shuffle() noexcept;
	int indexof(T &value) const noexcept;
	String tinfo() const;
};

template <class T>
Array<T>::Array() noexcept {
	static_assert(!std::is_scalar<T>::value, "type must not be scalar");

	max = -1;
	num_elem = 0;
	elements = NULL;
}

template <class T>
Array<T>::Array(const Array &src)
{
	static_assert(!std::is_scalar<T>::value, "type must not be scalar");

	max = -1;
	num_elem = 0;
	elements = NULL;
	try {
		*this = src;
	} catch (...) {
		clear();
		throw;
	}
}

template <class T>
Array<T>::Array(Array &&src) noexcept
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
Array<T>::Array(std::initializer_list<T> ilist)
{
	max = -1;
	num_elem = 0;
	elements = NULL;
	for (auto& x: ilist) {
		(*this).emplace_back(x);
	}
}

template <class T>
Array<T>::~Array() noexcept {
	clear();
}

template <class T>
void Array<T>::clear() noexcept {
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
	setsize(max + 1);
	if (i <= max) {
		memmove(&elements[i + 1], &elements[i], sizeof(T*) * (max - i + 1));
		max++;
		elements[i] = NULL;
	}
}

template <class T>
typename Array<T>::iterator
Array<T>::insert(const_iterator pos, const T& value)
{
	int i = pos.pos - elements;
	setsize(max + 1);
	if (i <= max) {
		memmove(&elements[i + 1], &elements[i], sizeof(T*) * (max - i + 1));
		max++;
		elements[i] = NULL;
	}
	elements[i] = new T(value);
	return iterator(&elements[i]);
}

template <class T>
typename Array<T>::iterator
Array<T>::insert(const_iterator pos, T&& value )
{
	int i = pos.pos - elements;
	setsize(max + 1);
	if (i <= max) {
		memmove(&elements[i + 1], &elements[i], sizeof(T*) * (max - i + 1));
		max++;
		elements[i] = NULL;
	}
	elements[i] = new T(std::move(value));
	return iterator(&elements[i]);
}

template <class T>
typename Array<T>::iterator
Array<T>::insert(const_iterator pos, int count, const T& value)
{
	int i = pos.pos - elements;
	setsize(max + count);
	if (i <= max) {
		memmove(&elements[i + count], &elements[i], sizeof(T*) * (max - i + 1));
		max++;
		for (int x = i; x < count; ++x) {
			elements[i + x] = NULL;
		}
	}
	for (int x = i; x < count; ++x) {
		elements[i] = new T(value);
	}
	return iterator(&elements[i]);
}

template <class T>
template<class InputIt>
typename Array<T>::iterator
Array<T>::insert(const_iterator pos, InputIt first, InputIt last)
{
	int i = pos.pos - elements;
	for (auto x = first; x != last; ++x) {
		pos = insert(pos, *x);
		++pos;
	}
	return iterator(&elements[i]);
}

template <class T>
typename Array<T>::iterator
Array<T>::insert(const_iterator pos, std::initializer_list<T> ilist)
{
	int i = pos.pos - elements;
	for (auto x = ilist.begin(); x != ilist.end(); ++x) {
		pos = insert(pos, T(*x));
		++pos;
	}
	return iterator(&elements[i]);
}

template <class T>
void
Array<T>::sort() noexcept
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
Array<T>::sort(int (*func) (const T**, const T**)) noexcept
{
	::qsort(elements, max + 1, sizeof(T*), (int (*)(const void*, const void*))func);
}

template <class T>
void
Array<T>::shuffle() noexcept
{
	Array<T> tmp = std::move(*this);
	clear();
	while(tmp.max >= 0) {
		int64_t i = getrandom() % (tmp.max + 1);
		(*this)[max + 1] = std::move(tmp[i]);
		tmp.erase(i);
	}
}

template <class T>
void
Array<T>::erase(const int i) noexcept {
	abort_assert(i <= max);
	abort_assert(i >= 0);
	delete elements[i];
	if (i != max)
		memmove(&elements[i], &elements[i + 1], sizeof(T*) * (max - i));
	elements[max] = NULL;
	max--;
}

template <class T>
typename Array<T>::iterator
Array<T>::erase(iterator pos)
{
	int i = pos.pos - elements;
	erase(i);
	return iterator(&elements[i]);
}

template <class T>
typename Array<T>::iterator
Array<T>::erase(const_iterator pos)
{
	int i = pos.pos - elements;
	erase(i);
	return iterator(&elements[i]);
}

template <class T>
typename Array<T>::iterator
Array<T>::erase(iterator first, iterator last)
{
	int i = first.pos - elements;
	int j = last.pos - elements;
	const int num = j - i;
	for (int x = i; x < j; ++x) {
		delete elements[x];
	}
	if (i != max)
		memmove(&elements[i], &elements[i + num], sizeof(T*) * (max - i));
	max -= num;
	return iterator(&elements[i]);
}

template <class T>
typename Array<T>::iterator
Array<T>::erase(const_iterator first, const_iterator last)
{
	int i = first.pos - elements;
	int j = last.pos - elements;
	const int num = j - i;
	for (int x = i; x < j; ++x) {
		delete elements[x];
	}
	if (i != max)
		memmove(&elements[i], &elements[i + num], sizeof(T*) * (max - i));
	max -= num;
	return iterator(&elements[i]);
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
	}
}

template <class T>
bool
Array<T>::operator==(const Array &lh) const noexcept
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
Array<T>::operator!=(const Array &lh) const noexcept
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
Array<T>::operator=(Array &&lh) noexcept
{
	std::swap(num_elem, lh.num_elem);
	std::swap(max, lh.max);
	std::swap(elements, lh.elements);
	return *this;
}

template <class T>
const Array<T>& Array<T>::operator=(std::initializer_list<T> ilist)
{
	clear();
	for (auto& x: ilist) {
		(*this).emplace_back(x);
	}
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
	Array<T> *me = const_cast<Array<T>*>(this);
	if (!elements[i]) {
		TError(S + "index " + i + "out of range");
	}
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
void
Array<T>::push_back(const T& rh) {
	setsize(max + 1);
	max++;
	elements[max] = new T(rh);
}

template <class T>
void
Array<T>::push_back(T&& rh) {
	setsize(max + 1);
	max++;
	elements[max] = new T(std::forward<T>(rh));
}

template <class T>
template <typename... Args>
void
Array<T>::emplace_back(Args&&... args)
{
	setsize(max + 1);
	max++;
	elements[max] = new T(std::forward<Args>(args)...);
}

template <class T>
void
Array<T>::push_front(const T& rh) {
	setsize(max + 1);
	memmove(&elements[1], &elements[0], sizeof(T*) * (max + 1));
	max++;
	elements[0] = new T(rh);
}

template <class T>
void
Array<T>::push_front(T&& rh) {
	setsize(max + 1);
	memmove(&elements[1], &elements[0], sizeof(T*) * (max + 1));
	max++;
	elements[0] = new T(std::forward<T>(rh));
}

template <class T>
template <typename... Args>
void
Array<T>::emplace_front(Args&&... args)
{
	setsize(max + 1);
	memmove(&elements[1], &elements[0], sizeof(T*) * (max + 1));
	max++;
	elements[0] = new T(std::forward<Args>(args)...);
}

template <class T>
void
Array<T>::pop_front() {
	if (max < 0) {
		return;
	}
	delete elements[0];
	if (0 != max)
		memmove(&elements[0], &elements[1], sizeof(T*) * (max));
	elements[max] = NULL;
	--max;
}

template <class T>
void
Array<T>::pop_back() {
	delete elements[max];
	elements[max] = NULL;
	max--;
}

template <class T>
T&
Array<T>::operator<<(const T& rh) {
	setsize(max + 1);
	max++;
	elements[max] = new T(rh);
	return *elements[max];
}

template <class T>
T&
Array<T>::operator<<(T&& rh) {
	setsize(max + 1);
	max++;
	elements[max] = new T(std::forward<T>(rh));
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
bool
Array<T>::exists(const int i) noexcept {
	if (i > max || i < 0)
		return 0;
	return (elements[i] != NULL);
}

template <class T>
int
Array<T>::getsize() noexcept {
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
Array<T>::indexof(T &value) const noexcept {
	for (int i = 0; i <= max; i++) {
		if (elements[i] != NULL && *elements[i] == value)
			return i;
	}
	return -1;
}

template <class T>
String
Array<T>::tinfo() const
{
	String ret;
	ret << "(" << typeid(*this).name() << "@" << this << ")";
	return ret;
}

extern template class Array<String>;

#endif /* !_ARRAY */
