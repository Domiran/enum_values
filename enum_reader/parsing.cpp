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
    const std::string enum_header_name_text = R"(enum class ([A-z]+))";
    const std::string enum_header_prefix_text = R"(prefix:([A-z:]+))";

    const std::string enum_value_name_text = R"(^[\s]*([A-z_]+))";
    const std::string enum_value_expr_text = R"(=([0-9A-z\-| ]+))";
    const std::string enum_value_comment_text = R"r(\/\/[\s]*("\([A-z0-9_ ]+\)"+)?(.+))r";

    const auto regex_options = std::regex_constants::syntax_option_type::ECMAScript | std::regex_constants::syntax_option_type::optimize;

    std::regex enum_header_name_reg(enum_header_name_text, regex_options);
    std::regex enum_header_prefix_reg(enum_header_prefix_text, regex_options);

    std::regex enum_values_name_reg(enum_value_name_text, regex_options);
    std::regex enum_values_expr_reg(enum_value_expr_text, regex_options);
    std::regex enum_values_comment_reg(enum_value_comment_text, regex_options);

    int parse_enum_expression(value_map const& values, std::string expression)
    {
        trim(expression);
        auto iter = values.find(expression);

        auto is_digit = [](char c) { return c >= '0' && c <= '9'; };

        if (iter != values.end())
        {
            return iter->second;
        }
        else if (std::all_of(expression.begin(), expression.end(), is_digit))
        {
            return std::stoi(expression); // for now, we aren't parsing math expressions
        }
        else
        {
            return -1;
        }
    }

    bool is_valid_enum_value(std::string_view const& line)
    {
        // trimmed, it must not be an empty line
        // its first character cannot be / (really checking for //, we're ignoring multiline comments as it's too hard)

        if (line.find("{") != std::string::npos)
            return false;

        auto trimmed = trim_size(line);
        if (trimmed == 0)
            return false;

        auto first_char = line.find_first_not_of(whitespace);
        if (first_char != std::string::npos && (line[first_char] == '/'))
            return false;

        return true;
    }

    void parse_enum_header(enum_data& data, std::string const& line)
    {
        auto name_iter = std::sregex_iterator(line.begin(), line.end(), enum_header_name_reg);
        auto prefix_iter = std::sregex_iterator(line.begin(), line.end(), enum_header_prefix_reg);
        auto end_iter = std::sregex_iterator();

        data.name = (*name_iter)[1].str();
        data.prefix = (prefix_iter != end_iter ? (*prefix_iter)[1].str() : std::string()); // parsing class would be a huge pain so we just look for //class@

        // we need class and namespace separately cuz we need to put the def. in a namespace and the class as part of the enum type name when we use it
    }

    /*void parse_enum_value(enum_data& data, value_map& values, int& next_value, const std::string& line)
    {
        int enum_value;
        auto name_iter = std::sregex_iterator(line.begin(), line.end(), enum_values_name_reg);
        auto end_iter = std::sregex_iterator();

        // if we can't get a name, exit
        if (name_iter == end_iter)
            return;

        auto name = (*name_iter)[1].str();
        auto expr_iter = std::sregex_iterator(line.begin(), line.end(), enum_values_expr_reg);
        auto comment_iter = std::sregex_iterator(line.begin(), line.end(), enum_values_comment_reg);

        auto expr = (expr_iter != end_iter ? (*expr_iter)[1].str() : std::string());
        auto alt_name = (comment_iter != end_iter ? (*comment_iter)[1].str() : std::string());
        auto comment = (comment_iter != end_iter ? (*comment_iter)[2].str() : std::string());

        if (!alt_name.empty())
        {
            trim(alt_name);
            name = alt_name.substr(2, alt_name.size() - 4);
        }

        if (expr_iter == end_iter)
        {
            enum_value = next_value;
            next_value++;
        }
        else
        {
            enum_value = parse_enum_expression(values, expr);
        }

        values[name] = enum_value;
        data.add_value(name, enum_value, comment);
    }*/

    /*const std::string enum_class_text = "enum class ";
    enum_data parse_enum(std::vector<std::string_view> const& file_lines, size_t& current)
    {
        enum_data data;
        std::string_view line = file_lines[current];

        parse_enum_header(data, std::string(line));

        // go until we reach "};"
        current++;
        line = file_lines[current];
        int next_value = 0;
        value_map found_values;
        while (line.find("};") == std::string::npos)
        {
            parse_enum_value(data, found_values, next_value, std::string(line));
            current++;
            if (current == file_lines.size())
                break;
            else
                line = file_lines[current];
        }

        return data;
    }*/

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

    std::string create_enum_map_data(enum_data const& data)
    {
        /*
        enum class myvalues : int;
        template<>
        class enum_data<myvalues>
        {
        public:
            static constexpr auto get_data()
            {
                constexpr std::array<data_pair<myvalues>, 7> d = { {
                { 0, std::string_view("zero") },
                { 1, std::string_view("one") },
                { 2, std::string_view("two") },
                { 3, std::string_view("three") },
                { 3, std::string_view("also_three") },
                { 4, std::string_view("four") },
                { 5, std::string_view("five") },
                } };
                return d;
            }

            static constexpr auto get_desc()
            {
                return std::string("these values are nice");
            }
        };
        */

        return enum_data_format(data);
    }

    /*std::vector<enum_data> get_file_enum_data(std::string const& cpp_text)
    {
        std::vector<std::string_view> lines;
        spliterator splitter(cpp_text, "\n");
        std::vector<enum_data> enums;

        std::string_view v;
        while (splitter.next(v))
        {
            lines.push_back(v);
        }

        for (size_t i = 0u; i < lines.size(); i++)
        {
            auto n = lines[i].find("enum class ");
            if ((n != std::string::npos) && (lines[i].find(";") == std::string::npos))
            {
                auto comment_text = (i > 0 ? lines[i - 1] : std::string());
                auto comment_iter = comment_text.find("//");
                auto data = parse_enum(lines, i);
                if (comment_iter != std::string::npos)
                {
                    data.comment = comment_text.substr(comment_iter + 2, comment_text.size() - comment_iter - 2 - 1);
                    trim(data.comment);
                }
                enums.push_back(data);
            }
        }

        return enums;
    }*/

    std::vector<std::string> create_enums_maps(std::vector<enum_data> const& enums)
    {
        std::vector<std::string> all_map_text;

        for (auto& e : enums)
        {
            all_map_text.push_back(create_enum_map_data(e));
        }

        return all_map_text;
    }
}
