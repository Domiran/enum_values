#pragma once
namespace enum_reader
{
	std::vector<enum_data> get_enums_in_file(std::string const& file, std::string const& in_nspace, std::vector<std::string> const& exclude_enums);
}
