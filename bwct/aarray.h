/*
 * Copyright (c) 2001-2014 Bernd Walter Computer Technology
 * Copyright (c) 2008-2014 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/aarray.h $
 * $Date: 2022-05-11 00:54:26 +0200 (Wed, 11 May 2022) $
 * $Author: ticso $
 * $Rev: 45533 $
 */

#ifndef _AARRAY
#define _AARRAY

#include "tool.h"

template <class T>
class AArray : public Base {
private:
	class Elem {
	public:
		T* data;
		Elem* next;
		String key;

		Elem() {
			data = NULL;
			next = NULL;
		}
		~Elem() {
			delete next;
			delete data;
		}
	};
	Elem** elems;
	int buckets;
	int getbucket(uint32_t h) const;

public:
#if 0
	struct Iterator {
		using iterator_category = std::forward_iterator_tag;

		Iterator(AArray<T>* ref, Elem* elem, int bucket) {
			a = ref;
			e = elem;
			b = bucket;
		}

		struct retVals {
			String& key;
			T& data;
		};
		retVals operator*() const {
			return retVals{e->key, *(e->data)};
		}
		Iterator& operator++() {
			e = e->next;
			while (e == NULL) {
				b++;
				if (b >= a->buckets) {
					return *this;
				}
			}
			return *this;
		}
		Iterator operator++(int) {
			Iterator tmp = *this;
			++(*this);
			return tmp;
		}
		friend bool operator== (const Iterator& a, const Iterator& b) {
			return a.elem == b.elem;
		}
		friend bool operator!= (const Iterator& a, const Iterator& b) {
			return a.elem != b.elem;
		}

	private:
		AArray<T>* a;
		Elem* e;
		int b;
	};

	Iterator begin() {
		if (elems == NULL) {
			return Iterator(this, NULL, buckets);
		}
		return Iterator(this, elems[0], 0);
	}
	Iterator end() {
		return Iterator(this, NULL, buckets);
	}
#endif

	AArray(int buckets = 4);
	AArray(AArray<T>&& src);
	AArray(const AArray<T>& src);
	~AArray();
	void empty();
	void del(const String& key);
	T& operator[](const String& key);
	const T& operator[](const String& key) const;
	const AArray<T>& operator=(const AArray<T>& rhs);
	const AArray<T>& operator=(AArray<T>&& rhs);
	bool exists(const String& key) const;
	T* getexistingptr(const String& key);
	Array<String> getkeys(bool sorted = false) const;
	Array<String> getfiltkeys(bool (*func) (const T&), bool sorted = false) const;
};

template <class T>
AArray<T>::AArray(int buckets)
{
	this->buckets = buckets;
	elems = new Elem*[buckets];
	bzero(elems, sizeof(Elem*) * buckets);
}

template <class T>
AArray<T>::~AArray()
{
	if (elems != NULL) {
		empty();
		delete[] elems;
		elems = NULL;
	}
}

template <class T>
AArray<T>::AArray(AArray<T>&& src)
{
	buckets = src.buckets;
	elems = src.elems;
	src.elems = NULL;
}

template <class T>
AArray<T>::AArray(const AArray<T>& src)
{
	buckets = src.buckets;
	elems = NULL;

	try {
		elems = new Elem*[buckets];
		bzero(elems, sizeof(Elem*) * buckets);
		for (int i = 0; i < buckets; i++) {
			if (src.elems[i] != NULL) {
				elems[i] = new Elem;
				elems[i]->key = src.elems[i]->key;
				elems[i]->data = new T;
				*elems[i]->data = *src.elems[i]->data;
				Elem** ep = &elems[i];
				Elem* se = src.elems[i]->next;
				while (se != NULL) {
					(*ep)->next = new Elem;
					(*ep)->next->key = se->key;
					(*ep)->next->data = new T;
					*(*ep)->next->data = *se->data;
					ep = &(*ep)->next;
					se = se->next;
				}
			}
		}
	} catch(...) {
		empty();
		throw;
	}
}

template <class T>
const AArray<T>&
AArray<T>::operator=(AArray<T>&& rhs)
{
	empty();
	delete[] elems;
	elems = NULL;

	buckets = rhs.buckets;
	elems = rhs.elems;

	rhs.elems = NULL;
	return *this;
}

