#include <string>
#include <string_view>
#include <iostream>
#include <filesystem>

#include "util.h"

namespace enum_reader
{
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

    const std::string whitespace = " \r\t\n\f\v";
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

    void replace(std::string& source, std::string const& find, std::string const& replaceWith)
    {
        if (source.empty())
            return;

        auto findLen = find.size();
        auto replaceLen = replaceWith.size();

        size_t startPos = source.find(find);
        while (startPos != std::string::npos)
        {
            source.replace(startPos, findLen, replaceWith);
            startPos = source.find(find, startPos + replaceLen);
        }
    }

    int trim_size(std::string_view const& s)
    {
        auto b = s.find_first_not_of(whitespace);
        if (b == std::string::npos) // if one failed, both will
        {
            return 0;
        }
        auto e = s.find_last_not_of(whitespace);
        return static_cast<int>(e - b) + 1;
    }

    void sanitize_comment(std::string& comment)
    {
        if (comment.empty())
            return;
        auto is_multiline = comment.starts_with("/*");
        comment = comment.substr(2); // strip off the "//" or "/*"
        if (is_multiline)
        {
            auto endcomment = comment.find("*/");
            comment = comment.substr(0, endcomment);
        }
        trim(comment);
        replace(comment, "\r\n", "\r\n");
        replace(comment, "\n", "\r");
        replace(comment, "\r", "\\\r");
    }

    std::string remove_quotes(std::string text)
    {
        size_t start = 0U;
        size_t end = text.size();
        if (text.starts_with("\""))
            start++;
        if (text.ends_with("\""))
            end--;

        return text.substr(start, end - start);
    }


    std::string read_file(std::string const& path)
    {
        std::filesystem::directory_entry entry(path);
        size_t file_size = static_cast<size_t>(entry.file_size());
        auto file_data = std::unique_ptr<char>(new char[file_size + 1]);
        FILE* f;
        auto err = fopen_s(&f, path.c_str(), "rb");
        if (err)
        {
            std::cerr << "Reading file \"" + path + "\" got error " + std::to_string(err) + ".";
            return std::string();
        }
        auto read_size = fread(file_data.get(), 1, file_size, f);
        file_data.get()[read_size] = 0;
        fclose(f);
        return std::string(file_data.get());
    }

    bool write_file(std::string const& path, std::string const& text)
    {
        FILE* f;
        auto err = fopen_s(&f, path.c_str(), "wb");
        if (err)
        {
            std::cerr << "Writing file \"" + path + "\" got error " + std::to_string(err) + ".";
            return false;
        }
        auto written = fwrite(text.c_str(), 1, text.size(), f);
        fclose(f);

        if (written != text.size())
        {
            std::cerr << "Writing lib file, wrote " + std::to_string(written) + " bytes but have " + std::to_string(text.size()) + " bytes to write.";
        }

        return written = text.size();
    }
}
