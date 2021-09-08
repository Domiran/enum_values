#include <string>
#include <string_view>
#include <vector>
#include "util.h"
#include "data.h"

namespace enum_reader
{
    enum_value_data::enum_value_data(std::string const& smbl, std::string const& nm, std::string const& d)
    {
        symbol = smbl;
        name = (nm.empty() ? symbol : nm);
        desc = d;
        trim(name);
        trim(desc);
    }

    void enum_data::add_value(std::string const& smbl, std::string const& nm, std::string const& d)
    {
        values.emplace_back(smbl, nm, d);
    }

    std::string enum_data::get_name() const
    {
        std::string all;
        if (!prefix.empty())
        {
            all.append(prefix);
        }
        all.append(name);
        return all;
    }

    size_t enum_data::size() const
    {
        return values.size();
    }

    std::string enum_data::get_value_name(size_t i) const
    {
        return values.at(i).name;
    }

    std::string enum_data::get_value_symbol(size_t i) const
    {
        return prefix + name + "::" + values.at(i).symbol;
    }
}
