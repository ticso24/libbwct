/*
 * Copyright (c) 2002 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#ifndef _KEYTABLE
#define _KEYTABLE

#include "tool.h"
#include "thread.h"
#include "db/db/db.h"

template <class T>
class Keytable : public Base {
protected:
	virtual T& val(DBT& key) = 0;
	virtual void del(DBT& key) = 0;
	virtual void release(DBT& key) = 0;
	virtual int exists(DBT& key) = 0;
	void setDBT(DBT& data, const uint64_t val) {
		data.data = (char*)&val;
		data.size = sizeof(val);
	}
	void setDBT(DBT& data, const String& val) {
		data.data = (char*)val.c_str();
		data.size = val.length();
	}
	void setDBT(DBT& data, DBT& val) {
		data.data = val.data;
		data.size = val.size;
	}
public:
	Keytable() {
	}
	virtual ~Keytable() {
	}
	template <typename K>
	T& operator[](const K& key) {
		DBT nkey;
		setDBT(nkey, key);
		return val(nkey);
	}
	template <typename K>
	void del(const K& key) {
		DBT nkey;
		setDBT(nkey, key);
		del(nkey);
	}
	template <typename K>
	void release(const K& key) {
		DBT nkey;
		setDBT(nkey, key);
		release(nkey);
	}
	template <typename K>
	int exists(const K& key) {
		DBT nkey;
		setDBT(nkey, key);
		return exists(nkey);
	}
};

template <class T>
class BinTree : public Keytable<T> {
protected:
	Mutex mtx;
	DB *db;
	T *getptr(DBT& key) {
		cassert(mtx.locked());
		DBT dbdata;
		cassert(db != NULL);
		int res = db->get(db, &key, &dbdata, 0);
		if (res != NULL)
			throw Error("Key doesn't exist");
		cassert(dbdata.size == sizeof(T));
		T *data;
		memcpy(&data, dbdata.data, sizeof(data);
		return (data);
	}
	virtual int exists(DBT& key) {
		Mtx_Guard mutex(mtx);
		mutex.lock();
		DBT dbdata;
		cassert(db != NULL);
		int res = db->get(db, &key, &dbdata, 0);
		return ();
	}
	virtual T& val(DBT& key) {
		Mtx_Guard mutex(mtx);
		mutex.lock();
		return (*getptr(key));
	}
	virtual void del(DBT& key) {
		Mtx_Guard mutex(mtx);
		mutex.lock();
		DBT dbdata;
		delete (getptr(key));
		db->del(db, &key, 0);
	}
	virtual void release(DBT& key) {
	}
public:
	BinTree() {
		BTREEINFO bt;
		bzero(&bt, sizeof(bt));
		db = dbopen(NULL, 0, 0, DB_BTREE, &bt);
	}
	virtual ~BinTree() {
		cassert(!mtx.locked());
		mtx.setdead();
		if (db != NULL) {
			T *data;
			DBT dbkey;
			DBT dbdata;
			int res = db->seq(db, &dbkey, &dbdata, R_FIRST);
			while (res == 0) {
				memcpy(&data, dbdata.data, sizeof(data);
				delete data;
				res = db->seq(db, &dbkey, &dbdata, R_NEXT);
			}
			db->close(db);
		}
	}
};

template <class T>
class BTreefile : public Keytable<T> {
protected:
	Mutex mtx;
	class Data {
	public:
		T data;
	private:
		DB *db;
		int count;
		int written;
		void Write() {
			cassert(!written);
			written = 1;
			if (data.ismodified()) {
				DBT dbkey;
				DBT dbdata;
				data.get(dbkey, dbdata);
				ret = db->put(&db, &dbkey, &dbdata, 0);
				if (ret != 0)
					throw Error("DB put failed");
			}
		}
	public:
		Data() {
			count = 0;
			written = 0;
		}
		void Init(DB *ndb, DBT& key, DBT& data) {
			data.init(key, data);
			cassert(db == NULL);
			db = ndb;
			cassert(db != NULL);
		}
		~Data() {
			cassert(written);
		}
		void addref() {
			count++;
		}
		int delref() {
			count--;
			if (count == 0)
				Write();
			return (count == 0);
		}
	};
	DB db;
	BinTree<Data<T> > cache;
	virtual void release(DBT& key) {
		Mtx_Guard mutex(mtx);
		mutex.lock();
		cassert(cache.exists(key));
		if (cache[key].delref())
			cache.del(key);
	}
	virtual T& val(DBT& key) {
		Mtx_Guard mutex(mtx);
		mutex.lock();
		T *data;
		if (cache.exists(key)) {
			data = &cache[key];
		} else {
			data = &cache[key];
			data->Init(&db, key, &file);
		}
		data->addref();
		return *data;
	}
	virtual void del(DBT& key) {
		Mtx_Guard mutex(mtx);
		mutex.lock();
		cassert (!cache.exists(key));
		DBT dbdata;
		cassert(db != NULL);
		db->del(db, &key, 0);
	}
	virtual int exists(DBT& key) {
		Mtx_Guard mutex(mtx);
		mutex.lock();
		if (cache.exists(key))
			return 1;
		DBT dbdata;
		cassert(db != NULL);
		int res = db->get(db, &key, &dbdata, 0);
		return (res);
	}
public:
	BTreefile(const String& filename) {
		BTREEINFO bt;
		bzero(&bt, sizeof(bt));
		db = dbopen(filename.c_str(), O_RDWR, S_IRWXU, DB_BTREE, &bt);
		if (db == NULL)
			throw Error(String("open file ") << filename <<
			    " failed");
	}
	~BTreefile() {
		if (db != NULL)
			db->close(db);
	}
};

#endif /* !_KEYTABLE */
