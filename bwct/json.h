/*
 * Copyright (c) 2001-2014 Bernd Walter Computer Technology
 * Copyright (c) 2008-2014 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/json.h $
 * $Date: 2021-07-19 14:49:27 +0200 (Mon, 19 Jul 2021) $
 * $Author: ticso $
 * $Rev: 44535 $
 */

#ifndef _JSON
#define _JSON

#include "tool.h"
#include "aarray.h"
#include "array.h"
#include <memory>

class JSON;

class JSON : public Base {
public:
	enum class Type {
		null = 0,	// no storage
		string,
		object,		// aaray of subnodes
		number,		// use string container and let caller decide wether frac, int, ...
		array,		// array of subnodes
		boolean
	};

private:
	Type type;
	bool bool_state;
	String *str;
	Array<JSON> *array;
	AArray<JSON> *aarray;

	void clear();
	void iparse(const String& json, int64_t& parserpos);
	void parsewhitespace(const String& json, int64_t& parserpos);
	String parsestring(const String& json, int64_t& parserpos);

	static String ESC(const String& val);
	void int_generate(Array<String>& data, bool formated, int level) const;
	Array<JSON> int_query(String q) const;

public:
	JSON();
	JSON(const JSON& rh);
	JSON(JSON&& rh);
	~JSON();

	void parse(const String& json);
	String generate(bool newline = false) const;
	void create_table(AArray<JSON>& data, String path) const;

	const JSON& operator=(const JSON& rh);
	const JSON& operator=(JSON&& rh);
	const JSON& operator=(bool rh);
	const JSON& operator=(const String& rh);
	const JSON& operator=(String&& rh);
	const JSON& operator=(const char* rh);
	const JSON& operator=(int64_t rh);
	const JSON& operator=(const Array<JSON>& rh);
	const JSON& operator=(Array<JSON>&& rh);
	const JSON& operator=(const AArray<JSON>& rh);
	const JSON& operator=(AArray<JSON>&& rh);
	const JSON& set_null();

	template <class T>
	const JSON& set_number(const T &rh) {
		clear();
		type = Type::number;
		delete str;
		str = new String;
		*str = rh;
		return *this;
	}

	bool operator==(const char* rh) const;
	bool operator==(const String& rh) const;
	bool operator!=(const char* rh) const;
	bool operator!=(const String& rh) const;
	JSON& operator[](const char* rh);
	const JSON& operator[](const char* rh) const;
	JSON& operator[](const String& rh);
	const JSON& operator[](const String& rh) const;
	JSON& operator[](int64_t rh);
	const JSON& operator[](int64_t rh) const;
	operator bool() const;
	Array<JSON> query(const String& q) const;
	bool is_null() const;
	bool is_string() const;
	bool is_object() const;
	bool is_number() const;
	bool is_array() const;
	bool is_boolean() const;
	const String& get_numstr() const;
	const String& get_str() const;
	const char* c_str() const;
	const Array<JSON>& get_array() const;
	int64_t get_max() const;
	Array<JSON>& get_array();
	const AArray<JSON>& get_object() const;
	AArray<JSON>& get_object();
	bool exists(const String& rh) const;
	bool is_type(const String& rh) const;
	Type get_type() const;
};

#endif /* !_JSON */
