/*
 * Copyright (c) 2002,03 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#include <mird.h>
#include <bwct/base.h>
#include <bwct/database.h>

void
DB::open(const String& filename, int flags, uint mode) {
	MIRD_RES res;
	Mutex::Guard mutex(mtx);
	res = mird_initialize((char*)filename.c_str(), &db);
	if (res != 0)
		throw Error("init db failed");
	db->flags |= flags;
	db->cache_size = 1024;
	db->file_mode = mode;
	res = mird_open(db);
	if (res != 0)
		throw Error("open db failed");
	res = mird_sync(db);
	if (res != 0)
		throw Error("sync db failed");
}

DB::DB(const String& filename, int flags, uint mode) {
	open(filename, flags, mode);
}

DB::DB() {
	db = NULL;
}

DB::~DB() {
	Mutex::Guard mutex(mtx);
	if (db != NULL) {
		mird_close(db);
		db = NULL;
	}
}

void
DB::n_create(uint32_t num) {
	cassert(db != NULL);
	MIRD_RES res;
	Mutex::Guard mutex(mtx);
	struct mird_transaction *mtr;
	res = mird_transaction_new(db, &mtr);
	if (res != 0) 
		throw Error("Failed to create transaction");
	try {
		res = mird_key_new_table(mtr, num);
		if (res != 0) 
			throw Error("Failed to create table");
	} catch (...) {
		res = mird_transaction_cancel(mtr);
		if (res != 0) 
			throw Error("Failed to cancel transaction");
		throw;
	}
	res = mird_transaction_close(mtr);
	if (res != 0) 
		throw Error("Failed to close transaction");
}

void
DB::s_create(uint32_t num) {
	cassert(db != NULL);
	MIRD_RES res;
	Mutex::Guard mutex(mtx);
	struct mird_transaction *mtr;
	res = mird_transaction_new(db, &mtr);
	if (res != 0) 
		throw Error("Failed to create transaction");
	try {
		res = mird_s_key_new_table(mtr, num);
		if (res != 0) 
			throw Error("Failed to create table");
	} catch (...) {
		res = mird_transaction_cancel(mtr);
		if (res != 0) 
			throw Error("Failed to cancel transaction");
		throw;
	}
	res = mird_transaction_close(mtr);
	if (res != 0) 
		throw Error("Failed to close transaction");
}

void
DB::del(uint32_t num) {
	cassert(db != NULL);
	MIRD_RES res;
	Mutex::Guard mutex(mtx);
	struct mird_transaction *mtr;
	res = mird_transaction_new(db, &mtr);
	if (res != 0) 
		throw Error("Failed to create transaction");
	try {
		res = mird_delete_table(mtr, num);
		if (res != 0) 
			throw Error("Failed to delete table");
	} catch (...) {
		res = mird_transaction_cancel(mtr);
		if (res != 0) 
			throw Error("Failed to cancel transaction");
		throw;
	}
	res = mird_transaction_close(mtr);
	if (res != 0) 
		throw Error("Failed to close transaction");
}

void
DB::free(void* data) {
	if (data != NULL)
		mird_free((unsigned char*)data);
}

void
DB::get(uint32_t table, const uint32_t key, void** data, size_t* size) {
	cassert(db != NULL);
	MIRD_RES res;
	Mutex::Guard mutex(mtx);
	res = mird_key_lookup(db, table, key,
	    (unsigned char**)data, (mird_size_t*)size);
	if (res != 0) 
		throw Error("Failed to lookup key");
}

void
DB::get(uint32_t table, const String& key, void** data, size_t* size) {
	cassert(db != NULL);
	MIRD_RES res;
	Mutex::Guard mutex(mtx);
	res = mird_s_key_lookup(db, table,
	    (unsigned char*)key.c_str(), key.length(),
	    (unsigned char**)data, (mird_size_t*)size);
	if (res != 0) 
		throw Error("Failed to lookup key");
}

void
DB::del(uint32_t table, const uint32_t key) {
	set (table, key, NULL, 0);
}

void
DB::del(uint32_t table, const String& key) {
	set (table, key, NULL, 0);
}

void
DB::set(uint32_t table, const uint32_t key, void* data, size_t size) {
	cassert(db != NULL);
	MIRD_RES res;
	Mutex::Guard mutex(mtx);
	struct mird_transaction *mtr;
	res = mird_transaction_new(db, &mtr);
	if (res != 0) 
		throw Error("Failed to create transaction");
	try {
		res = mird_key_store(mtr, table, key,
		    (unsigned char*)data, size);
		if (res != 0) 
			throw Error("Failed to set key");
	} catch (...) {
		res = mird_transaction_cancel(mtr);
		if (res != 0) 
			throw Error("Failed to cancel transaction");
		throw;
	}
	res = mird_transaction_close(mtr);
	if (res != 0) 
		throw Error("Failed to close transaction");
}

void
DB::set(uint32_t table, const String& key, void* data, size_t size) {
	cassert(db != NULL);
	MIRD_RES res;
	Mutex::Guard mutex(mtx);
	struct mird_transaction *mtr;
	res = mird_transaction_new(db, &mtr);
	if (res != 0) 
		throw Error("Failed to create transaction");
	try {
		res = mird_s_key_store(mtr, table,
		    (unsigned char*)key.c_str(), key.length(),
		    (unsigned char*)data, size);
		if (res != 0) 
			throw Error("Failed to set key");
	} catch (...) {
		res = mird_transaction_cancel(mtr);
		if (res != 0) 
			throw Error("Failed to cancel transaction");
		throw;
	}
	res = mird_transaction_close(mtr);
	if (res != 0) 
		throw Error("Failed to close transaction");
}

void
DB::sync() {
	cassert(db != NULL);
	MIRD_RES res;
	Mutex::Guard mutex(mtx);
	res = mird_sync(db);
	if (res != 0)
		throw Error("Sync failed");
}

uint32_t
DB::n_select(const String& name) {
	cassert(db != NULL);
	uint32_t tableno = 0;
	try {
		s_create(1);
	} catch (...) {
	}
	try {
		void *data;
		size_t size;
		get(1, name, &data, &size);
		if (size != sizeof(uint32_t))
			throw Error("");
		bcopy(&tableno, data, sizeof(uint32_t));
		(this)->free(data);
	} catch (...) {
		do {
			try {
				tableno = (uint32_t)genid();
				n_create(tableno);
			} catch (...) {
				tableno = 0;
			}
		} while (tableno == 0);
		set(1, name, &tableno, sizeof(uint32_t));
	}
	return tableno;
}

uint32_t
DB::s_select(const String& name) {
	cassert(db != NULL);
	uint32_t tableno = 0;
	try {
		s_create(1);
	} catch (...) {
	}
	try {
		void *data;
		size_t size;
		get(1, name, &data, &size);
		if (size != sizeof(uint32_t))
			throw Error("");
		bcopy(&tableno, data, sizeof(uint32_t));
		(this)->free(data);
	} catch (...) {
		do {
			try {
				tableno = (uint32_t)genid();
				s_create(tableno);
			} catch (...) {
				tableno = 0;
			}
		} while (tableno == 0);
		set(1, name, &tableno, sizeof(uint32_t));
	}
	return tableno;
}

bool
DB::NumTable::exists(const uint32_t key) {
	void *data;
	size_t size;
	db.get(tableno, key, &data, &size);
	if (data == NULL)
		return false;
	db.free(data);
	return true;
}

bool
DB::NumTable::get(const uint32_t key, Obj& dobj) {
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
 
void
DB::NumTable::set(const uint32_t key, Obj& dobj) {
	size_t size = dobj.calcsize();
	char data[size];
	dobj.read((void*)data, size);
	db.set(tableno, key, data, size);
}

bool
DB::StringTable::exists(const String& key) {
	void *data;
	size_t size;
	db.get(tableno, key, &data, &size);
	if (data == NULL)
		return false;
	db.free(data);
	return true;
}

bool
DB::StringTable::get(const String& key, Obj& dobj) {
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

void
DB::StringTable::set(const String& key, Obj& dobj) {
	size_t size = dobj.calcsize();
	char data[size];
	dobj.read((void*)data, size);
	db.set(tableno, key, data, size);
}

DB::Obj::Obj() {
	dirty = false;
}

DB::Obj::~Obj() {
}

bool
DB::Obj::isdirty() {
	return dirty;
}

size_t
DB::Obj::init(void* data, size_t size) {
	size_t tmp = 0;
	for(int i = 0; i < sub.max; i++) {
		tmp = sub[i]->init(data, size);
		(char*)data += tmp;
		size -= tmp;
	}
	return tmp;
}

size_t
DB::Obj::calcsize() {
	size_t ret = 0;
	for(int i = 0; i < sub.max; i++) {
		ret += sub[i]->calcsize();
	}
	return ret;
}

size_t
DB::Obj::read(void* data, size_t size) {
	size_t tmp = 0;
	for(int i = 0; i < sub.max; i++) {
		tmp = sub[i]->read(data, size);
		(char*)data += tmp;
		size -= tmp;
	}
	return tmp;
}

void
DB::NumTable::get(const uint32_t key, void** data, size_t* size) {
	db.get(tableno, key, data, size);
}

void
DB::NumTable::set(const uint32_t key, void* data, size_t size) {
	db.set(tableno, key, data, size);
}

DB::NumTable::NumTable(DB& ndb, const String& name)
    : tablename(name), db(ndb) {
	tableno = db.n_select(name);
}

void
DB::NumTable::del(const uint32_t key) {
	db.del(tableno, key);
}

void
DB::StringTable::get(const String& key, void** data, size_t* size) {
	db.get(tableno, key, data, size);
}

void
DB::StringTable::set(const String& key, void* data, size_t size) {
	db.set(tableno, key, data, size);
}

DB::StringTable::StringTable(DB& ndb, const String& name)
    : tablename(name), db(ndb) {
	tableno = db.s_select(name);
}

void
DB::StringTable::del(const String& key) {
	db.del(tableno, key);
}

DB::DBString::DBString() {
}

size_t
DB::DBString::calcsize() {
	size_t size = str.length() + 1;
	size += Obj::calcsize();
	return size;
}

size_t
DB::DBString::init(void* data, size_t size) {
	cassert(size >= (size_t)str.length() + 1);
	bcopy(data, (void*)str.c_str(), str.length() + 1);
	(char*)data += str.length() + 1;
	size -= str.length() + 1;
	return Obj::init(data, size);
}

size_t
DB::DBString::read(void* data, size_t size) {
	str = (char*)data;
	cassert(size >= (size_t)str.length() + 1);
	(char*)data += str.length() + 1;
	size -= str.length() + 1;
	return Obj::read(data, size);
}

DB::DBString::operator const String&() const {
	return str;
}

const String&
DB::DBString::operator=(const String& rhs) {
	dirty = true;
	str = rhs;
	return str;
}

DB::DBint64::DBint64() {
}

size_t
DB::DBint64::calcsize() {
	return sizeof(num);
}

size_t
DB::DBint64::init(void* data, size_t size) {
	cassert(size >= sizeof(num));
	uint32_t data1, data2;
	data2 = htonl(num & 0xffffffff);
	data1 = htonl(num >> 32);
	bcopy(data, &data1, sizeof(data1));
	bcopy((char*)data + sizeof(data1), &data2, sizeof(data2));
	return sizeof(num);
}

size_t
DB::DBint64::read(void* data, size_t size) {
	cassert(size >= sizeof(num));
	uint32_t data1, data2;
	bcopy(&data1, data, sizeof(data1));
	bcopy(&data2, (char*)data + sizeof(data1), sizeof(data2));
	num = ((uint64_t)ntohl(data1)) << 32 | ntohl(data2);
	return sizeof(num);
}

DB::DBint64::operator uint64_t() const {
	return num;
}

uint64_t
DB::DBint64::operator=(uint64_t rhs) {
	dirty = true;
	num = rhs;
	return num;
}

DB::String_idx::String_idx(DB& ndb, const String& name) :
    table(ndb, name) {
    sub[0] = &key;
}

DB::String_idx::~String_idx() {
// TODO
}

void
DB::String_idx::del() {
	table.del(key);
}

DB::Num64_idx::Num64_idx(DB& ndb, const String& name) :
    table(ndb, name) {
    sub[0] = &key;
}

DB::Num64_idx::~Num64_idx() {
// TODO
}

void 
DB::Num64_idx::del() {
	const uint64_t& tmp = key;
	table.del(tmp);
}

