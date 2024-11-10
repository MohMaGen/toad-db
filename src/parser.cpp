#include <cctype>
#include <iostream>
#include <memory>
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

    bool is_whitespace(char c) {
        return c == ' ' || c == '\n' || c == '\r' || c == '\t';
    }

    std::string_view trim_left(std::string_view str) {
        size_t start = 0;
        while (start < str.size() && is_whitespace(str[start])) start++;
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

    bool is_rule_type(char c) {
        return c == '?' || c == '!' || c == '@';
    }

    std::string_view read_name(std::string_view view) {
        view = trim_left(view); 
        size_t end = 0;
        while (end < view.size() && is_name_char(view[end])) end++;
        return { view.begin(), view.begin() + end };
    } 

    std::string error_help(std::string_view full_view, std::string_view error) {
        std::string ret;
        std::string_view line;
        while ((line = read_until(full_view, '\n')).size() > 0) {
            ret += "\t"; ret += line;
            ret += line.end() == full_view.end() ? "\n" : "";
            if (line.begin() <= error.begin() && line.end() >= error.end()) {
                ret += "\t";
                ret += std::string(error.begin() - line.begin(), ' ');
                ret += std::string(error.size() ? error.size() : 1, '^');
                ret += " -- error\n";
            }

            full_view = { line.end(), full_view.end() };
        }

        return ret;
	}

    Top_Level_Statement::Table_Data parse_table(std::string_view view) {
        Top_Level_Statement::Table_Data data { };
        auto full_view = view;

        view = trim_left(view);
        view = { view.begin() + std::string("table").size(), view.end() };

        auto name_view = read_name(view); 
        if (name_view.size() == 0) {
            throw Expected_Table_Name(error_help(full_view, name_view));
        }
        data.table_name = name_view;

        view = { name_view.end(), view.end() };
        view = trim_left(view); 

        if (view.size() == 0 || view[0] != '{') {
            throw Expected_Char('{',
                    "Table define syntax is:\n"
                    "\ttable Table_Name {\n"
                    "\t                 ^ -- expect open bracket here\n"
                    "\t\t ... fields\n"
                    "\t};\n",
                    error_help(full_view, { view.begin(), view.begin()+1 })
            );
        }
        view = trim_left({ view.begin()+1, view.end() });

        while (view.size() > 0 && view[0] != '}') {
            Top_Level_Statement::Table_Data::Field field;

            auto field_name = read_name(view); 
            if (field_name.size() == 0) throw Expected_Feild_Name(error_help(full_view, field_name));
            field.name = field_name;
            view = trim_left({ field_name.end(), view.end() });

            if (view.size() == 0 || view[0] != '(') {
                throw Expected_Char('(', 
                    "Table define syntax is:\n"
                    "\ttable Table_Name {\n"
                    "\t\tfield_name(Type): some_validation_rule? some_display_rule@ some_gen_rule!,\n"
                    "\t\t          ^ -- expected open bracket there.\n"
                    "\t};\n",
                    error_help(full_view, { view.begin(), view.begin() })
                ); 
            }
            view = { view.begin() + 1, view.end() };

            auto domain_name = read_name(view); 
            if (domain_name.size() == 0) throw Expected_Domain_Name(error_help(full_view, domain_name));
            field.type = domain_name;
            view = trim_left({ domain_name.end(), view.end() });

            if (view.size() == 0 || view[0] != ')') {
                throw Expected_Char(')', 
                    "Table define syntax is:\n"
                    "\ttable Table_Name {\n"
                    "\t\tfield_name(Type): some_validation_rule? some_display_rule@ some_gen_rule!,\n"
                    "\t\t               ^ -- expected close bracket there.\n"
                    "\t};\n",
                    error_help(full_view, { view.begin(), view.begin() })
                ); 
            }
            view = trim_left({ view.begin() + 1, view.end() });

            if (view.size() != 0 && view[0] == ':') {
                view = trim_left({ view.begin() + 1, view.end() }); 

                while (view.size() > 0 && view[0] != ',' && view[0] != '}') {
                    auto rule = read_name(view);
                    if (rule.size() == 0) throw Expected_Rule_Name(error_help(full_view, rule));

                    view = { rule.end(), view.end() };
                    if (view.size() == 0 || !is_rule_type(view[0])) {
                        throw Expected_Rule_Type(error_help(full_view, {view.begin(), view.begin()}));
                    }
                    field.rules.push_back({ rule.begin(), rule.end() + 1 });
                    view = trim_left({ view.begin() + 1, view.end() }); 
                }
            }

            data.fields.push_back(field);

            if (view.size() != 0 && view[0] == ',') {
                view = trim_left({ view.begin() + 1, view.end() });
            }
        }
        if (view.size() == 0) {
            throw Expected_Char('}',
                    "Table define syntax is:\n"
                    "\ttable Table_Name {\n"
                    "\t\t ... fields,\n"
                    "\t};\n"
                    "\t^ -- expect close bracket here.\n",
                    error_help(full_view, { view.begin(), view.begin()+1 })
            );
        }

        return data;
    }

    Top_Level_Statement::Domain_Data parse_domain(std::string_view view) {
        Top_Level_Statement::Domain_Data data { };
        auto full_view = view;

        view = trim_left(view);
        view = { view.begin() + std::string("domain").size(), view.end() };

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
                tree->stmts.push_back(Top_Level_Statement { parse_table(stmt) });
            } break;

            case Top_Level_Statement::Domain_Define: {
                tree->stmts.push_back(Top_Level_Statement { parse_table(stmt) });
            } break;

            case Top_Level_Statement::Function_Define:

            case Top_Level_Statement::Call:
                throw Parsing_Exception("Unimplemented statement type");
            }

            code = { stmt.end(), code.end() };
        }

        return tree;
    }
}
