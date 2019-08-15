/*
 * Copyright (c) 2001-2014 Bernd Walter Computer Technology
 * Copyright (c) 2008-2014 FIZON GmbH
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#ifndef _JSON
#define _JSON

#include "tool.h"
#include "aarray.h"
#include <memory>

class JSON;

class JSON : public Base {
private:
	enum class Type {
		undefined = 0,	// object is invalid
		null,		// no storage
		string,
		object,		// aaray of subnodes
		number,		// use string container and let caller decide wether frac, int, ...
		array,		// array of subnodes
		boolean
	};

	Type type;
	bool bool_state;
	std::shared_ptr<String> str;
	std::shared_ptr<Array<JSON>> array;
	std::shared_ptr<AArray<JSON>> aarray;

	void clear();
	void iparse(const String& json, int64_t& parserpos);
	void parsewhitespace(const String& json, int64_t& parserpos);
	String parsestring(const String& json, int64_t& parserpos);

	static String ESC(const String& val);

public:
	JSON();
	JSON(const JSON& rh);
	JSON(JSON&& rh);
	~JSON();

	void parse(const String& json);
	String generate(bool newline = false) const;

	const JSON& operator=(const JSON& rh);
	const JSON& operator=(JSON&& rh);
	const JSON& operator=(bool rh);
	const JSON& operator=(const String& rh);
	const JSON& operator=(int64_t rh);
	const JSON& operator=(const Array<JSON>& rh);
	const JSON& operator=(const AArray<JSON>& rh);
	const JSON& set_null();

	template <class T>
	const JSON& set_number(const T &rh) {
		clear();
		type = Type::number;
		str.reset(new String(rh));
		return *this;
	}

	JSON& operator[](const char* rh);
	const JSON& operator[](const char* rh) const;
	JSON& operator[](const String& rh);
	const JSON& operator[](const String& rh) const;
	JSON& operator[](int64_t rh);
	const JSON& operator[](int64_t rh) const;
	operator bool() const;
	operator String() const;
	bool is_null() const;
	const String& get_numstr() const;
	const Array<JSON>& get_array() const;
	Array<JSON>& get_array();
	const AArray<JSON>& get_object() const;
	AArray<JSON>& get_object();
	bool exists(const String& rh) const;
};

#endif /* !_JSON */
