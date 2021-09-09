#include <string>
#include <string_view>
#include <unordered_map>
#include <regex>
#include "data.h"
#include "parsing.h"
#include "util.h"
#include "data.h"

namespace enum_reader
{
    const std::string enum_value_comment_text = R"r(\/\/[\s]*("\([A-z0-9_ ]+\)"+)?(.+))r";
    const auto regex_options = std::regex_constants::syntax_option_type::ECMAScript | std::regex_constants::syntax_option_type::optimize;
    std::regex enum_values_comment_reg(enum_value_comment_text, regex_options);

    const std::string enum_data_pairs(enum_data const& data)
    {
        std::string pairs;

        for (auto i = 0u; i < data.size(); i++)
        {
            pairs.append("\t\t\t{ ")
                .append(data.get_value_symbol(i))
                .append(", std::string_view(\"")
                .append(data.get_value_name(i))
                .append("\") },\r");
        }

        return pairs;
    }

    std::string enum_data_format(enum_data const& data)
    {
        std::string fmt = R"(template<>
class enum_data<%enum_name%>
{
public:
	static constexpr auto get_data()
	{
		constexpr std::array<data_pair<%enum_name%>, %enum_count%> d = { {
%enum_pairs%
		} };
		return d;
	}

	static constexpr auto get_desc()
	{
		return std::string("%enum_desc%");
	}
};
)";

        replace(fmt, "%enum_name%", data.get_name());
        replace(fmt, "%enum_desc%", data.desc);
        replace(fmt, "%enum_count%", std::to_string(data.values.size()));
        replace(fmt, "%enum_pairs%", enum_data_pairs(data));

        return fmt;
    }

    std::vector<std::string> create_enums_maps(std::vector<enum_data> const& enums)
    {
        std::vector<std::string> all_map_text;

        for (auto& e : enums)
        {
            if (!e.empty())
            {
                all_map_text.push_back(enum_data_format(e));
            }
        }

        return all_map_text;
    }
}
