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
		bool dirty;
	public:
		Obj();
		virtual ~Obj();
		bool isdirty();
		virtual size_t init(void* data, size_t size);
		virtual size_t calcsize();
		virtual size_t read(void* data, size_t size);
	};
	class NumTable : public Base {
	protected:
		const String tablename;
		DB& db;
		uint32_t tableno;
		void get(const uint32_t key, void** data, size_t* size);
		void set(const uint32_t key, void* data, size_t size);
	public:
		NumTable(DB& ndb, const String& name);
		bool exists(const uint32_t key);
		bool get(const uint32_t key, Obj& dobj);
		void set(const uint32_t key, Obj& dobj);
		void del(const uint32_t key);
	};
	class StringTable : public Base {
	protected:
		const String tablename;
		DB& db;
		uint32_t tableno;
		void get(const String& key, void** data, size_t* size);
		void set(const String& key, void* data, size_t size);
	public:
		StringTable(DB& ndb, const String& name);
		bool exists(const String& key);
		bool get(const String& key, Obj& dobj);
		void set(const String& key, Obj& dobj);
		void del(const String& key);
	};
	class DBString : public Obj {
	protected:
		String str;
	public:
		DBString();
		virtual size_t calcsize();
		virtual size_t init(void* data, size_t size);
		virtual size_t read(void* data, size_t size);
		operator const String&() const;
		const String& operator=(const String& rhs);
	};
	class DBint64 : public Obj {
	protected:
		uint64_t num;
	public:
		DBint64();
		virtual size_t calcsize();
		virtual size_t init(void* data, size_t size);
		virtual size_t read(void* data, size_t size);
		operator uint64_t() const;
		uint64_t operator=(uint64_t rhs);
	};

	class String_idx : public Obj {
	protected:
		DB::StringTable table;
		DBString key;
		SArray<uint32_t> refno;
	public:
		String_idx(DB& ndb, const String& name);
		~String_idx();
		void del();
	};

	class Num64_idx : public Obj {
	protected:
		DB::NumTable table;
		DBint64 key;
		SArray<uint32_t> refno;
	public:
		Num64_idx(DB& ndb, const String& name);
		~Num64_idx();
		void del();
	};
};

#endif /* !_DATABASE */
