#include <cctype>
#include <memory>
#include <optional>
#include <ostream>
#include <parser.hpp>
#include <string_view>


namespace toad_db::parser {
    std::ostream& operator<<(std::ostream& os, const Syntax_Tree &tree) {
        os << "Syntax Tree:" << std::endl;

        for (auto &stmt: tree.stmts) {
            switch (stmt.variant) {
                case Top_Level_Statement::Table_Define: {
                    os << "  (TABLE: `" << stmt.table_data.table_name << "`):\n";
                    for (auto &field: stmt.table_data.fields) {
                        os << "    (Feild: `" << field.name << "`):\n";
                        os << "      (domain: `" << field.type << "`).\n";
                        for (auto &rule: field.rules) {
                            os << "      (rule: `" << rule << "`).\n";
                        }
                    }
                } break;
                case Top_Level_Statement::Domain_Define:
                case Top_Level_Statement::Function_Define:
                case Top_Level_Statement::Call:
                default: {
                    os << "not implemented!" << std::endl;
                }
            }
            os << std::endl;
        }
        return os;
    }

    std::string_view trim_left(std::string_view str) {
        size_t start = 0;
        while (start < str.size() && (str[start] == ' ' || str[start] == '\t' || str[start] == '\n')) start++;
        return { str.begin() + start, str.end() };
    }

    std::string_view read_until(std::string_view str, char sep) {
        size_t end = 0;
        while (end < str.size() && str[end++] != sep);
        return { str.begin(), str.begin() + end };
    }

    std::string_view read_stmt(std::string_view str) {
        return read_until(trim_left(str), ';');
    }

    Top_Level_Statement::Variant read_variant(std::string_view stmt) {
        if (stmt.starts_with("table")) {
            return Top_Level_Statement::Variant::Table_Define;
        }

        if (stmt.starts_with("domain")) {
            return Top_Level_Statement::Variant::Domain_Define;
        }

        if (stmt.starts_with("function")) {
            return Top_Level_Statement::Variant::Function_Define;
        }

        return Top_Level_Statement::Variant::Call;
    }

    std::string to_string(Top_Level_Statement::Variant variant) {
        switch (variant) {
        case Top_Level_Statement::Table_Define: return "table definition";
        case Top_Level_Statement::Domain_Define: return "domain definition";
        case Top_Level_Statement::Function_Define: return "function definition";
        case Top_Level_Statement::Call: return "call statement";
        }
    }


    bool is_name_char(char c) {
        return std::isalnum(c) || c == '_'; 
    }

    std::string_view read_name(std::string_view view) {
        view = trim_left(view); 
        size_t end = 0;
        while (end < view.size() && is_name_char(view[end])) end++;
        return { view.begin(), view.begin() + end };
	} 

    Top_Level_Statement::Table_Data parse_table(std::string_view view) {
        Top_Level_Statement::Table_Data data { };

        view = { view.begin() + std::string("table").size(), view.end() };
        data.table_name = read_name(view); 
        if (data.table_name.size() == 0) {
            throw Expected_Table_Name();
        }


        return data;
    }

    std::unique_ptr<Syntax_Tree> parse(const std::string &source) {
        auto tree = std::make_unique<Syntax_Tree>(Syntax_Tree {});

        tree->content = source;

        std::string_view code = {tree->content.begin(), tree->content.end()};
        while (code.size() >= 0) {
            std::string_view stmt = read_stmt(code);
            if (stmt.size() == 0) break;

            switch (read_variant(stmt)) {
            case Top_Level_Statement::Table_Define: {
                tree->stmts.push_back(Top_Level_Statement{ parse_table(stmt) });
            } break;
            case Top_Level_Statement::Domain_Define:
            case Top_Level_Statement::Function_Define:
            case Top_Level_Statement::Call:
                throw Parsing_Exception("Unimplemented statement type");
            }

            code = { stmt.end(), code.end() };
        }

        return tree;
    }
}
