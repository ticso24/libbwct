/*
 * Copyright (c) 2001-2014 Bernd Walter Computer Technology
 * Copyright (c) 2008-2014 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/aarray.h $
 * $Date: 2025-06-10 19:26:00 +0200 (Tue, 10 Jun 2025) $
 * $Author: ticso $
 * $Rev: 49347 $
 */

#ifndef _AARRAY
#define _AARRAY

#include "tool.h"

namespace bwct
{
	template <class T, class K = String, size_t buckets = 4>
	class AArray {
	private:
		class Elem {
		public:
			std::pair<K, T> data;
			Elem* next;

			Elem() noexcept {
				next = NULL;
			}
			~Elem() noexcept {
				delete next;
			}
		};
		Elem* elems[buckets];
		static constexpr int getbucket(uint32_t h) noexcept;

	public:

		template <bool IsConst>
		struct Iterator {
			using iterator_category = std::forward_iterator_tag;
			using value_type      = typename std::conditional_t<IsConst, const std::pair<K, T>, std::pair<K, T>>;
			using reference         = value_type&;
			using pointer           = value_type*;

		private:
			AArray<T, K, buckets>* a;
			Elem* e;
			size_t b;

		public:
			Iterator(const Iterator&) = default;
			Iterator& operator=(const Iterator&) = default;

			template<bool WasConst, class = std::enable_if_t<IsConst && !WasConst>>
			Iterator(const Iterator<WasConst>& rhs) {
				a = rhs.a;
				e = rhs.e;
				b = rhs.b;
			}

			Iterator(const AArray<T, K, buckets>* ref, const Elem* elem, int bucket) {
				a = const_cast<AArray<T, K, buckets>*>(ref);
				e = const_cast<Elem*>(elem);
				b = bucket;
			}

			template<bool WasConst, class = std::enable_if_t<IsConst && !WasConst>>
			Iterator& operator=(const Iterator<WasConst>& rhs) {
				a = rhs.a;
				e = rhs.e;
				b = rhs.b;
				return (*this);
			}

			pointer operator->() const {
				return &(e->data);
			}

			reference operator*() const {
				return (e->data);
			}

			Iterator& operator++() {
				e = e->next;
				while (e == NULL) {
					b++;
					if (b >= buckets) {
						e = NULL;
						break;
					}
					e = a->elems[b];
				}
				return *this;
			}

			Iterator operator++(int) {
				Iterator tmp = *this;
				++(*this);
				return tmp;
			}

			friend bool operator== (const Iterator& a, const Iterator& b) {
				return a.e == b.e;
			}

			friend bool operator!= (const Iterator& a, const Iterator& b) {
				return a.e != b.e;
			}

		};

		using iterator = Iterator<false>;
		using const_iterator = Iterator<true>;

		iterator begin() {
			Elem* elem;
			size_t bucket;
			for (bucket = 0; bucket < buckets; ++bucket) {
				elem = elems[bucket];
				if (elem != NULL) {
					break;
				}
			}
			return iterator(this, elem, bucket);
		}

		iterator end() {
			return iterator(this, NULL, buckets);
		}

		const_iterator cbegin() const {
			Elem* elem;
			size_t bucket;
			for (bucket = 0; bucket < buckets; ++bucket) {
				elem = elems[bucket];
				if (elem != NULL) {
					break;
				}
			}
			return const_iterator(this, elem, bucket);
		}

		const_iterator cend() const {
			return const_iterator(this, NULL, buckets);
		}

		const_iterator begin() const {
			return cbegin();
		}

		const_iterator end() const {
			return cend();
		}

		AArray() noexcept;
		AArray(AArray<T, K, buckets>&& src) noexcept;
		AArray(const AArray<T, K, buckets>& src);
		AArray(std::initializer_list<std::pair<K, T>> ilist);
		~AArray() noexcept;
		void clear() noexcept;
		int erase(const K& key) noexcept;
		iterator erase(iterator pos);
		iterator erase(const_iterator pos);
		iterator erase(iterator first, iterator last);
		iterator erase(const_iterator first, const_iterator last);
		bool empty() noexcept;
		T& operator[](const K& key);
		const T& operator[](const K& key) const;
		const AArray<T, K, buckets>& operator=(const AArray<T, K, buckets>& rhs);
		const AArray<T, K, buckets>& operator=(AArray<T, K, buckets>&& rhs) noexcept;
		const AArray<T, K, buckets>& operator=(std::initializer_list<std::pair<K, T>> ilist);
		bool exists(const K& key) const noexcept;
		T* getexistingptr(const K& key) noexcept;
		Array<K> getkeys(bool sorted = false) const;
		Array<K> getfiltkeys(bool (*func) (const T&), bool sorted = false) const;
		Array<std::pair<K*, T*>> getpairs(bool sorted = false) const;
		template <typename... Args>
		void emplace(Args&&... args);
		String tinfo() const;
		int size() const {
			int ret = -1;
			for (auto& x: (*this)) {
				++ret;
			}
			return ret;
		}
		iterator insert(const T& value);
		iterator insert(T&& value );
		iterator insert(const_iterator pos, const T& value);
		iterator insert(const_iterator pos, T&& value );
		template<class InputIt>
		iterator insert(const_iterator pos, InputIt first, InputIt last);
		iterator insert(const_iterator pos, std::initializer_list<T> ilist);
	};

