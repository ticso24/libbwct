/*
 * Copyright (c) 2001,02,03,08,11,12 Bernd Walter Computer Technology
 * Copyright (c) 2008,11,12 FIZON GmbH
 * All rights reserved.
 *
 * crc_hash by Bob Jenkins, (c) 2006, Public Domain
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#include "bwct.h"

JSON::JSON()
{
	type = Type::undefined;
}

JSON::JSON(const JSON& rh)
{
	type = rh.type;
	switch(type) {
	case Type::string:
	case Type::number:
		str.reset(new String(*rh.str.get()));
		break;
	case Type::boolean:
		bool_state = rh.bool_state;
		break;
	case Type::array:
		array.reset(new Array<JSON>(*rh.array.get()));
		break;
	case Type::object:
		aarray.reset(new AArray<JSON>(*rh.aarray.get()));
		break;
	case Type::null:
	case Type::undefined:
		break;
	}
}

JSON::JSON(JSON&& rh)
{
	type = rh.type;
	switch(type) {
	case Type::string:
	case Type::number:
		std::swap(str, rh.str);
		break;
	case Type::boolean:
		bool_state = rh.bool_state;
		break;
	case Type::array:
		std::swap(array, rh.array);
		break;
	case Type::object:
		std::swap(aarray, rh.aarray);
		break;
	case Type::null:
	case Type::undefined:
		break;
	}
}

JSON::~JSON()
{
}

void
JSON::parse(const String& json)
{
	String njson(json);
	int64_t parserpos = 0;
	iparse(njson, parserpos);
	cassert(parserpos == (int64_t)json.length());
}

void
JSON::parsewhitespace(const String& json, int64_t& parserpos)
{
	for(;;) {
		char c = json[parserpos];
		switch (c) {
		case ' ':
		case '\n':
		case '\r':
		case '\t':
			parserpos++;
			break;
		default:
			return;
		}
	}
}

String
JSON::parsestring(const String& json, int64_t& parserpos)
{
	String ret;
	uint8_t c;
	char tmp[2] = {'\0', '\0'};

	while ((c = json[parserpos]) != '"') {
		switch (c) {
		case '\\':
			parserpos += 1;
			c = json[parserpos];
			switch(c) {
			case '\\':
			case '"':
			case '/':
				tmp[0] = c;
				ret += tmp;
				parserpos += 1;
				break;
			case 'u':
				{
					parserpos += 1;
					uint16_t hval = 0;
					for (int i = 0; i < 4; i++) {
						hval <<= 4;
						c = json[parserpos];
						switch(c) {
						case 'a':
						case 'b':
						case 'c':
						case 'd':
						case 'e':
						case 'f':
							hval |= 10 + c - 'a';
							break;
						case 'A':
						case 'B':
						case 'C':
						case 'D':
						case 'E':
						case 'F':
							hval |= 10 + c - 'A';
							break;
						case '0':
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':
						case '8':
						case '9':
							hval |= c;
							break;
						default:
							TError("\\u encoding error");
						}
						parserpos += 1;
					}
					if (hval < 0x80) {
						tmp[0] = hval & 0x7f;
						ret += tmp;
					} else if (hval < 0x0800) {
						tmp[0] = 0xc0 + ((hval >> 6) & 0x1f);
						ret += tmp;
						tmp[0] = 0x80 + (((hval >> 0) & 0x3f));
						ret += tmp;
					} else {
						// we only handle up to 16bit, so this is the last one to handle
						tmp[0] = 0xe0 + ((hval >> 12) & 0x1f);
						ret += tmp;
						tmp[0] = 0x80 + (((hval >> 6) & 0x3f));
						ret += tmp;
						tmp[0] = 0x80 + (((hval >> 0) & 0x3f));
						ret += tmp;
					}
				}
				break;
			case 'b':
				tmp[0] = 0x08;
				ret += tmp;
				parserpos += 1;
				break;
			case 'f':
				tmp[0] = 0x0c;
				ret += tmp;
				parserpos += 1;
				break;
			case 'n':
				tmp[0] = '\n';
				ret += tmp;
				parserpos += 1;
				break;
			case 'r':
				tmp[0] = '\r';
				ret += tmp;
				parserpos += 1;
				break;
			case 't':
				tmp[0] = '\t';
				ret += tmp;
				parserpos += 1;
				break;
			default:
				TError("\\ encoding error");
			}
			break;
		default:
			tmp[0] = c;
			ret += tmp;
			parserpos += 1;
			break;
		}
	}
	parserpos += 1;
	return ret;
}

void
JSON::iparse(const String& json, int64_t& parserpos)
{
	clear();
	parsewhitespace(json, parserpos);
	if (json.strncmp(parserpos, "\"")) {
		type = Type::string;
		parserpos += 1;
		str.reset(new String(parsestring(json, parserpos)));
	} else if (json.strncmp(parserpos, "[")) {
		{
			bool cont = true;
			type = Type::array;
			array.reset(new Array<JSON>());
			parserpos += 1;
			parsewhitespace(json, parserpos);
			while (cont) {
				int64_t newpos = array->max + 1;
				(*array.get())[newpos].iparse(json, parserpos);
				if (json[parserpos] != ',') {
					if (json[parserpos] != ']') {
						TError("no closing ']'");
					}
					parserpos += 1;
					cont = false;
				} else {
					parserpos += 1;
					parsewhitespace(json, parserpos);
				}
			}
		}
	} else if (json.strncmp(parserpos, "{")) {
		{
			bool cont = true;
			type = Type::object;
			aarray.reset(new AArray<JSON>());
			parserpos += 1;
			parsewhitespace(json, parserpos);
			while (cont) {
				if (json[parserpos] != '"') {
					TError("no key string");
				}
				parserpos += 1;
				String key = parsestring(json, parserpos);
				parsewhitespace(json, parserpos);
				if (json[parserpos] != ':') {
					TError("missing \":\"");
				}
				parserpos += 1;
				parsewhitespace(json, parserpos);
				(*aarray.get())[key].iparse(json, parserpos);
				if (json[parserpos] != ',') {
					if (json[parserpos] != '}') {
						TError("no closing '}'");
					}
					parserpos += 1;
					cont = false;
				} else {
					parserpos += 1;
					parsewhitespace(json, parserpos);
				}
			}
		}
	} else if (json.strncmp(parserpos, "true")) {
		type = Type::boolean;
		bool_state = true;
		parserpos += 4;
	} else if (json.strncmp(parserpos, "false")) {
		type = Type::boolean;
		bool_state = false;
		parserpos += 5;
	} else if (json.strncmp(parserpos, "null")) {
		type = Type::null;
		parserpos += 4;
	} else {
		{
			// probe for numberic data
			type = Type::number;
			bool cont = true;
			String val;
			char tmp[2] = {'\0', '\0'};
			while (cont) {
				uint8_t c = json[parserpos];
				switch(c) {
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				case '.':
				case '+':
				case '-':
				case 'E':
				case 'e':
					tmp[0] = c;
					val += tmp;
					parserpos += 1;
					break;
				default:
					cont = false;
				}
			}
			str.reset(new String(val));
		}
	}
	parsewhitespace(json, parserpos);
}

void
JSON::clear()
{
	type = Type::undefined;
	str.reset();
	array.reset();
	aarray.reset();
}

String
JSON::ESC(const String& val)
{
	String ret;
	char buf[2];
	const char* plh;
	uint8_t tmp;

	buf[1] = '\0';
	plh = val.c_str();
	for (size_t i = 0; i < val.length(); i++) {
		tmp = plh[i];

		switch (tmp) {
		case '\\':
			ret += "\\\\";
			break;
		case '"':
			ret += "\\\"";
			break;
		case '/':
			ret += "\\/";
			break;
		case '\b':
			ret += "\\\b";
			break;
		case '\t':
			ret += "\\\t";
			break;
		case '\n':
			ret += "\\\n";
			break;
		case '\f':
			ret += "\\\f";
			break;
		case '\r':
			ret += "\\\r";
			break;
		default:
			if (tmp < 0x20) {
				uint8_t nibble;
				ret += "\\u00";
				nibble = tmp >> 4;
				if (nibble > 9) {
					buf[0] = 'a' + nibble - 10;
				} else {
					buf[0] = '0' + nibble;
				}
				ret += buf;
				nibble = tmp & 0xf;
				if (nibble > 9) {
					buf[0] = 'a' + nibble - 10;
				} else {
					buf[0] = '0' + nibble;
				}
				ret += buf;
			} else {
				buf[0] = tmp;
				ret += buf;
			}
		}
	}
	return ret;
}

String
JSON::generate(bool newline) const
{
	String ret;
	String nl;
	if (newline) {
		nl = "\n";
	}
	switch(type) {
	case Type::undefined:
		TError("undefined Object");
	case Type::null:
		ret = "null";
		break;
	case Type::string:
		ret = S + "\"" + ESC(*str.get()) + "\"";
		break;
	case Type::object:
		{
			Array<String> keys = aarray->getkeys(true);
			ret += S + "{" + nl;
			for (int i = 0; i <= keys.max; i++) {
				ret += S + "\"" + ESC(keys[i]) + "\"" + ":" + (*aarray.get())[keys[i]].generate(newline);
				if (i != keys.max) {
					ret += ",";
				}
				ret += nl;
			}
			ret += S + "}" + nl;
		}
		break;
	case Type::number:
		ret = *str.get();
		break;
	case Type::array:
		ret += S + "[" + nl;
		for (int i = 0; i <= array->max; i++) {
			ret += S + (*array.get())[i].generate(newline);
			if (i != array->max) {
				ret += ",";
			}
			ret += nl;
		}
		ret += S + "]" + nl;
		break;
	case Type::boolean:
		ret = (bool_state) ? "true" : "false";
		break;
	}
	return ret;
}

const JSON&
JSON::operator=(const JSON& rh)
{
	clear();
	type = rh.type;
	switch(type) {
	case Type::string:
	case Type::number:
		str.reset(new String(*rh.str.get()));
		break;
	case Type::boolean:
		bool_state = rh.bool_state;
		break;
	case Type::array:
		array.reset(new Array<JSON>(*rh.array.get()));
		break;
	case Type::object:
		aarray.reset(new AArray<JSON>(*rh.aarray.get()));
		break;
	case Type::null:
	case Type::undefined:
		break;
	}
	return *this;
}

const JSON&
JSON::operator=(JSON&& rh)
{
	clear();
	type = rh.type;
	switch(type) {
	case Type::string:
	case Type::number:
		std::swap(str, rh.str);
		break;
	case Type::boolean:
		bool_state = rh.bool_state;
		break;
	case Type::array:
		std::swap(array, rh.array);
		break;
	case Type::object:
		std::swap(aarray, rh.aarray);
		break;
	case Type::null:
	case Type::undefined:
		break;
	}
	return *this;
}

const JSON&
JSON::operator=(bool rh)
{
	clear();
	type = Type::boolean;
	bool_state = rh;
	return *this;
}

const JSON&
JSON::operator=(const String& rh)
{
	clear();
	type = Type::string;
	str.reset(new String(rh));
	return *this;
}

const JSON&
JSON::operator=(int64_t rh)
{
	clear();
	type = Type::number;
	str.reset(new String(rh));
	return *this;
}

const JSON&
JSON::operator=(const Array<JSON>& rh)
{
	clear();
	type = Type::array;
	array.reset(new Array<JSON>(rh));
	return *this;
}

const JSON&
JSON::operator=(const AArray<JSON>& rh)
{
	clear();
	type = Type::object;
	aarray.reset(new AArray<JSON>(rh));
	return *this;
}

const JSON&
JSON::set_null()
{
	clear();
	type = Type::null;
	return *this;
}

const JSON&
JSON::operator[](const char* rh) const
{
	cassert(type == Type::object);
	JSON* ret;
	ret = &(*aarray.get())[rh];
	return *ret;
}

JSON&
JSON::operator[](const char* rh)
{
	cassert(type == Type::object);
	JSON* ret;
	ret = &(*aarray.get())[rh];
	return *ret;
}

const JSON&
JSON::operator[](const String& rh) const
{
	cassert(type == Type::object);
	JSON* ret;
	ret = &(*aarray.get())[rh];
	return *ret;
}

JSON&
JSON::operator[](const String& rh)
{
	cassert(type == Type::object);
	JSON* ret;
	ret = &(*aarray.get())[rh];
	return *ret;
}

const JSON&
JSON::operator[](int64_t rh) const
{
	cassert(type == Type::array);
	JSON* ret;
	ret = &(*array.get())[rh];
	return *ret;
}

JSON&
JSON::operator[](int64_t rh)
{
	cassert(type == Type::array);
	JSON* ret;
	ret = &(*array.get())[rh];
	return *ret;
}

JSON::operator bool() const
{
	cassert(type == Type::boolean);
	return bool_state;
}

JSON::operator String() const
{
	cassert(type == Type::string);
	return *str.get();
}

const String&
JSON::get_numstr() const
{
	cassert(type == Type::number);
	return *str.get();
}

Array<JSON>&
JSON::get_array()
{
	cassert(type == Type::array);
	return *array.get();
}

AArray<JSON>&
JSON::get_object()
{
	cassert(type == Type::object);
	return *aarray.get();
}

bool
JSON::exists(const String& rh) const
{
	cassert(type == Type::object);
	return (*aarray.get()).exists(rh);
}

const Array<JSON>&
JSON::get_array() const
{
	cassert(type == Type::array);
	return *array.get();
}

const AArray<JSON>&
JSON::get_object() const
{
	cassert(type == Type::object);
	return *aarray.get();
}

bool
JSON::is_null() const
{
	return (type == Type::null);
}

