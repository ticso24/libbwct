/*
 * Copyright (c) 2002,03 Bernd Walter Computer Technology
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

struct mird;

class DB : public Base {
protected:
	struct mird *db;
	Mutex mtx;
public:
	DB(const DB& tmp);
	DB(const String& filename, int flags = 0, uint mode = 600);
	~DB();
	void create(int num);
	void del(int num);
	void del(int table, const String& key);
	void free(void* data);
	void get(int table, const String& key, void** data, size_t* size);
	void set(int table, const String& key, void* data, size_t size);
	void sync();

	template <class T>
	class Table : public Base {
	protected:
		DB db;
		int tableno;
		const String tablename;
		void get(const String& key, void** data, size_t* size) {
			db.get(tableno, key, data, size);
		}
		void set(const String& key, void* data, size_t size) {
			db.get(tableno, key, data, size);
		}
	public:
		Table(DB& ndb, const String& name) : tablename(name) {
			tableno = 0;
			db = ndb;
			try {
				db.create(1);
			} catch (...) {
			}
			try {
				void *data;
				size_t size;
				db.get(1, tablename, &data, &size);
				if (size != sizeof(int))
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
				db.set(1, tablename, &tableno, sizeof(int));
			}
		}
		bool exists(const String& key) {
			void *data;
			size_t size;
			db.get(tableno, key, &data, &size);
			if (data == NULL)
				return false;
			db.free(data);
			return true;
		}
		bool get(const String& key, T& dobj) {
			void *data;
			size_t size;
			db.get(tableno, key, &data, &size);
			if (data == NULL)
				return false;
			try {
				dobj.init(data, size);
			} catch (...) {
				db.free(data);
				throw;
			}
			db.free(data);
			return true;
		}
		void set(const String& key, T& dobj) {
			void *data;
			size_t size;
			dobj.read(&data, &size);
			db.get(tableno, key, data, size);
		}
		void del(const String& key) {
			db.del(tableno, key);
		}
	};
};


#endif /* !_KEYTABLE */
