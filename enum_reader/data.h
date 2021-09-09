#pragma once
namespace enum_reader
{
    struct enum_value_data
    {
        std::string symbol;
        std::string name;
        std::string desc;

        enum_value_data(std::string const& smbl, std::string const& nm, std::string const& desc);
    };

    struct enum_data
    {
        std::string name;
        std::string desc;
        std::string prefix; // full prefix of the name when used, including namespace and any classes
        std::vector<enum_value_data> values;

        void add_value(std::string const& smbl, std::string const& nm, std::string const& cmt);
        std::string get_name() const;
        size_t size() const;
        std::string get_value_symbol(size_t i) const;
        std::string get_value_name(size_t i) const;
        bool empty() const;
    };
}
