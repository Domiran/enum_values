#pragma once
#include <unordered_map>
#include <array>

#define SPLIT_FUNC

#ifdef SPLIT_FUNC
#include "spliterator.h"
#endif

template<typename ETy, typename UTy = std::underlying_type_t<ETy>>
struct data_pair
{
	ETy value;
	std::string_view name;

	UTy as_number()
	{
		return static_cast<UTy>(value);
	}

	std::string as_string()
	{
		return std::string(name);
	}
};

template<typename T> class enum_data;

template<class ETy>
concept has_enum_data =
requires()
{
	enum_data<ETy>::get_data();
	enum_data<ETy>::get_desc();
};

template<class ETy>
requires std::is_enum_v<ETy> && has_enum_data<ETy>
class enum_static
{
public:
	using UTy = std::underlying_type_t<ETy>; // underlying type
	using data_pair_t = data_pair<ETy>;

private:
	static constexpr auto desc = enum_data<ETy>::get_desc();
	static constexpr auto data = enum_data<ETy>::get_data();

public:
	static constexpr auto iterable()
	{
		return data;
	}

	static constexpr std::string description()
	{
		return desc;
	}

	static constexpr ETy flag_set(ETy value, ETy flag)
	{
		return static_cast<ETy>(static_cast<UTy>(value) | static_cast<UTy>(flag));
	}

	static constexpr ETy flag_remove(ETy value, ETy flag)
	{
		return static_cast<ETy>(static_cast<UTy>(value) & ~static_cast<UTy>(flag));
	}

	static constexpr ETy flag_toggle(ETy value, ETy flag)
	{
		return static_cast<ETy>(static_cast<UTy>(value) ^ static_cast<UTy>(flag));
	}

	template<typename Ty>
	requires std::is_same_v<ETy, Ty> || std::is_same_v<UTy, Ty>
		static constexpr bool has_flag(Ty value, UTy other)
	{
		return (static_cast<UTy>(value) & other) == other;
	}

	template<typename Ty>
	requires std::is_same_v<ETy, Ty> || std::is_same_v<UTy, Ty>
		static constexpr bool has_flag(Ty value, ETy other)
	{
		return has_flag(static_cast<UTy>(value), static_cast<UTy>(other));
	}

	template<typename Ty>
	requires std::is_same_v<ETy, Ty> || std::is_same_v<UTy, Ty>
		static constexpr std::string to_string(Ty value)
	{
		auto fn = [&value](const auto& pair) { return static_cast<UTy>(pair.value) == static_cast<UTy>(value); };
		auto iter = std::find_if(data.begin(), data.end(), fn);
		if (iter == data.end())
		{
			return std::string();
		}
		return std::string(iter->name);
	}

	template<typename Ty>
	requires std::is_same_v<ETy, Ty> || std::is_same_v<UTy, Ty>
		static constexpr bool from_string(Ty & value, const std::string & text)
	{
		auto iter = std::find_if(data.begin(), data.end(), [&text](const auto& pair) { return pair.name == text; });
		if (iter == data.end())
		{
			return false;
		}
		value = static_cast<UTy>(iter->value);
		return true;
	}

	template<typename Ty>
	requires std::is_same_v<ETy, Ty> || std::is_same_v<UTy, Ty>
		static constexpr std::string to_flag_string(Ty value, const std::string & separator)
	{
		std::string s;
		for (auto& p : data)
		{
			if (has_flag(value, p.value))
			{
				s.append(std::string(p.name)).append(separator);
			}
		}

		if (!s.empty())
		{
			s.erase(s.size() - separator.size()); // remove last separator
		}

		return s;
	}

#ifdef SPLIT_FUNC
	template<typename Ty>
	requires std::is_same_v<ETy, Ty> || std::is_same_v<UTy, Ty>
		static constexpr std::string from_flag_string(Ty & value, const std::string & text, const std::string & separator)
	{
		spliterator splitter(text, separator);
		value = static_cast<Ty>(0);
		std::string unknowns;
		std::string_view part;
		while (splitter.next(part))
		{
			auto iter = std::find_if(data.begin(), data.end(), [&part](const auto& pair) { return pair.name == part; });
			if (iter == data.end())
			{
				unknowns.append(part).append(separator);
			}
			else
			{
				value = value + static_cast<UTy>(iter->value);
			}
		}

		if (!unknowns.empty())
		{
			unknowns.erase(unknowns.size() - separator.size()); // remove last separator
		}

		return unknowns;
	}
#endif
};

template<class ETy, ETy _first_value = static_cast<ETy>(0), ETy _last_value = static_cast<ETy>(0)>
requires std::is_enum_v<ETy>
class enum_value
{
public:
	using UTy = std::underlying_type_t<ETy>;

protected:
	UTy _value;

public:
	void clear()
	{
		_value = static_cast<UTy>(0);
	}

	enum_value()
	{
		clear();
	}

	enum_value(ETy value)
		: _value(static_cast<UTy>(value))
	{
	}

	explicit enum_value(UTy value)
		: _value(value)
	{
	}

	template<typename Ty>
	requires std::is_same_v<ETy, Ty> || std::is_same_v<UTy, Ty>
		bool has_flag(enum_value other) const
	{
		return (_value & other._value) == other._value;
	}

	bool has_flag(UTy other) const
	{
		return enum_static<ETy>::has_flag(_value, other);
	}

	bool has_flag(ETy other) const
	{
		return enum_static<ETy>::has_flag(other, static_cast<UTy>(other));
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

	void flag_set(ETy flag)
	{
		_value = enum_static<ETy>::flag_set(_value, flag);
	}

	void flag_remove(ETy flag)
	{
		_value = enum_static<ETy>::flag_remove(_value, flag);
	}

	void flag_toggle(ETy flag)
	{
		_value = enum_static<ETy>::flag_toggle(_value, flag);
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

	ETy operator*() const
	{
		return static_cast<ETy>(_value);
	}

	ETy& operator*()
	{
		return *reinterpret_cast<ETy*>(&_value);
	}

	UTy& operator&()
	{
		return _value;
	}

	UTy& data()
	{
		return _value;
	}

	UTy data() const
	{
		return _value;
	}

	std::string to_string() const
	{
		return enum_static<ETy>::to_string(_value);
	}

	bool from_string(const std::string& text)
	{
		return enum_static<ETy>::from_string(_value, text);
	}

	std::string to_flag_string(const std::string& separator) const
	{
		return enum_static<ETy>::to_flag_string(_value, separator);
	}

#ifdef SPLIT_FUNC
	std::string from_flag_string(const std::string& text, const std::string& separator)
	{
		return enum_static<ETy>::from_flag_string(_value, text, separator);
	}
#endif
};
