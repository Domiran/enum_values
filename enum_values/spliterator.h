#pragma once
class spliterator
{
private:
	std::string_view _source;
	std::string_view _split_by;
	size_t _source_size;
	size_t _split_size;
	size_t _next;

public:
	spliterator(const std::string_view& source, const std::string_view& split_by);
	bool next(std::string_view& out);
	bool next();
};
