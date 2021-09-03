#include <unordered_map>
#include "enum_values.h"

enum class myvalues : int;
const std::string enum_static<myvalues>::enum_desc = "these values are used by those values, top kek!";
const std::unordered_map<int, std::string> enum_static<myvalues>::value_to_name = {
	{ 0, "zero" },
	{ 1, "one" },
	{ 2, "two" },
	{ 3, "three" },
	{ 3, "also_three" },
	{ 4, "four" },
	{ 5, "five" },
};
const std::unordered_map<std::string, int> enum_static<myvalues>::name_to_value = {
	{ "zero", 0 },
	{ "one", 1 },
	{ "two", 2 },
	{ "three", 3 },
	{ "also_three", 3 },
	{ "four", 4 },
	{ "five", 5 },
};

enum class yourvalues : int;
const std::string enum_static<yourvalues>::enum_desc = "";
const std::unordered_map<int, std::string> enum_static<yourvalues>::value_to_name = {
	{ 0, "zero" },
	{ 1, "one" },
	{ 2, "two" },
	{ 2, "also_two" },
	{ 3, "three" },
	{ 4, "four" },
	{ 5, "five" },
};
const std::unordered_map<std::string, int> enum_static<yourvalues>::name_to_value = {
	{ "zero", 0 },
	{ "one", 1 },
	{ "two", 2 },
	{ "also_two", 2 },
	{ "three", 3 },
	{ "four", 4 },
	{ "five", 5 },
};

namespace espace
{
	enum class theirvalues : int;
}
const std::string enum_static<espace::theirvalues>::enum_desc = "";
const std::unordered_map<int, std::string> enum_static<espace::theirvalues>::value_to_name = {
	{ 0, "zero" },
	{ 1, "one" },
	{ 2, "two" },
	{ 2, "also_two" },
	{ 3, "three" },
	{ 4, "four" },
	{ 5, "five" },
};
const std::unordered_map<std::string, int> enum_static<espace::theirvalues>::name_to_value = {
	{ "zero", 0 },
	{ "one", 1 },
	{ "two", 2 },
	{ "also_two", 2 },
	{ "three", 3 },
	{ "four", 4 },
	{ "five", 5 },
};

