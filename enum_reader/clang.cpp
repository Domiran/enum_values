#include <regex>
#include <set>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include "util.h"
#include "clang-c/Index.h"
#include "data.h"
#include "clang.h"

namespace enum_reader
{
    extern std::regex enum_values_comment_reg;
    const std::set<std::string> exclude_files{ "vector", "string", "unordered_map", "map", "set" };

    struct stuff
    {
        std::vector<enum_data> found_enums;
        std::string nspace;
    };

    std::string get_and_destroy(CXString s)
    {
        if (s.data == nullptr)
            return std::string();
        auto txt = std::string(clang_getCString(s));
        clang_disposeString(s);
        return txt;
    }

    void get_alt_name_and_desc(std::string const& full_comment, std::string& out_desc, std::string& out_name)
    {
        auto comment_iter = std::sregex_iterator(full_comment.begin(), full_comment.end(), enum_values_comment_reg);
        auto end_iter = std::sregex_iterator();

        auto alt_name = (comment_iter != end_iter ? (*comment_iter)[1].str() : std::string());
        out_desc = (comment_iter != end_iter ? (*comment_iter)[2].str() : std::string());

        trim(out_desc);
        trim(alt_name);

        if (!alt_name.empty())
        {
            out_name = alt_name.substr(2, alt_name.size() - 4);
        }
    }

    bool is_in_namespace(CXCursor c, std::string const& nspace, std::string& out_prefix, bool ok = false)
    {
        auto parent = clang_getCursorSemanticParent(c);
        auto kind = clang_getCursorKind(parent);
        switch (kind)
        {
        case CXCursorKind::CXCursor_Namespace:
        case CXCursorKind::CXCursor_ClassDecl:
        case CXCursorKind::CXCursor_StructDecl:
        case CXCursorKind::CXCursor_ClassTemplate:
        case CXCursorKind::CXCursor_ClassTemplatePartialSpecialization:
            auto name = clang_getCursorDisplayName(parent);
            auto txt = std::string(clang_getCString(name));
            if (txt == nspace)
                ok = true;
            clang_disposeString(name);
            out_prefix = std::string(txt) + "::" + out_prefix;
            break;
        }

        if (clang_isInvalid(kind))
            return ok;

        ok |= is_in_namespace(parent, nspace, out_prefix, ok);
        return ok;
    }

    enum_data get_enum_data(CXCursor c, std::string const& prefix)
    {
        enum_data e;
        e.prefix = prefix;

        auto name = get_and_destroy(clang_getCursorDisplayName(c));
        auto cmt = get_and_destroy(clang_Cursor_getRawCommentText(c));
        
        e.name = name;
        trim(e.name);
        e.desc = cmt;
        trim_comment(e.desc);

        std::cout << "[Enum] Found enum " << prefix << name << std::endl;

        auto value_visitor = [](CXCursor c, [[maybe_unused]] CXCursor parent, CXClientData client_data)
        {
            auto& e = *static_cast<enum_data*>(client_data);

            auto kind = clang_getCursorKind(c);
            if (kind == CXCursorKind::CXCursor_EnumConstantDecl)
            {
                auto symbol = get_and_destroy(clang_getCursorDisplayName(c));
                auto cmt = get_and_destroy(clang_Cursor_getRawCommentText(c));
                std::string desc;
                std::string name;
                get_alt_name_and_desc(cmt, desc, name);

                e.add_value(symbol, name, desc);
            }

            return CXChildVisit_Continue;
        };

        clang_visitChildren(c, value_visitor, &e);

        return e;
    }

	std::vector<enum_data> get_enums_in_file(std::string const& file, std::string const& in_nspace)
	{
        std::vector<enum_data> result;
        auto options = CXTranslationUnit_SkipFunctionBodies | CXTranslationUnit_IncludeBriefCommentsInCodeCompletion;
        const char* cmdline = "-fparse-all-comments";
        CXIndex index = clang_createIndex(0, 0);
        auto unit = clang_parseTranslationUnit(
            index,
            file.c_str(), &cmdline, 1,
            nullptr, 0,
            options);

        if (unit == nullptr)
        {
            std::cerr << "[LibClang] Can't parse file " << file << ". Ensure all files are compilable." << std::endl;
            exit(-1);
        }

        stuff s{ {}, in_nspace };

        auto cursor = clang_getTranslationUnitCursor(unit);
        auto enum_visitor = [](CXCursor c, [[maybe_unused]] CXCursor parent, CXClientData client_data)
        {
            auto& data = *static_cast<stuff*>(client_data);
            auto loc = clang_getCursorLocation(c);
            if(!clang_Location_isInSystemHeader(loc))
            {
                if (clang_getCursorKind(c) == CXCursorKind::CXCursor_EnumDecl)
                {
                    std::string prefix;
                    if (is_in_namespace(c, data.nspace, prefix))
                    {
                        data.found_enums.push_back(get_enum_data(c, prefix));
                        std::cout << "[Enum] Found enum " << prefix << data.found_enums.back().name << std::endl;
                    }
                }
            }
            return CXChildVisit_Recurse;
        };

        clang_visitChildren(cursor, enum_visitor, &s);
        clang_disposeTranslationUnit(unit);
        clang_disposeIndex(index);

        return s.found_enums;
	}
}