	template <class T, class K, size_t buckets>
	AArray<T, K, buckets>::AArray() noexcept
	{
		bzero(elems, sizeof(void*) * buckets);
	}

	template <class T, class K, size_t buckets>
	AArray<T, K, buckets>::~AArray() noexcept
	{
		clear();
	}

	template <class T, class K, size_t buckets>
	AArray<T, K, buckets>::AArray(AArray<T, K, buckets>&& src) noexcept
	{
		for (size_t i = 0; i < buckets; i++) {
			elems[i] = src.elems[i];
			src.elems[i] = NULL;
		}
	}

	template <class T, class K, size_t buckets>
	AArray<T, K, buckets>::AArray(const AArray<T, K, buckets>& src)
	{
		try {
			bzero(elems, sizeof(Elem*) * buckets);
			for (size_t i = 0; i < buckets; i++) {
				if (src.elems[i] != NULL) {
					elems[i] = new Elem;
					elems[i]->data = src.elems[i]->data;
					Elem** ep = &elems[i];
					Elem* se = src.elems[i]->next;
					while (se != NULL) {
						(*ep)->next = new Elem;
						(*ep)->next->data = se->data;
						ep = &(*ep)->next;
						se = se->next;
					}
				}
			}
		} catch(...) {
			clear();
			throw;
		}
	}

	template <class T, class K, size_t buckets>
	AArray<T, K, buckets>::AArray(std::initializer_list<std::pair<K, T>> ilist)
	{
		try {
			bzero(elems, sizeof(void*) * buckets);
			for (auto& x: ilist) {
				auto h = std::hash<K>{}(x.first);
				int bucket = getbucket(h);

				Elem* e = elems[bucket];
				if (e == NULL) {
					e = new Elem;
					elems[bucket] = e;
					e->data = x;
				} else {
					while (e != NULL) {
						if (e->data.first == x.first) {
							break;
						}
						if (e->next == NULL) {
							e->next = new Elem;
							e->next->data = x;
							break;
						}
						e = e->next;
					}
				}
			}
		} catch(...) {
			clear();
			throw;
		}
	}

	template <class T, class K, size_t buckets>
	const AArray<T, K, buckets>&
	AArray<T, K, buckets>::operator=(AArray<T, K, buckets>&& rhs) noexcept
	{
		clear();

		for (size_t i = 0; i < buckets; i++) {
			elems[i] = rhs.elems[i];
			rhs.elems[i] = NULL;
		}

		return *this;
	}

	template <class T, class K, size_t buckets>
	const AArray<T, K, buckets>&
	AArray<T, K, buckets>::operator=(const AArray<T, K, buckets>& rhs)
	{
		clear();
		for (size_t i = 0; i < buckets; i++) {
			if (rhs.elems[i] != NULL) {
				elems[i] = new Elem;
				elems[i]->data = rhs.elems[i]->data;
				Elem** ep = &elems[i];
				Elem* se = rhs.elems[i]->next;
				while (se != NULL) {
					(*ep)->next = new Elem;
					(*ep)->next->data = se->data;
					ep = &(*ep)->next;
					se = se->next;
				}
			}
		}
		return *this;
	}

	template <class T, class K, size_t buckets>
	const AArray<T, K, buckets>&
	AArray<T, K, buckets>::operator=(std::initializer_list<std::pair<K, T>> ilist)
	{
		for (auto& x: ilist) {
			auto h = std::hash<K>{}(x.first);
			int bucket = getbucket(h);

			Elem* e = elems[bucket];
			if (e == NULL) {
				e = new Elem;
				elems[bucket] = e;
				e->data = x;
			} else {
				while (e != NULL) {
					if (e->data.first == x.first) {
						break;
					}
					if (e->next == NULL) {
						e->next = new Elem;
						e->next->data = x;
						break;
					}
					e = e->next;
				}
			}
		}
		return *this;
	}

	template <class T, class K, size_t buckets>
	void
	AArray<T, K, buckets>::clear() noexcept
	{
		for (size_t i = 0; i < buckets; i++) {
			delete elems[i];
			elems[i] = NULL;
		}
	}

	template <class T, class K, size_t buckets>
	bool
	AArray<T, K, buckets>::empty() noexcept
	{
		for (size_t i = 0; i < buckets; i++) {
			if (elems[i] != NULL) {
				return false;
			}
		}
		return true;
	}

