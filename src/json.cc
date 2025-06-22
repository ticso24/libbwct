/*
 * Copyright (c) 2001,02,03,08,11,12 Bernd Walter Computer Technology
 * Copyright (c) 2008,11,12 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/json.cc $
 * $Date: 2025-06-22 18:40:52 +0200 (Sun, 22 Jun 2025) $
 * $Author: ticso $
 * $Rev: 49411 $
 */

#include "bwct.h"

namespace bwct
{

	JSON::JSON()
	{
#ifdef WITH_VARIANT
		data = std::monostate();
#else
		str = NULL;
		array = NULL;
		aarray = NULL;
		type = Type::null;
#endif
	}

	JSON::JSON(const JSON& rh)
	{
#ifdef WITH_VARIANT
		data = rh.data;
#else
		str = NULL;
		array = NULL;
		aarray = NULL;

		try {
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
		} catch (...) {
			clear();
			throw;
		}
#endif
	}

	JSON::JSON(JSON&& rh) noexcept
	{
#ifdef WITH_VARIANT
		data = std::move(rh.data);
#else
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
#endif
	}

	JSON::~JSON() noexcept
	{
#ifndef WITH_VARIANT
		clear();
#endif
	}

	void
	JSON::parse(const String& json)
	{
		Parserargs args(json);
		iparse(args);
		cassert(args.parserpos == (int64_t)json.length());
	}

	String
	JSON::parseerrormsg(const char* msg, Parserargs& args) const
	{
		String fullmsg;
		return fullmsg.printf("%s in line:%ld column:%ld", msg, args.linenr, args.columnnr);
	}

	void
	JSON::parsewhitespace(Parserargs& args)
	{
		const char* str = args.json.c_str();
		for(;;) {
			char c = str[args.parserpos];
			switch (c) {
			case '\n':
				args.parserpos++;
				args.linenr++;
				args.columnnr = 1;
				break;
			case ' ':
			case '\r':
			case '\t':
				args.parserpos++;
				args.columnnr++;
				break;
			default:
				return;
			}
		}
	}

