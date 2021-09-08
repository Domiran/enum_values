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

int main(int argc, char** argv)
{
    std::vector<std::string> args(argv, argv + argc);

    auto code_path = get_named_arg_value(args, "-indir");
    auto out_file = get_named_arg_value(args, "-outfile");
    auto nspace = get_named_arg_value(args, "-nspace");

    std::vector<enum_reader::enum_data> all_enums;
    std::vector<std::string> all_enum_maps;

    std::string all_files;

    for (auto& entry : std::filesystem::directory_iterator(code_path))
    {
        /*if (entry.is_regular_file() && (!entry.path().string().ends_with("enum_data.h")) && entry.path().extension().string().ends_with(".cpp"))
        {
            all_files.append(entry.path().string()).append(" ");
        }*/

        if (entry.is_regular_file() && (!entry.path().string().ends_with("enum_data.h")) &&
            (entry.path().extension().string().ends_with(".cpp") || entry.path().extension().string().ends_with(".h")))
        {
            std::cout << "[Source] Opening " << entry.path().string() << "." << std::endl;
            auto cpp_text = read_file(entry.path().string());

            auto pos = cpp_text.find("enum class ");
            if (pos != std::string::npos)
            {
                auto file_enum_data = get_enums_in_file(entry.path().string(), nspace);



                /*std::cout << "[Enum] Reading enums, ";
                auto file_enum_data = get_file_enum_data(cpp_text);
                std::cout << " found " << file_enum_data.size() << "." << std::endl;
                */
                std::cout << "[Map] Creating map text";
                auto map_text = create_enums_maps(file_enum_data);
                all_enum_maps.insert(all_enum_maps.end(), map_text.begin(), map_text.end());
                std::cout << ".\n" << std::endl;
            }
        }
    }

    std::cout << "[Out] Creating out file";
    auto out_file_text = create_out_file_text(all_enum_maps);
    std::cout << ".\n" << std::endl;

    write_file(out_file, out_file_text);

    return 0;
}