	template <class T, class K, size_t buckets>
	constexpr int
	AArray<T, K, buckets>::getbucket(uint32_t h) noexcept
	{
		int ret = 0;
		uint32_t mask = 0;

		switch (buckets) {
			case 2:
			case 4:
			case 8:
			case 16:
			case 32:
			case 64:
			case 128:
			case 256:
			case 512:
			case 1024:
			case 2048:
			case 4096:
			case 8192:
			case 16384:
			case 32768:
			case 65536:
				mask = buckets - 1;
				ret = h & mask;
				break;
			default:
				static_assert("unsupported buckets");
		}
		return ret;
	}

	template <class T, class K, size_t buckets>
	Array<K>
	AArray<T, K, buckets>::getfiltkeys(bool (*func) (const T&), bool sorted) const
	{
		Array<K> ret;
		for (size_t i = 0; i < buckets; i++) {
			Elem* e = elems[i];
			while (e != NULL) {
				if (func(e->data.second)) {
					ret.emplace_back(e->data.first);
				}
				e = e->next;
			}
		}

		if (sorted) {
			ret.sort();
		}

		return ret;
	}

	template <class T, class K, size_t buckets>
	Array<K>
	AArray<T, K, buckets>::getkeys(bool sorted) const
	{
		Array<K> ret;
		for (size_t i = 0; i < buckets; i++) {
			Elem* e = elems[i];
			while (e != NULL) {
				ret.emplace_back(e->data.first);
				e = e->next;
			}
		}

		if (sorted) {
			ret.sort();
		}

		return ret;
	}

	template <class T, class K, size_t buckets>
	Array<std::pair<K*, T*>>
	AArray<T, K, buckets>::getpairs(bool sorted) const
	{
		Array<std::pair<K*, T*>> ret;
		for (size_t i = 0; i < buckets; i++) {
			Elem* e = elems[i];
			while (e != NULL) {
				ret.emplace_back(std::make_pair<K*, T*> (&(e->data.first), &(e->data.second)));
				e = e->next;
			}
		}

		if (sorted) {
			auto helper = [] (const std::pair<K*, T*>** a, const std::pair<K*, T*>** b) {
				int ret = 0;
				if (*(**a).first < *(**b).first) {
					ret = -1;
				} else if (*(**a).first > *(**b).first) {
					ret = +1;
				}
				return ret;
			};
			ret.sort(helper);
		}

		return ret;
	}

	template <class T, class K, size_t buckets>
	int
	AArray<T, K, buckets>::erase(const K& key) noexcept
	{
		auto h = std::hash<K>{}(key);
		int bucket = getbucket(h);

		Elem** ep = &elems[bucket];
		while (*ep != NULL) {
			Elem* e = *ep;
			if (e->data.first == key) {
				*ep = e->next;
				e->next = NULL;
				delete e;
				return 1;
			}
			ep = &e->next;
		}
		return 0;
	}

	template <class T, class K, size_t buckets>
	typename AArray<T, K, buckets>::iterator
	AArray<T, K, buckets>::erase(iterator pos)
	{
		auto ret = pos + 1;
		erase(*pos.first);
		return ret;
	}

	template <class T, class K, size_t buckets>
	typename AArray<T, K, buckets>::iterator
	AArray<T, K, buckets>::erase(const_iterator pos)
	{
		auto ret = pos + 1;
		erase(*pos.first);
		return ret;
	}

	template <class T, class K, size_t buckets>
	typename AArray<T, K, buckets>::iterator
	AArray<T, K, buckets>::erase(iterator first, iterator last)
	{
		// TODO optimize
		Array<K> tmp;
		for (auto x = first; x != last; ++x) {
			tmp.pushback(x->first);
		}

		for (auto& x: tmp) {
			erase(std::move(x));
		}
		return last;
	}

	template <class T, class K, size_t buckets>
	typename AArray<T, K, buckets>::iterator
	AArray<T, K, buckets>::erase(const_iterator first, const_iterator last)
	{
		// TODO optimize
		Array<K> tmp;
		for (auto x = first; x != last; ++x) {
			tmp.pushback(x->first);
		}

		for (auto& x: tmp) {
			erase(std::move(x));
		}
		return last;
	}

	template <class T, class K, size_t buckets>
	template <typename... Args>
	void
	AArray<T, K, buckets>::emplace(Args&&... args)
	{
		auto tmp = T(std::forward<Args>(args)...);
		auto h = std::hash<K>{}(tmp.first);
		int bucket = getbucket(h);

		Elem* e = elems[bucket];
		if (e == NULL) {
			e = new Elem;
			elems[bucket] = e;
		} else {
			while (e != NULL) {
				if (e->data.first == tmp.first) {
					break;
				}
				if (e->next == NULL) {
					e->next = new Elem;
					return;
				}
				e = e->next;
			}
		}
		e->data = std::move(tmp);
	}

