/*
 * Copyright (c) 2002 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#include <bwct/base.h>
#include <bwct/keytable.h>

Database::Database(const String& filename, int flags, uint mode) {
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

Database::~Database() {
	Mutex::Guard mutex(mtx);
	if (db != NULL) {
		mird_close(db);
		db = NULL;
	}
}

void
Database::create(int num) {
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
Database::del(int num) {
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
Database::free(void* data) {
	if (data != NULL)
		mird_free((unsigned char*)data);
}

void
Database::get(int table, const String& key, void** data, size_t* size) {
	MIRD_RES res;
	Mutex::Guard mutex(mtx);
	res = mird_s_key_lookup(db, table,
	    (unsigned char*)key.c_str(), key.length(),
	    (unsigned char**)data, (mird_size_t*)size);
	if (res != 0) 
		throw Error("Failed to lookup key");
}

void
Database::set(int table, const String& key, void* data, size_t size) {
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
Database::sync() {
	MIRD_RES res;
	Mutex::Guard mutex(mtx);
	res = mird_sync(db);
	if (res != 0)
		throw Error("Sync failed");
}
