#pragma once
namespace enum_reader
{
    using value_map = std::unordered_map<std::string, int>;

    std::vector<std::string> create_enums_maps(std::vector<enum_data> const& enums);
};
