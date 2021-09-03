#include <vector>
#include <string>
#include <filesystem>
#include <regex>
#include <unordered_map>

using namespace std;

const string enum_header_name_text = R"(enum class ([A-z]+))";
const string enum_header_type_text = R"((:|: )([A-z]+))"; // 2nd group is what we take
const string enum_header_nspace_text = R"(\/\/ns@([A-z]+))";

const string enum_value_name_text = R"(^\s([A-z_]+))";
const string enum_value_expr_text = R"(=([0-9A-z\-| ]+))";
const string enum_value_comment_text = R"(\/\/([ A-z0-9!?]+))";

using value_map = unordered_map<string, int>;

regex enum_header_name_reg(enum_header_name_text, regex_constants::syntax_option_type::ECMAScript | regex_constants::syntax_option_type::optimize);
regex enum_header_type_reg(enum_header_type_text, regex_constants::syntax_option_type::ECMAScript | regex_constants::syntax_option_type::optimize);
regex enum_header_nspace_reg(enum_header_nspace_text, regex_constants::syntax_option_type::ECMAScript | regex_constants::syntax_option_type::optimize);

regex enum_values_name_reg(enum_value_name_text, regex_constants::syntax_option_type::ECMAScript | regex_constants::syntax_option_type::optimize);
regex enum_values_expr_reg(enum_value_expr_text, regex_constants::syntax_option_type::ECMAScript | regex_constants::syntax_option_type::optimize);
regex enum_values_comment_reg(enum_value_comment_text, regex_constants::syntax_option_type::ECMAScript | regex_constants::syntax_option_type::optimize);

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

spliterator::spliterator(const std::string_view& source, const std::string_view& split_by)
    : _source(source), _split_by(split_by), _source_size(source.size()), _split_size(split_by.size()), _next(0u)
{
}

bool spliterator::next(std::string_view& out)
{
    if (_next >= _source_size)
        return false;

    auto find = _source.find(_split_by, _next);
    if (find != std::string::npos)
    {
        out = _source.substr(_next, find - _next);
        _next = find + _split_size;
    }
    else
    {
        out = _source.substr(_next);
        _next = _source_size;
    }
    return true;
}

bool spliterator::next()
{
    if (_next >= _source_size)
        return false;

    auto find = _source.find(_split_by, _next);
    if (find != std::string::npos)
    {
        _next = find + _split_size;
    }
    else
    {
        _next = _source_size;
    }
    return true;
}

const string whitespace = " \r\t\n\f\v";
void ltrim(std::string& s)
{
    s.erase(0, s.find_first_not_of(whitespace));
}

void rtrim(std::string& s)
{
    s.erase(s.find_last_not_of(whitespace) + 1);
}

void trim(std::string& s)
{
    rtrim(s);
    ltrim(s);
}

int trim_size(string_view const& s)
{
    auto b = s.find_first_not_of(whitespace);
    if (b == string::npos) // if one failed, both will
    {
        return 0;
    }
    auto e = s.find_last_not_of(whitespace);
    return e - b + 1;
}

struct enum_value_data
{
    string name;
    int value;
    string comment;

    enum_value_data(string_view const& nm, int v, string_view const& cmt)
    {
        name = nm;
        value = v;
        comment = cmt;
        trim(name);
        trim(comment);
    }
};

struct enum_data
{
    string name;
    string comment;
    string type;
    string nspace;
    vector<enum_value_data> values;

    void add_value(string_view const& nm, int v, string_view const& cmt)
    {
        values.emplace_back(nm, v, cmt);
    }

    string full_name() const
    {
        if (nspace.empty())
        {
            return name;
        }
        else
        {
            return nspace + ":" + name;
        }
    }
};

string read_file(string const& path)
{
    filesystem::directory_entry entry(path);
    size_t file_size = static_cast<size_t>(entry.file_size());
    auto file_data = unique_ptr<char>(new char[file_size + 1]);
    FILE* f;
    auto err = fopen_s(&f, path.c_str(), "rb");
    if (err)
    {
        cerr << "Reading file \"" + path + "\" got error " + to_string(err) + ".";
        return string();
    }
    auto read_size = fread(file_data.get(), 1, file_size, f);
    file_data.get()[read_size] = 0;
    fclose(f);
    return string(file_data.get());
}

bool write_file(string const& path, string const& text)
{
    FILE* f;
    auto err = fopen_s(&f, path.c_str(), "wb");
    if (err)
    {
        cerr << "Writing file \"" + path + "\" got error " + to_string(err) + ".";
        return false;
    }
    auto written = fwrite(text.c_str(), 1, text.size(), f);
    fclose(f);

    if (written != text.size())
    {
        cerr << "Writing lib file, wrote " + to_string(written) + " bytes but have " + to_string(text.size()) + " bytes to write.";
    }

    return written = text.size();
}

int parse_enum_expression(value_map const& values, string expression)
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

bool is_valid_enum_value(string_view const& line)
{
    // trimmed, it must not be an empty line
    // its first character cannot be / (really checking for //, we're ignoring multiline comments as it's too hard)

    if (line.find("{") != string::npos)
        return false;

    auto trimmed = trim_size(line);
    if (trimmed == 0)
        return false;

    auto first_char = line.find_first_not_of(whitespace);
    if (first_char != string::npos && (line[first_char] == '/'))
        return false;

    return true;
}

