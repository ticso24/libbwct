/*
 * Copyright (c) 2001,02,03,08,11,12 Bernd Walter Computer Technology
 * Copyright (c) 2008,11,12 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/json.cc $
 * $Date: 2021-07-22 12:34:19 +0200 (Thu, 22 Jul 2021) $
 * $Author: ticso $
 * $Rev: 44553 $
 */

#include "bwct.h"

JSON::JSON()
{
	str = NULL;
	array = NULL;
	aarray = NULL;
	type = Type::null;
}

JSON::JSON(const JSON& rh)
{
	str = NULL;
	array = NULL;
	aarray = NULL;

	type = rh.type;
	switch(type) {
	case Type::string:
	case Type::number:
		str = new String;
		*str = *rh.str;
		break;
	case Type::boolean:
		bool_state = rh.bool_state;
		break;
	case Type::array:
		array = new Array<JSON>;
		*array = *rh.array;
		break;
	case Type::object:
		aarray = new AArray<JSON>;
		*aarray = *rh.aarray;
		break;
	case Type::null:
		break;
	}
}

JSON::JSON(JSON&& rh)
{
	str = NULL;
	array = NULL;
	aarray = NULL;

	type = rh.type;
	switch(type) {
	case Type::string:
	case Type::number:
		str = rh.str;
		rh.str = NULL;
		break;
	case Type::boolean:
		bool_state = rh.bool_state;
		break;
	case Type::array:
		array = rh.array;
		rh.array = NULL;
		break;
	case Type::object:
		aarray = rh.aarray;
		rh.aarray = NULL;
		break;
	case Type::null:
		break;
	}
}

JSON::~JSON()
{
	clear();
}

void
JSON::parse(const String& json)
{
	int64_t parserpos = 0;
	iparse(json, parserpos);
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
							hval |= c - '0';
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
		delete str;
		str = NULL;
		str = new String;
		*str = parsestring(json, parserpos);
	} else if (json.strncmp(parserpos, "[")) {
		{
			bool cont = true;
			type = Type::array;
			delete array;
			array = NULL;
			array = new Array<JSON>;
			parserpos += 1;
			parsewhitespace(json, parserpos);
			if (json[parserpos] == ']') {
				parserpos += 1;
				parsewhitespace(json, parserpos);
				cont = false;
			}
			while (cont) {
				int64_t newpos = array->max + 1;
				(*array)[newpos].iparse(json, parserpos);
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
			delete aarray;
			aarray = NULL;
			aarray = new AArray<JSON>;
			parserpos += 1;
			parsewhitespace(json, parserpos);
			if (json[parserpos] == '}') {
				parserpos += 1;
				parsewhitespace(json, parserpos);
				cont = false;
			}
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
				(*aarray)[key].iparse(json, parserpos);
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
					if (c == '.' && val == "") { // floats must start with '0'
						TError("Not a number");
					} else if ((val == "0" || val == "-0" || val == "+0") && !(c == '.' || c == 'e' || c == 'E')) { // numbers must not have leading zeros
						TError("Not a number");
					}
					tmp[0] = c;
					val += tmp;
					parserpos += 1;
					break;
				default:
					cont = false;
				}
			}
			char *p_end;
			std::strtod(val.c_str(), &p_end);
			if (*p_end != 0) {
			    TError("Not a number");
			}
			delete str;
			str = NULL;
			str = new String;
			*str = val;
		}
	}
	parsewhitespace(json, parserpos);
}

