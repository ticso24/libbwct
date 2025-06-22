/*
 * Copyright (c) 2001-2014 Bernd Walter Computer Technology
 * Copyright (c) 2008-2014 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/json.h $
 * $Date: 2025-05-26 13:11:59 +0200 (Mon, 26 May 2025) $
 * $Author: ticso $
 * $Rev: 49280 $
 */

#ifndef _JSON
#define _JSON

#include "db.h"
#include "tool.h"
#include "aarray.h"
#include "array.h"
#include <memory>

#ifndef VARIANT_DISABLE
#if __cplusplus >= 201703L
#include <variant>
#define WITH_VARIANT
#endif
#endif

namespace bwct
{
	class JSON;

	class JSON {
	public:
		class Error : public bwct::Error {
		public:
			Error(const String& msg)
			    : bwct::Error(msg.c_str()) {
			}
			Error(const char* msg)
			    : bwct::Error(msg) {
			}
		};

		// we use the enum for clarity over the std::variant state numbers as well
		enum class Type {
			null = 0,	// no storage
			string,
			object,		// aaray of subnodes
			number,		// use string container and let caller decide wether frac, int, ...
			array,		// array of subnodes
			boolean
		};

#ifdef WITH_VARIANT
		std::variant<std::monostate, String, AArray<JSON>, String, Array<JSON>, bool> data;

#else

	private:
		Type type;
		bool bool_state;
		String *str;
		Array<JSON> *array;
		AArray<JSON> *aarray;
#endif

#ifndef WITH_VARIANT
		void clear();
#endif
		struct Parserargs {
			const String& json;
			int64_t parserpos;
			int64_t linenr;
			int64_t columnnr;
			Parserargs(const String& str) : json(str){
				parserpos = 0;
				linenr = 1;
				columnnr = 1;
			}
		};
		String parseerrormsg(const char* msg, Parserargs& args) const;
		void iparse(Parserargs& args);
		void parsewhitespace(Parserargs& args);
		String parsestring(Parserargs& args);

		static String ESC(const String& val);
		void int_generate(String& val, bool formated, int level) const;

	public:
		JSON();
		JSON(const JSON& rh);
		JSON(JSON&& rh) noexcept;
		~JSON() noexcept;

		void parse(const String& json);
		String generate(bool newline = false) const;
		void create_table(AArray<JSON>& val, String path) const;

		const JSON& operator=(const JSON& rh);
		const JSON& operator=(JSON&& rh) noexcept;
		const JSON& operator=(bool rh) noexcept;
		const JSON& operator=(const String& rh);
		const JSON& operator=(String&& rh) noexcept;
		const JSON& operator=(const char* rh);
		const JSON& operator=(int64_t rh);
		const JSON& operator=(const Array<JSON>& rh);
		const JSON& operator=(Array<JSON>&& rh) noexcept;
		const JSON& operator=(const AArray<JSON>& rh);
		const JSON& operator=(AArray<JSON>&& rh) noexcept;
		const JSON& set_null() noexcept;

		template <class T>
		const JSON& set_number(const T &rh) {
#ifdef WITH_VARIANT
			data.emplace<(int)Type::number>(rh);
#else
			clear();
			type = Type::number;
			delete str;
			str = new String;
			*str = rh;
#endif
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
		bool is_null() const noexcept;
		bool is_string() const noexcept;
		bool is_object() const noexcept;
		bool is_number() const noexcept;
		bool is_array() const noexcept;
		bool is_boolean() const noexcept;
		const String& get_numstr() const;
		const String& get_str() const;
		const char* c_str() const;
		const Array<JSON>& get_array() const;
		int64_t get_max() const;
		Array<JSON>& get_array();
		const AArray<JSON>& get_object() const;
		AArray<JSON>& get_object();
		bool exists(const String& rh) const;
		bool is_type(const String& rh) const noexcept;
		Type get_type() const noexcept;
		String tinfo() const;
	};
}

#endif /* !_JSON */
