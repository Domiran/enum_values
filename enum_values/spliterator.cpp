#include <string>
#include <string_view>
#include "spliterator.h"

spliterator::spliterator(const std::string_view& source, const std::string_view& split_by)
	: _source(source), _split_by(split_by), _source_size(source.size()), _split_size(split_by.size()), _next(0u)
{
}

bool spliterator::next(std::string_view& out)
{
	if (_next >= _source_size)
		return false;

	auto find = _source.find(_split_by, _next);
	if (find != std::string::npos)
	{
		out = _source.substr(_next, find - _next);
		_next = find + _split_size;
	}
	else
	{
		out = _source.substr(_next);
		_next = _source_size;
	}
	return true;
}

bool spliterator::next()
{
	if (_next >= _source_size)
		return false;

	auto find = _source.find(_split_by, _next);
	if (find != std::string::npos)
	{
		_next = find + _split_size;
	}
	else
	{
		_next = _source_size;
	}
	return true;
}
