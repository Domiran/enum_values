#pragma once
#include <unordered_map>
#include "spliterator.h"
template<class _ETy>
requires std::is_enum_v<_ETy>
class enum_static
{
public:
	using _UTy = std::underlying_type_t<_ETy>; // underlying type

private:
	static const std::unordered_map<_UTy, std::string> value_to_name;
	static const std::unordered_map<std::string, _UTy> name_to_value;
	static const std::string enum_desc;

public:

	static const auto iterable_by_values()
	{
		return value_to_name;
	}

	static const auto iterable_by_names()
	{
		return value_to_name;
	}

	static const auto description()
	{
		return enum_desc;
	}

	static _ETy flag_set(_ETy value, _ETy flag)
	{
		return static_cast<_UTy>(value) | static_cast<_UTy>(flag);
	}

	static _ETy flag_remove(_ETy value, _ETy flag)
	{
		return static_cast<_UTy>(value) & ~static_cast<_UTy>(flag);
	}

	static _ETy flag_toggle(_ETy value, _ETy flag)
	{
		return static_cast<_UTy>(value) ^ static_cast<_UTy>(flag);
	}

	template<typename Ty>
	requires std::is_same_v<_ETy, Ty> || std::is_same_v<_UTy, Ty>
		static bool has_flag(Ty value, _UTy other)
	{
		return (static_cast<_UTy>(value) & other) == other;
	}

	template<typename Ty>
	requires std::is_same_v<_ETy, Ty> || std::is_same_v<_UTy, Ty>
		static bool has_flag(Ty value, _ETy other)
	{
		return has_flag(static_cast<_UTy>(value), static_cast<_UTy>(other));
	}

	template<typename Ty>
	requires std::is_same_v<_ETy, Ty> || std::is_same_v<_UTy, Ty>
		static std::string to_string(Ty value)
	{
		auto iter = value_to_name.find(static_cast<_UTy>(value));
		if (iter == value_to_name.end())
		{
			return std::string();
		}
		return iter->second;
	}

	template<typename Ty>
	requires std::is_same_v<_ETy, Ty> || std::is_same_v<_UTy, Ty>
		static bool from_string(Ty & value, const std::string & text)
	{
		auto iter = name_to_value.find(text);
		if (iter == name_to_value.end())
		{
			return false;
		}
		value = static_cast<Ty>(iter->second);
		return true;
	}

	template<typename Ty>
	requires std::is_same_v<_ETy, Ty> || std::is_same_v<_UTy, Ty>
		static std::string to_flag_string(Ty value, const std::string & separator)
	{
		std::string s;
		for (auto& p : value_to_name)
		{
			if (has_flag(value, p.first))
			{
				s.append(p.second).append(separator);
			}
		}

		if (!s.empty())
		{
			s.erase(s.size() - separator.size()); // remove last separator
		}

		return s;
	}

	template<typename Ty>
	requires std::is_same_v<_ETy, Ty> || std::is_same_v<_UTy, Ty>
		static std::string from_flag_string(Ty & value, const std::string & text, const std::string & separator)
	{
		spliterator splitter(text, separator);
		value = static_cast<Ty>(0);
		std::string unknowns;
		std::string_view part;
		while (splitter.next(part))
		{
			auto iter = name_to_value.find(std::string(part));
			if (iter == name_to_value.end())
			{
				unknowns.append(part).append(separator);
			}
			else
			{
				value = value + static_cast<Ty>(iter->second);
			}
		}

		if (!unknowns.empty())
		{
			unknowns.erase(unknowns.size() - separator.size()); // remove last separator
		}

		return unknowns;
	}
};

template<class _ETy, _ETy _first_value = static_cast<_ETy>(0), _ETy _last_value = static_cast<_ETy>(0)>
requires std::is_enum_v<_ETy>
class enum_value
{
public:
	using _UTy = std::underlying_type_t<_ETy>;

protected:
	_UTy _value;

public:
	void clear()
	{
		_value = static_cast<_UTy>(0);
	}

	enum_value()
	{
		clear();
	}

	enum_value(_ETy value)
		: _value(static_cast<_UTy>(value))
	{
	}

	explicit enum_value(_UTy value)
		: _value(value)
	{
	}

	template<typename Ty>
	requires std::is_same_v<_ETy, Ty> || std::is_same_v<_UTy, Ty>
		bool has_flag(enum_value other) const
	{
		return (_value & other._value) == other._value;
	}

	bool has_flag(_UTy other) const
	{
		return enum_static<_ETy>::has_flag(_value, other);
	}

	bool has_flag(_ETy other) const
	{
		return enum_static<_ETy>::has_flag(other, static_cast<_UTy>(other));
	}

	enum_value& operator=(const enum_value& other)
	{
		_value = other._value;
		return *this;
	}

	bool operator==(const enum_value& other) const
	{
		return _value == other._value;
	}

	bool operator!=(const enum_value& other) const
	{
		return _value != other._value;
	}

	void flag_set(_ETy flag)
	{
		_value = enum_static<_ETy>::flag_set(_value, flag);
	}

	void flag_remove(_ETy flag)
	{
		_value = enum_static<_ETy>::flag_remove(_value, flag);
	}

	void flag_toggle(_ETy flag)
	{
		_value = enum_static<_ETy>::flag_toggle(_value, flag);
	}

	enum_value& operator++()
	{
		_value++;
		return *this;
	}

	enum_value& operator++(int)
	{
		auto temp = *this;
		*this++;
		return *temp;
	}

	enum_value& operator--()
	{
		_value--;
		return *this;
	}

	enum_value& operator--(int)
	{
		auto temp = *this;
		*this--;
		return *temp;
	}

	_ETy operator*() const
	{
		return static_cast<_ETy>(_value);
	}

	_ETy& operator*()
	{
		return *reinterpret_cast<_ETy*>(&_value);
	}

	_UTy& operator&()
	{
		return _value;
	}

	_UTy& data()
	{
		return _value;
	}

	_UTy data() const
	{
		return _value;
	}

	std::string to_string() const
	{
		return enum_static<_ETy>::to_string(_value);
	}

	bool from_string(const std::string& text)
	{
		return enum_static<_ETy>::from_string(_value, text);
	}

	std::string to_flag_string(const std::string& separator) const
	{
		return enum_static<_ETy>::to_flag_string(_value, separator);
	}

	std::string from_flag_string(const std::string& text, const std::string& separator)
	{
		return enum_static<_ETy>::from_flag_string(_value, text, separator);
	}
};
