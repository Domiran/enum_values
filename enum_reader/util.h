#pragma once
namespace enum_reader
{
    extern const std::string whitespace;
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
    void ltrim(std::string& s);
    void rtrim(std::string& s);
    void trim(std::string& s);
    void replace(std::string& source, std::string const& find, std::string const& replaceWith);
    int trim_size(std::string_view const& s);
    void trim_comment(std::string& comment);

    std::string read_file(std::string const& path);
    bool write_file(std::string const& path, std::string const& text);
};