template <class T>
const AArray<T>&
AArray<T>::operator=(const AArray<T>& rhs)
{
	empty();
	buckets = rhs.buckets;
	delete[] elems;
	elems = NULL;
	elems = new Elem*[buckets];
	bzero(elems, sizeof(Elem*) * buckets);
	for (int i = 0; i < buckets; i++) {
		if (rhs.elems[i] != NULL) {
			elems[i] = new Elem;
			elems[i]->key = rhs.elems[i]->key;
			elems[i]->data = new T;
			*elems[i]->data = *rhs.elems[i]->data;
			Elem** ep = &elems[i];
			Elem* se = rhs.elems[i]->next;
			while (se != NULL) {
				(*ep)->next = new Elem;
				(*ep)->next->key = se->key;
				(*ep)->next->data = new T;
				*(*ep)->next->data = *se->data;
				ep = &(*ep)->next;
				se = se->next;
			}
		}
	}
	return *this;
}

template <class T>
void
AArray<T>::empty()
{
	if (elems != NULL) {
		for (int i = 0; i < buckets; i++) {
			delete elems[i];
			elems[i] = NULL;
		}
	}
}

template <class T>
int
AArray<T>::getbucket(uint32_t h) const
{
	int ret;
	uint32_t mask;

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
			TError("unsupported buckets");
	}
	return ret;
}

template <class T>
Array<String>
AArray<T>::getfiltkeys(bool (*func) (const T&), bool sorted) const
{
	Array<String> ret;
	for (int i = 0; i < buckets; i++) {
		Elem* e = elems[i];
		while (e != NULL) {
			if (func(*(e->data))) {
				ret << e->key;
			}
			e = e->next;
		}
	}

	if (sorted) {
		ret.sort();
	}

	return ret;
}

template <class T>
Array<String>
AArray<T>::getkeys(bool sorted) const
{
	Array<String> ret;
	for (int i = 0; i < buckets; i++) {
		Elem* e = elems[i];
		while (e != NULL) {
			ret << e->key;
			e = e->next;
		}
	}

	if (sorted) {
		ret.sort();
	}

	return ret;
}

template <class T>
void
AArray<T>::del(const String& key)
{
	uint32_t h = crc_hash(key.c_str(), key.length(), 0);
	int bucket = getbucket(h);

	Elem** ep = &elems[bucket];
	while (*ep != NULL) {
		Elem* e = *ep;
		if (e->key == key) {
			*ep = e->next;
			e->next = NULL;
			delete e;
		}
		ep = &e->next;
	}
}

template <class T>
T&
AArray<T>::operator[](const String& key)
{
	uint32_t h = crc_hash(key.c_str(), key.length(), 0);
	int bucket = getbucket(h);

	Elem* e = elems[bucket];
	if (e == NULL) {
		e = new Elem;
		elems[bucket] = e;
		e->key = key;
		e->data = new T;
	} else {
		while (e != NULL) {
			if (e->key == key) {
				break;
			}
			if (e->next == NULL) {
				e->next = new Elem;
				e->next->key = key;
				e->next->data = new T;
				return *e->next->data;
			}
			e = e->next;
		}
	}

	return *e->data;
}

template <class T>
const T&
AArray<T>::operator[](const String& key) const
{
	uint32_t h = crc_hash(key.c_str(), key.length(), 0);
	int bucket = getbucket(h);

	Elem* e = elems[bucket];
	while (e != NULL) {
		if (e->key == key) {
			break;
		}
		e = e->next;
	}
	if (e == NULL) {
		TError(String() + "key " + key + " does not exist");
	}

	return *e->data;
}

template <class T>
T*
AArray<T>::getexistingptr(const String& key)
{
	uint32_t h = crc_hash(key.c_str(), key.length(), 0);
	int bucket = getbucket(h);

	Elem* e = elems[bucket];
	while (e != NULL) {
		if (e->key == key) {
			break;
		}
		e = e->next;
	}
	if (e == NULL) {
		return NULL;
	}

	return e->data;
}

template <class T>
bool
AArray<T>::exists(const String& key) const
{
	uint32_t h = crc_hash(key.c_str(), key.length(), 0);
	int bucket = getbucket(h);

	Elem* e = elems[bucket];
	while (e != NULL) {
		if (e->key == key) {
			return true;
		}
		e = e->next;
	}

	return false;
}

#endif /* !_AARRAY */
