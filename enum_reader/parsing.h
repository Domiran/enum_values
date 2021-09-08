#pragma once
namespace enum_reader
{
    using value_map = std::unordered_map<std::string, int>;

    int parse_enum_expression(value_map const& values, std::string expression);
    bool is_valid_enum_value(std::string_view const& line);
    void parse_enum_header(enum_data& data, std::string const& line);
    //void parse_enum_value(enum_data& data, value_map& values, int& next_value, const std::string& line);
    enum_data parse_enum(std::vector<std::string_view> const& file_lines, size_t& current);
    const std::string enum_data_pairs(enum_data const& data);
    std::string enum_data_format(enum_data const& data);
    std::string create_enum_map_data(enum_data const& data);
    //std::vector<enum_data> get_file_enum_data(std::string const& cpp_text);
    std::vector<std::string> create_enums_maps(std::vector<enum_data> const& enums);
};
