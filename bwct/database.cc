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
	void *data;
	size_t size;
	dobj.read(&data, &size);
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
	void *data;
	size_t size;
	dobj.read(&data, &size);
	db.set(tableno, key, data, size);
}

