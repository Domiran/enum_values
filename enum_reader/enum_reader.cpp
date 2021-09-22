#include "clang-c/Index.h"
#include <set>
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

// -in .\..\enum_values\tester.cpp -file-exts .cpp -nspace tester_ns -rebuild-data-folder .\..\enum_values\x64\Debug -use-rebuild-check -exclude-enums "Chain::VertexArrayObjectBase<T>::DrawMode" -verbose

// -in C:\Stuff\C++\CHengine\Hauntlet\Source -file-exts .cpp .h  -nspace Chain -exclude-enums "Chain::VertexArrayObjectBase<T>::DrawMode" -verbose

using namespace enum_reader;

bool verbose_messages;

std::string create_out_file_text(std::vector<std::string> const& all_maps)
{
    std::string text;

    text
        .append("#pragma once\r\n")
        .append("#include \"enum_values.h\"\r\n\r\n");

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

bool get_named_arg_exists(std::vector<std::string> const& args, std::string const& name)
{
    for (size_t i = 0; i < args.size(); i++)
    {
        if (args[i] == name)
        {
            return true;
        }
    }
    return false;
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
                values.push_back(remove_quotes(args[value_i]));
            }
        }
    }
    return values;
}

std::set<std::string> get_named_arg_value_set(std::vector<std::string> const& args, std::string const& name)
{
    std::set<std::string> values;
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
                values.insert(remove_quotes(args[value_i]));
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

void process_file(std::filesystem::path const& entry, std::string const& nspace, std::vector<std::string> const& exclude_enums)
{
    auto file = entry.string();
    auto cpp_text = read_file(file);
    auto include_position = cpp_text.find("#include \"enum_values_");
    if (include_position != std::string::npos)
    {
        auto out_file = get_out_file(include_position, cpp_text);
        if (verbose_messages) std::cout << "[Process] Reading \"" << file << "\"" << std::endl;
        auto file_enum_data = get_enums_in_file(file, nspace, exclude_enums);
        if (verbose_messages) std::cout << "\t[Process] Found " << file_enum_data.size() << std::endl;

        if (!file_enum_data.empty())
        {
            if (verbose_messages) std::cout << "[Process] Creating map text... ";
            auto map_text = create_enums_maps(file_enum_data);
            if (verbose_messages) std::cout << "done." << std::endl;

            auto out_path = entry.parent_path().string() + "/" + out_file;
            if (verbose_messages) std::cout << "[Process] Creating out file (" << out_path << ")... ";
            auto out_file_text = create_out_file_text(map_text);
            write_file(out_path, out_file_text);
            if (verbose_messages) std::cout << "done.\n" << std::endl;
            if (!verbose_messages) std::cout << "[Process] Created/updated \"" << out_path << "\"" << std::endl;
        }
    }
    else
    {
        if (verbose_messages) std::cout << "[Process] \"" << file << "\"" << " does not contain [#include \"enum_values_...\"]" << std::endl;
    }
}

bool does_need_rebuild(std::filesystem::directory_entry const& cpp_path, std::string rebuild_data_folder, std::string const& rebuild_data_ext)
{
    if (!rebuild_data_folder.ends_with("\\") && !rebuild_data_folder.ends_with("/"))
    {
        rebuild_data_folder.append("/");
    }

    // hardcoded for now
    std::filesystem::directory_entry obj_entry{ rebuild_data_folder + cpp_path.path().stem().string() + rebuild_data_ext };
    if (!obj_entry.exists())
        return true;
    auto last_build_time = std::filesystem::last_write_time(obj_entry);
    auto source_file_time = std::filesystem::last_write_time(cpp_path);

    return source_file_time > last_build_time;
}

template<typename T>
concept can_iterate =
requires(T& t)
{
    t.begin();
    t.end();
};

template<typename T>
concept iterable_list = (can_iterate<T> && !std::is_same_v<T, std::string>);

template<iterable_list T>
std::ostream& operator<<(std::ostream& o, T const& list)
{
    for (auto& s : list)
    {
        o << "\"" << s << "\" ";
    }
    return o;
}

int main(int argc, char** argv)
{
    std::vector<std::string> args(argv, argv + argc);

    auto code_file = get_named_arg_value(args, "-in");
    auto file_exts = get_named_arg_value_set(args, "-file-exts");
    auto nspace = get_named_arg_value(args, "-nspace");
    auto rebuild_data_folder = get_named_arg_value(args, "-rebuild-data-folder");
    auto rebuild_data_ext = get_named_arg_value(args, "-rebuild-data-ext"); if (rebuild_data_ext.empty()) rebuild_data_ext = ".obj";
    auto use_rebuild_check = get_named_arg_exists(args, "-use-rebuild-check") && !rebuild_data_folder.empty();
    auto exclude_enums = get_named_arg_value_array(args, "-exclude-enums");
    verbose_messages = get_named_arg_exists(args, "-verbose");

    auto is_valid_file = [&file_exts](std::filesystem::directory_entry& entry) { return entry.is_regular_file() && (file_exts.empty() || file_exts.contains(entry.path().extension().string())); };

    if (verbose_messages)
    {
        std::cout << "[Startup] -in                  " << code_file << "\n";
        std::cout << "[Startup] -file-exts           " << file_exts << "\n";
        std::cout << "[Startup] -nspace              " << nspace << "\n";
        std::cout << "[Startup] -rebuild-data-folder " << rebuild_data_folder << "\n";
        std::cout << "[Startup] -rebuild-data-ext    " << rebuild_data_ext << "\n";
        std::cout << "[Startup] -use-rebuild-check   " << use_rebuild_check << "\n";
        std::cout << "[Startup] -exclude-enums       " << exclude_enums << "\n";
        std::cout << "[Startup] -verbose             " << verbose_messages << "\n";
        std::cout << std::endl;
    }

    auto passes_rebuild_check = [use_rebuild_check, &rebuild_data_folder, &rebuild_data_ext]
        (std::filesystem::directory_entry const& cpp_path)
    {
        if (!use_rebuild_check)
            return true;
        else
            return does_need_rebuild(cpp_path, rebuild_data_folder, rebuild_data_ext);
    };

    std::filesystem::directory_entry entry(code_file);

    if (is_valid_file(entry))
    {
        if (passes_rebuild_check(entry))
            process_file(entry.path(), nspace, exclude_enums);
        else
            std::cout << "[File] Up-to-date: " << entry.path().string() << std::endl;
    }
    else if (entry.is_directory())
    {
        std::cout << "[General] Reading directory \"" << code_file << "\"\n";
        for (auto file_entry : std::filesystem::directory_iterator(entry))
        {
            if (is_valid_file(file_entry))
            {
                if (passes_rebuild_check(file_entry))
                    process_file(file_entry.path(), nspace, exclude_enums);
                else
                    if(verbose_messages)
                        std::cout << "[File] Up-to-date: " << file_entry.path().string() << std::endl;
            }
        }
    }
    else
    {
        std::cout << "[General] \"" << code_file << "\" is not a directory, cannot find file, not a file or not a cpp file." << std::endl;
    }

    std::cout << "[General] All done!" << std::endl;

    return 0;
}