	String
	JSON::parsestring(Parserargs& args)
	{
		const char* str = args.json.c_str();
		String ret;
		uint8_t c;

		while ((c = str[args.parserpos]) != '"') {
			switch (c) {
			case '\\':
				args.parserpos += 1;
				args.columnnr += 1;
				c = str[args.parserpos];
				switch(c) {
				case '\\':
				case '"':
				case '/':
					ret.push_back(c);
					args.parserpos += 1;
					args.columnnr += 1;
					break;
				case 'u':
					{
						args.parserpos += 1;
						uint16_t hval = 0;
						for (int i = 0; i < 4; i++) {
							hval <<= 4;
							c = str[args.parserpos];
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
								TError(parseerrormsg("\\u encoding error", args));
							}
							args.parserpos += 1;
							args.columnnr += 1;
						}
						if (hval < 0x80) {
							ret.push_back(hval & 0x7f);
						} else if (hval < 0x0800) {
							ret.push_back(0xc0 + ((hval >> 6) & 0x1f));
							ret.push_back(0x80 + (((hval >> 0) & 0x3f)));
						} else {
							// we only handle up to 16bit, so this is the last one to handle
							ret.push_back(0xe0 + ((hval >> 12) & 0x1f));
							ret.push_back(0x80 + (((hval >> 6) & 0x3f)));
							ret.push_back(0x80 + (((hval >> 0) & 0x3f)));
						}
					}
					break;
				case 'b':
					ret.push_back(0x08);
					args.parserpos += 1;
					args.columnnr += 1;
					break;
				case 'f':
					ret.push_back(0x0c);
					args.parserpos += 1;
					args.columnnr += 1;
					break;
				case 'n':
					ret.push_back('\n');
					args.parserpos += 1;
					args.columnnr += 1;
					break;
				case 'r':
					ret.push_back('\r');
					args.parserpos += 1;
					args.columnnr += 1;
					break;
				case 't':
					ret.push_back('\t');
					args.parserpos += 1;
					args.columnnr += 1;
					break;
				default:
					TError(parseerrormsg("\\ encoding error", args));
				}
				break;
			default:
				ret.push_back(c);
				args.parserpos += 1;
				args.columnnr += 1;
				break;
			}
		}
		args.parserpos += 1;
		args.columnnr += 1;
		return ret;
	}

	void
	JSON::iparse(Parserargs& args)
	{
		const char* str = args.json.c_str();
#ifndef WITH_VARIANT
		clear();
#endif
		parsewhitespace(args);
		if (args.json.strncmp(args.parserpos, "\"")) {
			args.parserpos += 1;
			args.columnnr += 1;
#ifdef WITH_VARIANT
			data.emplace<(int)Type::string>(parsestring(args));
#else
			type = Type::string;
			delete str;
			str = NULL;
			str = new String;
			*str = parsestring(args);
#endif
		} else if (args.json.strncmp(args.parserpos, "[")) {
			{
				bool cont = true;
#ifdef WITH_VARIANT
				data = Array<JSON>();
#else
				type = Type::array;
				delete array;
				array = NULL;
				array = new Array<JSON>;
#endif
				args.parserpos += 1;
				args.columnnr += 1;
				parsewhitespace(args);
				if (str[args.parserpos] == ']') {
					args.parserpos += 1;
					args.columnnr += 1;
					parsewhitespace(args);
					cont = false;
				}
				while (cont) {
#ifdef WITH_VARIANT
					auto* array = &std::get<Array<JSON>>(data);
#endif
					int64_t newpos = array->max + 1;
					(*array)[newpos].iparse(args);
					if (str[args.parserpos] != ',') {
						if (str[args.parserpos] != ']') {
							TError(parseerrormsg("no closing ']'", args));
						}
						args.parserpos += 1;
						args.columnnr += 1;
						cont = false;
					} else {
						args.parserpos += 1;
						args.columnnr += 1;
						parsewhitespace(args);
					}
				}
			}
		} else if (args.json.strncmp(args.parserpos, "{")) {
			{
				bool cont = true;
#ifdef WITH_VARIANT
				data = AArray<JSON>();
#else
				type = Type::object;
				delete aarray;
				aarray = NULL;
				aarray = new AArray<JSON>;
#endif
				args.parserpos += 1;
				args.columnnr += 1;
				parsewhitespace(args);
				if (str[args.parserpos] == '}') {
					args.parserpos += 1;
					args.columnnr += 1;
					parsewhitespace(args);
					cont = false;
				}
				while (cont) {
					if (str[args.parserpos] != '"') {
						TError(parseerrormsg("no key string", args));
					}
					args.parserpos += 1;
					args.columnnr += 1;
					String key = parsestring(args);
					parsewhitespace(args);
					if (str[args.parserpos] != ':') {
						TError(parseerrormsg("missing \":\"", args));
					}
					args.parserpos += 1;
					args.columnnr += 1;
					parsewhitespace(args);
#ifdef WITH_VARIANT
					auto* aarray = &std::get<AArray<JSON>>(data);
#endif
					(*aarray)[key].iparse(args);
					if (str[args.parserpos] != ',') {
						if (str[args.parserpos] != '}') {
							TError(parseerrormsg("no closing '}'", args));
						}
						args.parserpos += 1;
						args.columnnr += 1;
						cont = false;
					} else {
						args.parserpos += 1;
						args.columnnr += 1;
						parsewhitespace(args);
					}
				}
			}
		} else if (args.json.strncmp(args.parserpos, "true")) {
#ifdef WITH_VARIANT
			data = true;
#else
			type = Type::boolean;
			bool_state = true;
#endif
			args.parserpos += 4;
			args.columnnr += 4;
		} else if (args.json.strncmp(args.parserpos, "false")) {
#ifdef WITH_VARIANT
			data = false;
#else
			type = Type::boolean;
			bool_state = false;
#endif
			args.parserpos += 5;
			args.columnnr += 5;
		} else if (args.json.strncmp(args.parserpos, "null")) {
#ifdef WITH_VARIANT
			data = std::monostate();
#else
			type = Type::null;
#endif
			args.parserpos += 4;
			args.columnnr += 4;
		} else {
			{
				// probe for numberic data
#ifndef WITH_VARIANT
				type = Type::number;
#endif
				bool cont = true;
				String val;
				while (cont) {
					uint8_t c = str[args.parserpos];
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
							TError(parseerrormsg("Not a number", args));
						} else if ((val == "0" || val == "-0" || val == "+0") && !(c == '.' || c == 'e' || c == 'E')) { // numbers must not have leading zeros
							TError(parseerrormsg("Not a number", args));
						}
						val.push_back(c);
						args.parserpos += 1;
						args.columnnr += 1;
						break;
					default:
						cont = false;
					}
				}
				char *p_end;
				std::strtod(val.c_str(), &p_end);
				if (*p_end != 0) {
				    TError(parseerrormsg("Not a number", args));
				}
#ifdef WITH_VARIANT
				data.emplace<(int)Type::number>(val);
#else
				delete str;
				str = NULL;
				str = new String;
				*str = val;
#endif
			}
		}
		parsewhitespace(args);
	}

	void
	JSON::create_table(AArray<JSON>& val, String path) const
	{
#ifdef WITH_VARIANT
		switch((Type)data.index()) {
#else
		switch(type) {
#endif
		case Type::null:
		case Type::string:
		case Type::number:
		case Type::boolean:
			val[path] = *this;
			break;
		case Type::object:
			{
#ifdef WITH_VARIANT
				auto* aarray = &std::get<AArray<JSON>>(data);
#endif
				Array<String> keys = aarray->getkeys(true);
				for (int i = 0; i <= keys.max; i++) {
					String subpath = path + "[\"" + keys[i] + "\"]";
					(*aarray)[keys[i]].create_table(val, subpath);
				}
			}
			break;
		case Type::array:
#ifdef WITH_VARIANT
			auto* array = &std::get<Array<JSON>>(data);
#endif
			for (int i = 0; i <= array->max; i++) {
				String subpath = path + "[" + i + "]";
				(*array)[i].create_table(val, subpath);
			}
			break;
		}
	}

#ifndef WITH_VARIANT
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
#endif

	String
	JSON::ESC(const String& val)
	{
		String ret;
		const char* plh;
		uint8_t tmp;

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
						ret.push_back('a' + nibble - 10);
					} else {
						ret.push_back('0' + nibble);
					}
					nibble = tmp & 0xf;
					if (nibble > 9) {
						ret.push_back('a' + nibble - 10);
					} else {
						ret.push_back('0' + nibble);
					}
				} else {
					ret.push_back(tmp);
				}
			}
		}
		return ret;
	}

	String
	JSON::generate(bool formated) const
	{
		String ret;
		int_generate(ret, formated, 0);
		if (formated) {
			ret += S + "\n";
		}
		return ret;
	}

	void
	JSON::int_generate(String& val, bool formated, int level) const
	{
		String indent;
		String indentx;
		if (formated) {
			for (int i; i < level; ++i) {
				indent.push_back('\t');
			}
			indentx = indent;
			indentx.push_back('\t');
		}
#ifdef WITH_VARIANT
		switch((Type)data.index()) {
#else
		switch(type) {
#endif
		case Type::null:
			val += S + "null";
			break;
		case Type::string:
#ifdef WITH_VARIANT
			val += S + "\"" + ESC(std::get<(int)Type::string>(data)) + "\"";
#else
			val += S + "\"" + ESC(*str) + "\"";
#endif
			break;
		case Type::object:
			{
#ifdef WITH_VARIANT
				auto* aarray = &std::get<AArray<JSON>>(data);
#endif

				if (formated) {
					auto pairs = aarray->getpairs(true);
					val += S + "{\n";
					for (int i = 0; i <= pairs.max; i++) {
						val += indentx + "\"" + ESC(*pairs[i].first) + "\" : ";
						pairs[i].second->int_generate(val, formated, level + 1);
						if (i != pairs.max) {
							val += S + ",\n";
						} else {
							val += "\n";
						}
					}
					val += indent + "}";
				} else {
					val += S + "{";
					for (auto it = aarray->begin(); it != aarray->end(); ++it) {
						val += S + "\"" + ESC(it->first) + "\":";
						it->second.int_generate(val, formated, level + 1);
						auto tmp_it = it;
						++tmp_it;
						if (tmp_it != aarray->end()) {
							val += S + ",";
						}
					}
					val += "}";
				}
			}
			break;
		case Type::number:
#ifdef WITH_VARIANT
			val += std::get<(int)Type::number>(data);
#else
			val += *str;
#endif
			break;
		case Type::array:
			{
#ifdef WITH_VARIANT
				auto* array = &std::get<Array<JSON>>(data);
#endif
				if (formated) {
					val += S + "[\n";
				} else {
					val += S + "[";
				}
				for (int i = 0; i <= array->max; i++) {
					if (formated) {
						val += indentx;
					}
					(*array)[i].int_generate(val, formated, level + 1);
					if (i != array->max) {
						if (formated) {
							val += S + ",\n";
						} else {
							val += S + ",";
						}
					} else if (formated) {
						val += "\n";
					}
				}
				if (formated) {
					val += indent + "]";
				} else {
					val += "]";
				}
			}
			break;
		case Type::boolean:
#ifdef WITH_VARIANT
			val += (std::get<bool>(data)) ? "true" : "false";
#else
			val += S + ((bool_state) ? "true" : "false");
#endif
			break;
		}
	}

	const JSON&
	JSON::operator=(const JSON& rh)
	{
#ifdef WITH_VARIANT
		data = rh.data;
#else
		clear();
		type = rh.type;
		switch(type) {
		case Type::string:
			str = new String;
			*str = *rh.str;
			break;
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
#endif
		return *this;
	}

	const JSON&
	JSON::operator=(JSON&& rh) noexcept
	{
#ifdef WITH_VARIANT
		data = std::move(rh.data);
#else
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
#endif
		return *this;
	}

	const JSON&
	JSON::operator=(bool rh) noexcept
	{
#ifdef WITH_VARIANT
		data = rh;
#else
		clear();
		type = Type::boolean;
		bool_state = rh;
#endif
		return *this;
	}

	const JSON&
	JSON::operator=(const char* rh)
	{
#ifdef WITH_VARIANT
		data.emplace<(int)Type::string>(rh);
#else
		clear();
		type = Type::string;
		str = new String;
		*str = rh;
#endif
		return *this;
	}

	const JSON&
	JSON::operator=(const String& rh)
	{
#ifdef WITH_VARIANT
		data.emplace<(int)Type::string>(rh);
#else
		clear();
		type = Type::string;
		str = new String;
		*str = rh;
#endif
		return *this;
	}

	const JSON&
	JSON::operator=(String&& rh) noexcept
	{
#ifdef WITH_VARIANT
		data.emplace<(int)Type::string>(std::move(rh));
#else
		clear();
		type = Type::string;
		str = new String;
		*str = std::move(rh);
#endif
		return *this;
	}

	const JSON&
	JSON::operator=(int64_t rh)
	{
#ifdef WITH_VARIANT
		data.emplace<(int)Type::number>(rh);
#else
		clear();
		type = Type::number;
		str = new String;
		*str = String(rh);
#endif
		return *this;
	}

	const JSON&
	JSON::operator=(const Array<JSON>& rh)
	{
#ifdef WITH_VARIANT
		data.emplace<(int)Type::array>(rh);
#else
		clear();
		type = Type::array;
		array = new Array<JSON>;
		*array = rh;
#endif
		return *this;
	}

	const JSON&
	JSON::operator=(Array<JSON>&& rh) noexcept
	{
#ifdef WITH_VARIANT
		data.emplace<(int)Type::array>(std::move(rh));
#else
		clear();
		type = Type::array;
		array = new Array<JSON>;
		*array = std::move(rh);
#endif
		return *this;
	}

	const JSON&
	JSON::operator=(const AArray<JSON>& rh)
	{
#ifdef WITH_VARIANT
		data.emplace<(int)Type::object>(rh);
#else
		clear();
		type = Type::object;
		aarray = new AArray<JSON>;
		*aarray = rh;
#endif
		return *this;
	}

	const JSON&
	JSON::operator=(AArray<JSON>&& rh) noexcept
	{
#ifdef WITH_VARIANT
		data.emplace<(int)Type::object>(std::move(rh));
#else
		clear();
		type = Type::object;
		aarray = new AArray<JSON>;
		*aarray = std::move(rh);
#endif
		return *this;
	}

	const JSON&
	JSON::set_null() noexcept
	{
#ifdef WITH_VARIANT
			data = std::monostate();
#else
		clear();
		type = Type::null;
#endif
		return *this;
	}

	bool
	JSON::operator==(const char* rh) const
	{
#ifdef WITH_VARIANT
		cassertm(data.index() == (int)Type::string, rh);
		return std::get<(int)Type::string>(data) == rh;
#else
		cassertm(type == Type::string, rh);
		return *str == rh;
#endif
	}

	bool
	JSON::operator==(const String& rh) const
	{
#ifdef WITH_VARIANT
		cassertm(data.index() == (int)Type::string, rh.c_str());
		return std::get<(int)Type::string>(data) == rh;
#else
		cassertm(type == Type::string, rh.c_str());
		return *str == rh;
#endif
	}

	bool
	JSON::operator!=(const char* rh) const
	{
#ifdef WITH_VARIANT
		cassertm(data.index() != (int)Type::string, rh);
		return std::get<(int)Type::string>(data) == rh;
#else
		cassertm(type == Type::string, rh);
		return *str != rh;
#endif
	}

	bool
	JSON::operator!=(const String& rh) const
	{
#ifdef WITH_VARIANT
		cassertm(data.index() != (int)Type::string, rh.c_str());
		return std::get<(int)Type::string>(data) == rh;
#else
		cassertm(type == Type::string, rh.c_str());
		return *str != rh;
#endif
	}

	const JSON&
	JSON::operator[](const char* rh) const
	{
#ifdef WITH_VARIANT
		cassertm(data.index() == (int)Type::object, rh);
		return std::get<(int)Type::object>(data)[rh];
#else
		cassertm(type == Type::object, rh);
		JSON* ret;
		ret = &(*aarray)[rh];
		return *ret;
#endif
	}

	JSON&
	JSON::operator[](const char* rh)
	{
#ifdef WITH_VARIANT
		cassertm(data.index() == (int)Type::object, rh);
		return std::get<(int)Type::object>(data)[rh];
#else
		cassertm(type == Type::object, rh);
		JSON* ret;
		ret = &(*aarray)[rh];
		return *ret;
#endif
	}

	const JSON&
	JSON::operator[](const String& rh) const
	{
#ifdef WITH_VARIANT
		cassertm(data.index() == (int)Type::object, rh.c_str());
		return std::get<(int)Type::object>(data)[rh];
#else
		cassertm(type == Type::object, rh.c_str());
		JSON* ret;
		ret = &(*aarray)[rh];
		return *ret;
#endif
	}

	JSON&
	JSON::operator[](const String& rh)
	{
#ifdef WITH_VARIANT
		cassertm(data.index() == (int)Type::object, rh.c_str());
		return std::get<(int)Type::object>(data)[rh];
#else
		cassertm(type == Type::object, rh.c_str());
		JSON* ret;
		ret = &(*aarray)[rh];
		return *ret;
#endif
	}

	const JSON&
	JSON::operator[](int64_t rh) const
	{
#ifdef WITH_VARIANT
		cassertm(data.index() == (int)Type::array, (S + rh).c_str());
		return std::get<(int)Type::array>(data)[rh];
#else
		cassertm(type == Type::array, (S + rh).c_str());
		JSON* ret;
		ret = &(*array)[rh];
		return *ret;
#endif
	}

	JSON&
	JSON::operator[](int64_t rh)
	{
#ifdef WITH_VARIANT
		cassertm(data.index() == (int)Type::array, (S + rh).c_str());
		return std::get<(int)Type::array>(data)[rh];
#else
		cassertm(type == Type::array, (S + rh).c_str());
		JSON* ret;
		ret = &(*array)[rh];
		return *ret;
#endif
	}

	JSON::operator bool() const
	{
#ifdef WITH_VARIANT
		cassertm(data.index() == (int)Type::boolean, (S + data.index()).c_str());
		return std::get<bool>(data);
#else
		cassertm(type == Type::boolean, (S + (int)type).c_str());
		return bool_state;
#endif
	}

	const String&
	JSON::get_numstr() const
	{
#ifdef WITH_VARIANT
		cassertm(data.index() == (int)Type::number, (S + data.index()).c_str());
		return std::get<(int)Type::number>(data);
#else
		cassertm(type == Type::number, (S + (int)type).c_str());
		return *str;
#endif
	}
	const String&
	JSON::get_str() const
	{
#ifdef WITH_VARIANT
		cassertm(data.index() == (int)Type::string, (S + data.index()).c_str());
		return std::get<(int)Type::string>(data);
#else
		cassertm(type == Type::string, (S + (int)type).c_str());
		return *str;
#endif
	}

	const char*
	JSON::c_str() const
	{
#ifdef WITH_VARIANT
		cassertm(data.index() == (int)Type::string, (S + data.index()).c_str());
		return std::get<(int)Type::string>(data).c_str();
#else
		cassertm(type == Type::string, (S + (int)type).c_str());
		return str->c_str();
#endif
	}

	Array<JSON>&
	JSON::get_array()
	{
#ifdef WITH_VARIANT
		cassertm(data.index() == (int)Type::array, (S + data.index()).c_str());
		return const_cast<Array<JSON>&>(std::get<(int)Type::array>(data));
#else
		cassertm(type == Type::array, (S + (int)type).c_str());
		return *array;
#endif
	}

	int64_t
	JSON::get_max() const
	{
#ifdef WITH_VARIANT
		cassertm(data.index() == (int)Type::array, (S + data.index()).c_str());
		return std::get<(int)Type::array>(data).max;
#else
		cassertm(type == Type::array, (S + (int)type).c_str());
		return array->max;
#endif
	}

	AArray<JSON>&
	JSON::get_object()
	{
#ifdef WITH_VARIANT
		cassertm(data.index() == (int)Type::object, (S + data.index()).c_str());
		return (std::get<(int)Type::object>(data));
#else
		cassertm(type == Type::object, (S + (int)type).c_str());
		return *aarray;
#endif
	}

	bool
	JSON::exists(const String& rh) const
	{
#ifdef WITH_VARIANT
		cassertm(data.index() == (int)Type::object, (S + data.index()).c_str());
		return std::get<(int)Type::object>(data).exists(rh);
#else
		cassertm(type == Type::object, rh.c_str());
		return aarray->exists(rh);
#endif
	}

	const Array<JSON>&
	JSON::get_array() const
	{
#ifdef WITH_VARIANT
		cassertm(data.index() == (int)Type::array, (S + data.index()).c_str());
		return std::get<(int)Type::array>(data);
#else
		cassertm(type == Type::array, (S + (int)type).c_str());
		return *array;
#endif
	}

	const AArray<JSON>&
	JSON::get_object() const
	{
#ifdef WITH_VARIANT
		cassertm(data.index() == (int)Type::object, (S + data.index()).c_str());
		return std::get<(int)Type::object>(data);
#else
		cassertm(type == Type::object, (S + (int)type).c_str());
		return *aarray;
#endif
	}

	bool
	JSON::is_null() const noexcept
	{
#ifdef WITH_VARIANT
		auto type = (Type)data.index();
#endif
		return (type == Type::null);
	}

	bool
	JSON::is_string() const noexcept
	{
#ifdef WITH_VARIANT
		auto type = (Type)data.index();
#endif
		return (type == Type::string);
	}

	bool
	JSON::is_object() const noexcept
	{
#ifdef WITH_VARIANT
		auto type = (Type)data.index();
#endif
		return (type == Type::object);
	}

	bool
	JSON::is_number() const noexcept
	{
#ifdef WITH_VARIANT
		auto type = (Type)data.index();
#endif
		return (type == Type::number);
	}

	bool
	JSON::is_array() const noexcept
	{
#ifdef WITH_VARIANT
		auto type = (Type)data.index();
#endif
		return (type == Type::array);
	}

	bool
	JSON::is_boolean() const noexcept
	{
#ifdef WITH_VARIANT
		auto type = (Type)data.index();
#endif
		return (type == Type::boolean);
	}

	bool
	JSON::is_type(const String& t) const noexcept
	{
		bool ret = false;
#ifdef WITH_VARIANT
		auto type = (Type)data.index();
#endif

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
	JSON::get_type() const noexcept
	{
#ifdef WITH_VARIANT
		int type = data.index();
#endif
		return (Type)type;
	}

	String
	JSON::tinfo() const
	{
		String ret;
		ret << "(" << typeid(*this).name() << "@" << this << ")";
		return ret;
	}

}
