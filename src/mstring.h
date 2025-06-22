/*
 * Copyright (c) 2001,02,03,04,08,09,10 Bernd Walter Computer Technology
 * Copyright (c) 2008,09,10 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/string.h $
 * $Date: 2025-06-19 17:44:40 +0200 (Thu, 19 Jun 2025) $
 * $Author: ticso $
 * $Rev: 49399 $
 */

#ifndef _STRING
#define _STRING

#define S String()
#define M_STR(str) #str
#define MSTR(str) M_STR(str)

#include <codecvt>

namespace bwct
{

	class JSON;

	class String {
	private:
		static constexpr size_t directsize = (sizeof(char*));
		size_t ln;
		size_t buflen;
		union {
			char *data;
			char sdata[directsize];
		};
		uint32_t *udata;
		void rebufsize(size_t len);
		void bufsize(size_t len);
		char* get_data() noexcept;
		const char* get_data() const noexcept;
		void free_data() noexcept;
	public:
		template <bool IsConst>
		struct Iterator {
		public:
			using iterator_category = std::random_access_iterator_tag;
			using value_type      = typename std::conditional_t<IsConst, const char, char>;
			using reference         = value_type&;
			using pointer           = value_type*;
			using difference_type   = std::ptrdiff_t;

			friend class String;

		private:
			value_type* pos;

		public:
			Iterator(const Iterator&) = default;
			Iterator& operator=(const Iterator&) = default;

			template<bool WasConst, class = std::enable_if_t<IsConst && !WasConst>>
			Iterator(const Iterator<WasConst>& rhs) : pos(rhs.pos) {}

			template<bool WasConst, class = std::enable_if_t<IsConst && !WasConst>>
			Iterator& operator=(const Iterator<WasConst>& rhs) {
				pos = rhs.pos;
				return (*this);
			}

			Iterator(value_type* rpos) {
				pos = rpos;
			}

			pointer operator->() const {
				return pos;
			}

			reference operator*() const {
				return *pos;
			}

			Iterator& operator--() {
				--pos;
				return *this;
			}

			Iterator operator--(int) {
				Iterator tmp = *this;
				--(*this);
				return tmp;
			}

			Iterator& operator++() {
				++pos;
				return *this;
			}

			Iterator operator++(int) {
				Iterator tmp = *this;
				++(*this);
				return tmp;
			}

			Iterator& operator+=(int a) {
				pos += a;
				return *this;
			}

			Iterator& operator-=(int a) {
				pos -= a;
				return *this;
			}

			reference operator[](int i) {
				return pos[i];
			}

			friend Iterator operator+(Iterator a, int b) {
				a += b;
				return a;
			}

			friend Iterator operator-(Iterator a, int b) {
				a -= b;
				return a;
			}

			friend int operator-(const Iterator& a, const Iterator& b) {
				return a.pos - b.pos;
			}

			friend bool operator< (const Iterator& a, const Iterator& b) {
				return a.pos < b.pos;
			}

			friend bool operator== (const Iterator& a, const Iterator& b) {
				return a.pos == b.pos;
			}

			friend bool operator!= (const Iterator& a, const Iterator& b) {
				return a.pos != b.pos;
			}
		};

		using iterator = Iterator<false>;
		using const_iterator = Iterator<true>;

		iterator begin() {
			return iterator(get_data());
		}
		iterator end() {
			return iterator(get_data() + ln + 1);
		}
		const_iterator begin() const {
			return const_iterator(get_data());
		}
		const_iterator end() const {
			return const_iterator(get_data() + ln + 1);
		}
		const_iterator cbegin() const {
			return const_iterator(get_data());
		}
		const_iterator cend() const {
			return const_iterator(get_data() + ln + 1);
		}
		auto rbegin() {
			return std::make_reverse_iterator(iterator(&(get_data()[ln])));
		}
		auto rend() {
			return std::make_reverse_iterator(iterator(&(get_data()[-1])));
		}
		auto rbegin() const {
			return std::make_reverse_iterator(const_iterator(&(get_data()[ln])));
		}
		auto rend() const {
			return std::make_reverse_iterator(const_iterator(&(get_data()[-1])));
		}
		auto crbegin() const {
			return std::make_reverse_iterator(const_iterator(&(get_data()[ln])));
		}
		auto crend() const {
			return std::make_reverse_iterator(const_iterator(&(get_data()[-1])));
		}