void
JSON::clear()
{
	type = Type::null;
	delete str;
	str = NULL;
	delete array;
	array = NULL;
	delete aarray;
	aarray = NULL;
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
			ret += "\\b";
			break;
		case '\t':
			ret += "\\t";
			break;
		case '\n':
			ret += "\\n";
			break;
		case '\f':
			ret += "\\f";
			break;
		case '\r':
			ret += "\\r";
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
JSON::generate(bool formated) const
{
	String ret;
	Array<String> data;
	int_generate(data, formated, 0);
	if (formated) {
		data << S + "\n";
	}
	ret = std::move(data);
	return ret;
}

void
JSON::int_generate(Array<String>& data, bool formated, int level) const
{
	String nl;
	String indent;
	String indentx;
	String space;
	if (formated) {
		nl = "\n";
		char tmp[level + 1];
		memset(tmp, '\t', level);
		tmp[level] = '\0';
		indent = tmp;
		indentx = indent + "\t";
		space = " ";
	}
	switch(type) {
	case Type::null:
		data << S + "null";
		break;
	case Type::string:
		data << S + "\"" + ESC(*str) + "\"";
		break;
	case Type::object:
		{
			Array<String> keys = aarray->getkeys(true);
			data << S + "{" + nl;
			for (int i = 0; i <= keys.max; i++) {
				data << indentx + "\"" + ESC(keys[i]) + "\"" + space + ":" + space;
				(*aarray)[keys[i]].int_generate(data, formated, level + 1);
				if (i != keys.max) {
					data << S + "," + nl;
				} else if (formated) {
					data << nl;
				}
			}
			data << indent + "}";
		}
		break;
	case Type::number:
		data << *str;
		break;
	case Type::array:
		data << S + "[" + nl;
		for (int i = 0; i <= array->max; i++) {
			if (formated) {
				data << indentx;
			}
			(*array)[i].int_generate(data, formated, level + 1);
			if (i != array->max) {
				data << S + "," + nl;
			} else if (formated) {
				data << nl;
			}
		}
		data << indent + "]";
		break;
	case Type::boolean:
		data << S + ((bool_state) ? "true" : "false");
		break;
	}
}

const JSON&
JSON::operator=(const JSON& rh)
{
	clear();
	type = rh.type;
	switch(type) {
	case Type::string:
	case Type::number:
		str = new String;
		*str = *rh.str;
		break;
	case Type::boolean:
		bool_state = rh.bool_state;
		break;
	case Type::array:
		array = new Array<JSON>;
		*array = *rh.array;
		break;
	case Type::object:
		aarray = new AArray<JSON>;
		*aarray = *rh.aarray;
		break;
	case Type::null:
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
JSON::operator=(const char* rh)
{
	clear();
	type = Type::string;
	str = new String;
	*str = rh;
	return *this;
}

const JSON&
JSON::operator=(const String& rh)
{
	clear();
	type = Type::string;
	str = new String;
	*str = rh;
	return *this;
}

const JSON&
JSON::operator=(String&& rh)
{
	clear();
	type = Type::string;
	str = new String;
	*str = std::move(rh);
	return *this;
}

const JSON&
JSON::operator=(int64_t rh)
{
	clear();
	type = Type::number;
	str = new String;
	*str = String(rh);
	return *this;
}

const JSON&
JSON::operator=(const Array<JSON>& rh)
{
	clear();
	type = Type::array;
	array = new Array<JSON>;
	*array = rh;
	return *this;
}

const JSON&
JSON::operator=(Array<JSON>&& rh)
{
	clear();
	type = Type::array;
	array = new Array<JSON>;
	*array = std::move(rh);
	return *this;
}

const JSON&
JSON::operator=(const AArray<JSON>& rh)
{
	clear();
	type = Type::object;
	aarray = new AArray<JSON>;
	*aarray = rh;
	return *this;
}

const JSON&
JSON::operator=(AArray<JSON>&& rh)
{
	clear();
	type = Type::object;
	aarray = new AArray<JSON>;
	*aarray = std::move(rh);
	return *this;
}

const JSON&
JSON::set_null()
{
	clear();
	type = Type::null;
	return *this;
}

bool
JSON::operator==(const char* rh) const
{
	cassertm(type == Type::string, rh);
	return *str == rh;
}

bool
JSON::operator==(const String& rh) const
{
	cassertm(type == Type::string, rh.c_str());
	return *str == rh;
}

bool
JSON::operator!=(const char* rh) const
{
	cassertm(type == Type::string, rh);
	return *str != rh;
}

bool
JSON::operator!=(const String& rh) const
{
	cassertm(type == Type::string, rh.c_str());
	return *str != rh;
}

const JSON&
JSON::operator[](const char* rh) const
{
	cassertm(type == Type::object, rh);
	JSON* ret;
	ret = &(*aarray)[rh];
	return *ret;
}

JSON&
JSON::operator[](const char* rh)
{
	cassertm(type == Type::object, rh);
	JSON* ret;
	ret = &(*aarray)[rh];
	return *ret;
}

const JSON&
JSON::operator[](const String& rh) const
{
	cassertm(type == Type::object, rh.c_str());
	JSON* ret;
	ret = &(*aarray)[rh];
	return *ret;
}

JSON&
JSON::operator[](const String& rh)
{
	cassertm(type == Type::object, rh.c_str());
	JSON* ret;
	ret = &(*aarray)[rh];
	return *ret;
}

const JSON&
JSON::operator[](int64_t rh) const
{
	cassertm(type == Type::array, reinterpret_cast<const char*>(rh));
	JSON* ret;
	ret = &(*array)[rh];
	return *ret;
}

JSON&
JSON::operator[](int64_t rh)
{
	cassertm(type == Type::array, reinterpret_cast<const char*>(rh));
	JSON* ret;
	ret = &(*array)[rh];
	return *ret;
}

JSON::operator bool() const
{
	cassertm(type == Type::boolean, reinterpret_cast<const char*>(type));
	return bool_state;
}

const String&
JSON::get_numstr() const
{
	cassertm(type == Type::number, reinterpret_cast<const char*>(type));
	return *str;
}
const String&
JSON::get_str() const
{
	cassertm(type == Type::string, reinterpret_cast<const char*>(type));
	return *str;
}

const char*
JSON::c_str() const
{
	cassertm(type == Type::string, reinterpret_cast<const char*>(type));
	return str->c_str();
}

Array<JSON>&
JSON::get_array()
{
	cassertm(type == Type::array, reinterpret_cast<const char*>(type));
	return *array;
}

int64_t
JSON::get_max() const
{
	cassertm(type == Type::array, reinterpret_cast<const char*>(type));
	return array->max;
}

AArray<JSON>&
JSON::get_object()
{
	cassertm(type == Type::object, reinterpret_cast<const char*>(type));
	return *aarray;
}

bool
JSON::exists(const String& rh) const
{
	cassertm(type == Type::object, rh.c_str());
	return aarray->exists(rh);
}

const Array<JSON>&
JSON::get_array() const
{
	cassertm(type == Type::array, reinterpret_cast<const char*>(type));
	return *array;
}

const AArray<JSON>&
JSON::get_object() const
{
	cassertm(type == Type::object, reinterpret_cast<const char*>(type));
	return *aarray;
}

Array<JSON>
JSON::query(const String& q) const
{
	Array<JSON> ret;
	//const JSON& res;

	return ret;
}

bool
JSON::is_null() const
{
	return (type == Type::null);
}

bool
JSON::is_string() const
{
	return (type == Type::string);
}

bool
JSON::is_object() const
{
	return (type == Type::object);
}

bool
JSON::is_number() const
{
	return (type == Type::number);
}

bool
JSON::is_array() const
{
	return (type == Type::array);
}

bool
JSON::is_boolean() const
{
	return (type == Type::boolean);
}

bool
JSON::is_type(const String& t) const
{
	bool ret = false;

	if (t == "string" && type == Type::string) {
		ret = true;
	} else if (t == "number" && type == Type::number) {
		ret = true;
	} else if (t == "boolean" && type == Type::boolean) {
		ret = true;
	} else if (t == "array" && type == Type::array) {
		ret = true;
	} else if (t == "aarray" && type == Type::object) {
		ret = true;
	}

	return ret;
}

JSON::Type
JSON::get_type() const
{
	return type;
}

