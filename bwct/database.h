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
	DB();
	~DB();
	void open(const String& filename, int flags = 0, uint mode = 600);
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
	protected:
		Mutex mtx;
		SArray<Obj*> sub;
	public:
		bool dirty;
		Obj(DB& ndb, const String& name) {
			dirty = false;
		}
		virtual size_t init(void* data, size_t size) {
			size_t tmp;
			for(int i = 0; i < sub.max; i++) {
				tmp = sub[i]->init(data, size);
				(char*)data += tmp;
				size -= tmp;
			}
			return tmp;
		}
		virtual size_t calcsize() {
			size_t ret = 0;
			for(int i = 0; i < sub.max; i++) {
				ret += sub[i]->calcsize();
			}
			return ret;
		}
		virtual size_t read(void* data, size_t size) {
			size_t tmp;
			for(int i = 0; i < sub.max; i++) {
				tmp = sub[i]->read(data, size);
				(char*)data += tmp;
				size -= tmp;
			}
			return tmp;
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
	class DBString : public Obj {
	protected:
		String str;
	public:
		DBString(DB& ndb, const String& name) :
		    Obj::Obj(ndb, name) {
		}
		virtual size_t calcsize() {
			size_t size = str.length() + 1;
			size += Obj::calcsize();
			return size;
		}
		virtual size_t init(void* data, size_t size) {
			cassert(size >= (size_t)str.length() + 1);
			bcopy(data, (void*)str.c_str(), str.length() + 1);
			(char*)data += str.length() + 1;
			size -= str.length() + 1;
			return Obj::init(data, size);
		}
		virtual size_t read(void* data, size_t size) {
			str = (char*)data;
			cassert(size >= (size_t)str.length() + 1);
			(char*)data += str.length() + 1;
			size -= str.length() + 1;
			return Obj::read(data, size);
		}
		operator const String&() const {
			return str;
		}
		operator String&() {
			dirty = true;
			return str;
		}
	};
	class DBint64 : public Obj {
	protected:
		uint64_t num;
	public:
		DBint64(DB& ndb, const String& name) :
		    Obj::Obj(ndb, name) {
		}
		virtual size_t calcsize() {
			return sizeof(num);
		}
		virtual size_t init(void* data, size_t size) {
			cassert(size >= sizeof(num));
			uint32_t data1, data2;
			data2 = htonl(num & 0xffffffff);
			data1 = htonl(num >> 32);
			bcopy(data, &data1, sizeof(data1));
			bcopy((char*)data + sizeof(data1), &data2, sizeof(data2));
			return sizeof(num);
		}
		virtual size_t read(void* data, size_t size) {
			cassert(size >= sizeof(num));
			uint32_t data1, data2;
			bcopy(&data1, data, sizeof(data1));
			bcopy(&data2, (char*)data + sizeof(data1), sizeof(data2));
			num = ((uint64_t)ntohl(data1)) << 32 | ntohl(data2);
			return sizeof(num);
		}
		operator const uint64_t& () const {
			return num;
		}
		operator uint64_t& () {
			dirty = true;
			return num;
		}
	};
	class String_idx : public Obj {
	protected:
		DB::StringTable table;
		DBString key;
		SArray<uint32_t> refno;
	public:
		String_idx(DB& ndb, const String& name, const String& namex) :
		    Obj::Obj(ndb, name),
		    table(ndb, name + namex),
		    key(ndb, name) {
		    sub[0] = &key;
		}
		~String_idx() {
		}
		void del() {
			table.del(key);
		}
	};

	class Num64_idx : public Obj {
	protected:
		DB::NumTable table;
		DBint64 key;
		SArray<uint32_t> refno;
	public:
		Num64_idx(DB& ndb, const String& name, const String& namex) :
		    Obj::Obj(ndb, name),
		    table(ndb, name + namex),
		    key(ndb, name) {
		    sub[0] = &key;
		}
		void del() {
			table.del((int)key);
		}
	};
};


#endif /* !_DATABASE */
