#include "clang-c/Index.h"
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map>
#include "data.h"
#include "clang.h"
#include "util.h"
#include "parsing.h"

// default args: -indir .\..\enum_values\ -outfile ..\..\enum_data.cpp

using namespace enum_reader;

std::string create_out_file_text(std::vector<std::string> const& all_maps)
{
    std::string text;

    for (auto& m : all_maps)
    {
        text.append(m + "\r");
    }

    return text;
}

std::string get_named_arg_value(std::vector<std::string> const& args, std::string const& name)
{
    for (size_t i = 0; i < args.size(); i++)
    {
        if ((args[i] == name && ((i + 1 < args.size()))))
        {
            return args[i + 1];
        }
    }
    return std::string();
}

std::vector<std::string> get_named_arg_value_array(std::vector<std::string> const& args, std::string const& name)
{
    std::vector<std::string> values;
    for (size_t i = 0; i < args.size(); i++)
    {
        if ((args[i] == name && ((i + 1 < args.size()))))
        {
            for (auto value_i = i + 1; value_i < args.size(); value_i++)
            {
                if (args[value_i].starts_with("-"))
                {
                    return values;
                }
                values.push_back(args[value_i]);
            }
        }
    }
    return values;
}

std::string get_out_file(size_t include_position, std::string const& cpp_text)
{
    /*
    We do this instead of as part of clang parsing because the file may not
    exist. In that case, clang won't tell us the file.
    */
    auto beginQuote = cpp_text.find('\"', include_position);
    auto endQuote = cpp_text.find('\"', beginQuote + 1);
    auto out_file = cpp_text.substr(beginQuote + 1, endQuote - beginQuote - 1);
    return out_file;
}

int main(int argc, char** argv)
{
    std::vector<std::string> args(argv, argv + argc);

    auto code_file = get_named_arg_value(args, "-in");
    auto nspace = get_named_arg_value(args, "-nspace");
    auto exclude_enums = get_named_arg_value_array(args, "-exclude");

    std::cout << "[General] Reading file \"" << code_file << "\"\n";

    std::filesystem::directory_entry entry(code_file);

    if (entry.is_regular_file() && (entry.path().extension().string().ends_with(".cpp")))
    {
        auto cpp_text = read_file(entry.path().string());
        auto include_position = cpp_text.find("#include \"enum_values_");
        if (include_position != std::string::npos)
        {
            std::cout << "[General] Enums in namespace \"" << nspace << "\"\n" << std::endl;
            auto out_file = get_out_file(include_position, cpp_text);
            std::cout << "[Parse] Opening " << entry.path().string() << "... " << std::endl;
            auto file_enum_data = get_enums_in_file(entry.path().string(), nspace, exclude_enums);
            std::cout << "\t[Enum] Found " << file_enum_data.size() << std::endl;

            if (!file_enum_data.empty())
            {
                std::cout << "[Map] Creating map text... ";
                auto map_text = create_enums_maps(file_enum_data);
                std::cout << "done." << std::endl;

                auto out_path = entry.path().parent_path().string() + "/" + out_file;
                std::cout << "[Out] Creating out file (" << out_path << ")... ";
                auto out_file_text = create_out_file_text(map_text);
                write_file(out_path, out_file_text);
                std::cout << "done.\n" << std::endl;
            }
        }
        else
        {
            std::cout << "[General] File does not contain #include \"enum_values_...\"" << std::endl;
        }
    }
    else
    {
        std::cout << "[General] Cannot find file, not a file or not a cpp file" << std::endl;
    }

    std::cout << "[General] All done!" << std::endl;

    return 0;
}
