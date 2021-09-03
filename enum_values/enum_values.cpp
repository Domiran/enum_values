#include <unordered_map>
#include "enum_values.h"

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

const std::unordered_map<int, std::string> enum_static<theirvalues>::value_to_name = {
	{ 0, "zero" },
	{ 1, "one" },
	{ 2, "two" },
	{ 2, "also_two" },
	{ 3, "three" },
	{ 4, "four" },
	{ 5, "five" },
};
const std::unordered_map<std::string, int> enum_static<theirvalues>::name_to_value = {
	{ "zero", 0 },
	{ "one", 1 },
	{ "two", 2 },
	{ "also_two", 2 },
	{ "three", 3 },
	{ "four", 4 },
	{ "five", 5 },
};