		String() noexcept;
		String(const char *rhs);
		String(const String &rhs);
		String(String &&rhs) noexcept;
		String(char rhs);
		String(short rhs);
		String(int rhs);
		String(long rhs);
		String(long long rhs);
		String(bool rhs) noexcept;
		String(unsigned char rhs);
		String(unsigned short rhs);
		String(unsigned int rhs);
		String(unsigned long rhs);
		String(unsigned long long rhs);
		String(double rhs);
		String(const void *rhs);
		String(File rhs);
		String(const JSON&  rhs);
		String(const Array<String>& rhs);
		void set_size(size_t len);
		void add_memory(const void* data, size_t len);
		const String& operator= (const String &rhs);
		const String& operator= (String &&rhs) noexcept;
		const String& operator= (const Array<String>& rhs);
		const String& operator= (char *rhs);
		const String& operator= (const char *rhs);
		const String& operator= (bool rhs);
		const String& operator= (unsigned int rhs);
		const String& operator= (unsigned long rhs);
		const String& operator= (unsigned long long rhs);
		const String& operator= (double rhs);
		const String& operator= (const void *rhs);
		const String& operator= (File rhs);
		const String& operator= (const JSON& rhs);
		const String& operator=(const std::string& rhs);
		template <class T>
		const String& operator=(const T &rhs);
		const String& join (const Array<String>& rhs, const String &bind);
		size_t length() const {
			return ln;
		}
		size_t u_length() const;
		const char *c_str() const noexcept;
		const uint32_t *u_str();
		~String() noexcept;
		bool operator== (const String &rhs) const noexcept;
		bool operator== (const char *rhs) const noexcept;
		bool operator!= (const String &rhs) const noexcept;
		bool operator!= (const char *rhs) const noexcept;
		template <class T>
		String operator+(const T &rhs) const;
		String operator+(const String &rhs) const;
		String operator+(String &&rhs) const;
		template <class T>
		String& operator<<(const T &rhs);
		bool operator< (const String &rhs) const noexcept;
		bool operator<= (const String &rhs) const noexcept;
		bool operator> (const String &rhs) const noexcept;
		bool operator>= (const String &rhs) const noexcept;
		String& operator+=(const String &rhs);
		String& operator+=(String &&rhs);
		String& operator+=(const Array<String>& rhs);
		String& operator+=(const char *rhs);
		String& operator+=(bool rhs);
		String& operator+=(char rhs);
		String& operator+=(short rhs);
		String& operator+=(int rhs);
		String& operator+=(long rhs);
		String& operator+=(long long rhs);
		String& operator+=(unsigned char rhs);
		String& operator+=(unsigned short rhs);
		String& operator+=(unsigned int rhs);
		String& operator+=(unsigned long rhs);
		String& operator+=(unsigned long long rhs);
		String& operator+=(double rhs);
		String& operator+=(const void *rhs);
		String trim() const noexcept;
		String trimstart(const char value) const noexcept;
		String trimend(const char value) const noexcept;
		void resize(size_t rhs);
		bool empty() const noexcept;
		void clear(void) noexcept;
		void lower() noexcept;
		void upper() noexcept;
		bool strncmp(const String &rhs, size_t len) const noexcept;
		bool strncmp(const String &rhs) const noexcept;
		bool strncmp(const char *rhs) const noexcept;
		bool strncmp(int64_t frompos, const String &rhs) const;
		bool replacefirst(const String &search, const String &replace);
		bool contains(const String &rhs) const noexcept;
		int replace(const String &search, const String &replace);
		String cut(size_t begin, ssize_t end = -1) const;
		String u_cut(size_t begin, ssize_t end = -1) const;
		Array<String> split(String delim) const;
		Array<String> split(String delim, int64_t len) const;
		Array<String> strsplit(String delim) const;
		Array<String> CSVsplit() const;
		Array<String> linesplit() const;
		long long getll() const noexcept;
		double getd() const noexcept;
		String re_subst(const String& re);
		bool re_comp(const String& re) const;
		String printf(String format, ...);
		bool is_numeric() noexcept;
		void reverse();
		const char& operator[](const size_t i) const;
		char& operator[](const size_t i);
		const char& at(const size_t i) const;
		char& at(const size_t i);
		const String& convert(const String& from, const String& to);
		bool test_utf() const;
		SArray<uint8_t> hex_to_bytes() const;
		String tinfo() const;
		void log(int priority, const String& str) const noexcept;
		void log(int priority, const char *str) const noexcept;
		void log(const String& str) const noexcept;
		void log(const char *str) const noexcept;
		void push_back(char c);
		void push_front(char c);
		void pop_front();
		void pop_back();
		friend std::ostream& operator<< (std::ostream& out, const String& rh);
	};

	template <class T>
	String&
	String::operator<<(const T &rhs) {
		*this += String(rhs);
		return *this;
	}

	template <class T>
	String
	String::operator+(const T &rhs) const {
		String nstr(*this);
		nstr += rhs;
		return nstr;
	}

	template <class T>
	const String&
	String::operator=(const T& rhs) {
		const char* rstr = rhs.c_str();

		if (rstr == NULL) {
			log ("rstr == NULL");
			rstr = "(null)";
		}
		bufsize(strlen(rstr) + 1);
		strcpy(get_data(), rstr);
		ln = strlen(get_data());
		return *this;
	}

	constexpr uint32_t crc_hash(const char *key, uint32_t len) noexcept;
}

template<>
struct std::hash<bwct::String>
{
	constexpr std::size_t operator()(const bwct::String& s) const noexcept
	{
		return bwct::crc_hash(s.c_str(), s.length());
	}
};

#endif /* !_STRING */
