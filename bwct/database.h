/*
 * Copyright (c) 2002,03 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#ifndef _DATABASE
#define _DATABASE

#include <bwct/tool.h>
#include <bwct/thread.h>

struct mird;

class DB : public Base {
protected:
	struct mird *db;
	Mutex mtx;
	void s_create(uint32_t num);
	void n_create(uint32_t num);
	DB(const DB& tmp);
public:
	DB(const String& filename, int flags = 0, uint mode = 600);
	~DB();
	void del(uint32_t num);
	void del(uint32_t table, const uint32_t key);
	void del(uint32_t table, const String& key);
	uint32_t s_select(const String& name);
	uint32_t n_select(const String& name);
	void free(void* data);
	void get(uint32_t table, const uint32_t key, void** data, size_t* size);
	void get(uint32_t table, const String& key, void** data, size_t* size);
	void set(uint32_t table, const uint32_t key, void* data, size_t size);
	void set(uint32_t table, const String& key, void* data, size_t size);
	void sync();

	class Obj : public Base {
	public:
		virtual void init(void* data, size_t size) {
		}
		virtual void read(void** data, size_t* size) {
		}
	};

	class NumTable : public Base {
	protected:
		const String tablename;
		DB& db;
		uint32_t tableno;
		void get(const uint32_t key, void** data, size_t* size) {
			db.get(tableno, key, data, size);
		}
		void set(const uint32_t key, void* data, size_t size) {
			db.set(tableno, key, data, size);
		}
	public:
		NumTable(DB& ndb, const String& name)
		    : tablename(name), db(ndb) {
			tableno = db.n_select(name);
		}
		bool exists(const uint32_t key);
		bool get(const uint32_t key, Obj& dobj);
		void set(const uint32_t key, Obj& dobj);
		void del(const uint32_t key) {
			db.del(tableno, key);
		}
	};
	class StringTable : public Base {
	protected:
		const String tablename;
		DB& db;
		uint32_t tableno;
		void get(const String& key, void** data, size_t* size) {
			db.get(tableno, key, data, size);
		}
		void set(const String& key, void* data, size_t size) {
			db.set(tableno, key, data, size);
		}
	public:
		StringTable(DB& ndb, const String& name)
		    : tablename(name), db(ndb) {
			tableno = db.s_select(name);
		}
		bool exists(const String& key);
		bool get(const String& key, Obj& dobj);
		void set(const String& key, Obj& dobj);
		void del(const String& key) {
			db.del(tableno, key);
		}
	};
};

#endif /* !_DATABASE */