void parse_enum_header(enum_data& data, string const& line)
{
    auto name_iter = sregex_iterator(line.begin(), line.end(), enum_header_name_reg);
    auto type_iter = sregex_iterator(line.begin(), line.end(), enum_header_type_reg);
    auto nspace_iter = sregex_iterator(line.begin(), line.end(), enum_header_nspace_reg);
    auto end_iter = sregex_iterator();

    data.name = (*name_iter)[1].str();
    data.type = (type_iter != end_iter ? (*type_iter)[2].str() : string("int")); // int is enum default by C++ spec
    data.nspace = (nspace_iter != end_iter ? (*nspace_iter)[1].str() : string()); // parsing namespace would be a huge pain so we just look for //ns@
}

void parse_enum_value(enum_data& data, value_map& values, int& next_value, const string& line)
{
    int enum_value;
    auto name_iter = sregex_iterator(line.begin(), line.end(), enum_values_name_reg);
    auto end_iter = sregex_iterator();

    // if we can't get a name, exit
    if (name_iter == end_iter)
        return;

    auto name = (*name_iter)[1].str();
    auto expr_iter = sregex_iterator(line.begin(), line.end(), enum_values_expr_reg);
    auto comment_iter = sregex_iterator(line.begin(), line.end(), enum_values_comment_reg);

    auto expr = (expr_iter != end_iter ? (*expr_iter)[1].str() : string());
    auto comment = (comment_iter != end_iter ? (*comment_iter)[1].str() : string());

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
}

const string enum_class_text = "enum class ";
enum_data parse_enum(vector<string_view> const& file_lines, size_t& current)
{
    enum_data data;
    string_view line = file_lines[current];

    parse_enum_header(data, string(line));

    // go until we reach "};"
    current++;
    line = file_lines[current];
    int next_value = 0;
    value_map found_values;
    while (line.find("};") == string::npos)
    {
        parse_enum_value(data, found_values, next_value, string(line));
        current++;
        if (current == file_lines.size())
            break;
        else
            line = file_lines[current];
    }

    return data;
}

string create_enum_map_data(enum_data const& data)
{
    // std::unordered_map<int, std::string> enum_flags<myvalues>::value_to_name = { {0, "zero"}, {1, "one"}, {2, "two"}, {3, "three"}, {4, "four"}, {5, "five"} };
    // and the reverse
    string map_text;

    map_text.append("std::unordered_map<" + data.type + ", std::string> enum_flags<" + data.name + ">::value_to_name = {\r");

    for (auto& enum_value : data.values)
    {
        map_text.append("\t{ " + to_string(enum_value.value) + ", \"" + enum_value.name + "\" }, \r");
    }
    map_text.append(" };\r");
    map_text.append("std::unordered_map<std::string, " + data.type + "> enum_flags<" + data.name + ">::name_to_value = {\r");

    for (auto& enum_value : data.values)
    {
        map_text.append("\t{ \"" + enum_value.name + "\", " + to_string(enum_value.value) + " }, \r");
    }
    map_text.append(" };\r");

    return map_text;
}

vector<enum_data> get_file_enum_data(string const& cpp_text)
{
    vector<string_view> lines;
    spliterator splitter(cpp_text, "\n");
    std::vector<enum_data> enums;

    string_view v;
    while (splitter.next(v))
    {
        lines.push_back(v);
    }

    for (size_t i = 0u; i < lines.size(); i++)
    {
        auto n = lines[i].find("enum class ");
        if (n != string::npos)
        {
            auto comment_text = lines[i - 1];
            auto comment_iter = comment_text.find("//");
            auto data = parse_enum(lines, i);
            if (comment_iter != string::npos)
            {
                data.comment = comment_text.substr(comment_iter + 2, comment_text.size() - comment_iter - 2 - 1);
                trim(data.comment);
            }
            enums.push_back(data);
        }
    }

    return enums;
}

vector<string> create_enums_maps(vector<enum_data> const& enums)
{
    vector<string> all_map_text;

    for (auto& e : enums)
    {
        all_map_text.push_back(create_enum_map_data(e));
    }

    return all_map_text;
}

string create_cpp_file_text(vector<string> const& all_maps)
{
    string text;

    text.append("#include \"enum_flags.h\"\r\r");

    for (auto& m : all_maps)
    {
        text.append(m + "\r");
    }

    return text;
}

string get_named_arg_value(vector<string> const& args, string const& name)
{
    for (size_t i = 0; i < args.size(); i++)
    {
        if ((args[i] == name && ((i + 1 < args.size()))))
        {
            return args[i + 1];
        }
    }
    return string();
}

int main(int argc, char** argv)
{
    vector<string> args(argv, argv + argc);

    auto code_path = get_named_arg_value(args, "-indir");
    auto out_file = get_named_arg_value(args, "-outfile");

    vector<enum_data> all_enums;
    vector<string> all_enum_maps;

    for (auto& entry : std::filesystem::directory_iterator(code_path))
    {
        if (entry.is_regular_file() &&
            (entry.path().extension().string().ends_with(".cpp")) || entry.path().extension().string().ends_with(".h"))
        {
            cout << "[Source] Opening " << entry.path().string() << "." << endl;
            auto cpp_text = read_file(entry.path().string());

            cout << "[Enum] Reading enums, ";
            auto file_enum_data = get_file_enum_data(cpp_text);
            cout << " found " << file_enum_data.size() << "." << endl;

            cout << "[Map] Creating map text";
            auto map_text = create_enums_maps(file_enum_data);
            all_enum_maps.insert(all_enum_maps.end(), map_text.begin(), map_text.end());
            cout << ".\n" << endl;
        }
    }

    cout << "[Cpp] Creating cpp file";
    auto cpp_file_text = create_cpp_file_text(all_enum_maps);
    cout << ".\n" << endl;

    write_file(out_file, cpp_file_text);

    return 0;
}