	template <class T, class K, size_t buckets>
	T&
	AArray<T, K, buckets>::operator[](const K& key)
	{
		auto h = std::hash<K>{}(key);
		int bucket = getbucket(h);

		Elem* e = elems[bucket];
		if (e == NULL) {
			e = new Elem;
			elems[bucket] = e;
			e->data.first = key;
		} else {
			while (e != NULL) {
				if (e->data.first == key) {
					break;
				}
				if (e->next == NULL) {
					e->next = new Elem;
					e->next->data.first = key;
					return e->next->data.second;
				}
				e = e->next;
			}
		}

		return e->data.second;
	}

	template <class T, class K, size_t buckets>
	const T&
	AArray<T, K, buckets>::operator[](const K& key) const
	{
		auto h = std::hash<K>{}(key);
		int bucket = getbucket(h);

		Elem* e = elems[bucket];
		while (e != NULL) {
			if (e->data.first == key) {
				break;
			}
			e = e->next;
		}
		if (e == NULL) {
			TError(String() + "key " + key + " does not exist");
		}

		return e->data.second;
	}

	template <class T, class K, size_t buckets>
	T*
	AArray<T, K, buckets>::getexistingptr(const K& key) noexcept
	{
		auto h = std::hash<K>{}(key);
		int bucket = getbucket(h);

		Elem* e = elems[bucket];
		while (e != NULL) {
			if (e->data.first == key) {
				break;
			}
			e = e->next;
		}
		if (e == NULL) {
			return NULL;
		}

		return &e->data.second;
	}

	template <class T, class K, size_t buckets>
	bool
	AArray<T, K, buckets>::exists(const K& key) const noexcept
	{
		auto h = std::hash<K>{}(key);
		int bucket = getbucket(h);

		Elem* e = elems[bucket];
		while (e != NULL) {
			if (e->data.first == key) {
				return true;
			}
			e = e->next;
		}

		return false;
	}

	template <class T, class K, size_t buckets>
	String
	AArray<T, K, buckets>::tinfo() const
	{
		String ret;
		ret << "(" << typeid(*this).name() << "@" << this << ")";
		return ret;
	}

	template <class T, class K, size_t buckets>
	typename AArray<T, K, buckets>::iterator
	AArray<T, K, buckets>::insert(const T& value)
	{
		auto h = std::hash<K>{}(value.first);
		int bucket = getbucket(h);

		Elem* e = elems[bucket];
		if (e == NULL) {
			e = new Elem;
			elems[bucket] = e;
			e->data = value;
		} else {
			while (e != NULL) {
				if (e->data.first == value.first) {
					e->data.second = value.second;
					break;
				}
				if (e->next == NULL) {
					e->next = new Elem;
					e->next->data = value;
					break;
				}
				e = e->next;
			}
		}

		return iterator(this, e, bucket);
	}

	template <class T, class K, size_t buckets>
	typename AArray<T, K, buckets>::iterator
	AArray<T, K, buckets>::insert(T&& value )
	{
		auto h = std::hash<K>{}(value.first);
		int bucket = getbucket(h);

		Elem* e = elems[bucket];
		if (e == NULL) {
			e = new Elem;
			elems[bucket] = e;
			e->data = std::move(value);
		} else {
			while (e != NULL) {
				if (e->data.first == value.first) {
					e->data.second = std::move(value.second);
					break;
				}
				if (e->next == NULL) {
					e->next = new Elem;
					e->next->data = std::move(value);
					break;
				}
			e = e->next;
			}
		}

		return iterator(this, e, bucket);
	}

	template <class T, class K, size_t buckets>
	typename AArray<T, K, buckets>::iterator
	AArray<T, K, buckets>::insert(const_iterator pos, const T& value)
	{
		return insert(value);
	}

	template <class T, class K, size_t buckets>
	typename AArray<T, K, buckets>::iterator
	AArray<T, K, buckets>::insert(const_iterator pos, T&& value )
	{
		return insert(std::move(value));
	}

	template <class T, class K, size_t buckets>
	template<class InputIt>
	typename AArray<T, K, buckets>::iterator
	AArray<T, K, buckets>::insert(const_iterator pos, InputIt first, InputIt last)
	{
		for(auto i = first; i != last; ++i) {
			return insert(*i);
		}
		return end();
	}

	template <class T, class K, size_t buckets>
	typename AArray<T, K, buckets>::iterator
	AArray<T, K, buckets>::insert(const_iterator pos, std::initializer_list<T> ilist)
	{
		for(auto x: ilist) {
			return insert(T(x));
		}
		return end();
	}

}

#endif /* !_AARRAY */
