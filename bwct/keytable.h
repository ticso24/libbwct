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

#include <bwct/tool.h>
#include <bwct/thread.h>
#include <mird.h>

class Keydata : public Base {
public:
	void read(void **data, size_t *size);
	void init(void *data, size_t size);
};

class Database : public Base {
protected:
	struct mird *db;
	Mutex mtx;
public:
	Database(const Database& tmp);
	Database(const String& filename, int flags = 0, uint mode = 600);
	~Database();
	void create(int num);
	void del(int num);
	void free(void* data);
	void get(int table, const String& key, void** data, size_t* size);
	void set(int table, const String& key, void* data, size_t size);
	void sync();
};

template <class T>
class Keytable : public Base {
protected:
	Database db;
	int tableno;
	void get(const String& table, const String& key,
	    void** data, size_t* size) {
		db.get(tableno, table, key, data, size);
	}
	void set(const String& table, const String& key,
	    void* data, size_t size) {
		db.get(tableno, table, key, data, size);
	}
public:
	Keytable(Database& ndb, const String& name) {
		tableno = 0;
		db = ndb;
		try {
			db.create(1);
		} catch (...) {
		}
		try {
			void *data;
			size_t size;
			db.get(1, name, &data, &size);
			if (size != sizeof(int)
				throw Error("");
			bcopy(&tableno, data, sizeof(int));
			db.free(data);
		} catch (...) {
			do {
				try {
					tableno = (int)genid();
					db.create(tableno);
				} catch (...) {
					tableno = 0;
				}
			} while (tableno == 0);
			db.set(1, name, &tableno, sizeof(int));
		}
	}
	void get(const String& table, const String& key, T& dobj) {
		void *data;
		size_t size;
		db.get(tableno, table, key, &data, &size);
		try {
			dobj.init(data, size);
		} catch (...) {
			db.free(data);
			throw;
		}
		db.free(data);
	}
	void set(const String& table, const String& key, T& dobj) {
		void *data;
		size_t size;
		dobj.read(&data, &size);
		db.get(tableno, table, key, data, size);
	}
};

#endif /* !_KEYTABLE */
