#include <algorithm>
#include <cctype>
#include <functional>
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
                case Top_Level_Statement::Domain_Define: {
                    os << "  (DOMAIN: `" << stmt.domain_data.domain_name << "`):\n";
                    switch (stmt.domain_data.variant) {
                    case Top_Level_Statement::Domain_Data::Alias: {
                        os << "    (Domain: `" << stmt.domain_data.fields[0].name << "`).\n";
                    } break;
                    case Top_Level_Statement::Domain_Data::Mul: {
                        for (auto &field: stmt.domain_data.fields) {
                            os << "    (Mul Field: `" << field.name << "`):\n";
                            os << "      (domain: `" << field.domain << "`).\n";
                        }
                    } break;
                    case Top_Level_Statement::Domain_Data::Add: {
                        for (auto &field: stmt.domain_data.fields) {
                            os << "    (Add Field: `" << field.name << "`):\n";
                            os << "      (domain: `" << field.domain << "`).\n";
                        }
                    } break;
                    }

                } break;
                case Top_Level_Statement::Call: {
                    os << "  (CALL):\n";
                    os << "    (Expr: `" << to_string(stmt.call_data)  << "`)\n";
                } break;

                case Top_Level_Statement::Function_Define:
                default: {
                    os << "not implemented!" << std::endl;
                }
            }
            os << std::endl;
        }
        return os;
    }
    std::string to_string(Top_Level_Statement::Expression_Data::pointer ptr) {
		using Kind = Top_Level_Statement::Expression_Data::kind;

        if (ptr->kind == Kind::Expression) {
            return "{" + (ptr->args.size() == 1 ? to_string(ptr->args[0]) : "" ) + "}";
        }
        auto prefix = [ptr] -> std::string {
            switch (ptr->kind) {
            case Kind::Num_Literal:
            case Kind::Char_Literal:
            case Kind::Str_Literal: return "L:";

            case Kind::Name: return "N:";

            case Kind::Operator: return "O:";

            default: return "";
            }
        }();
        std::string ret = "(" + prefix + "'" + std::string(ptr->name) + "'";

        for (auto arg: ptr->args) {
            ret += " " + to_string(arg);
        }

        return ret + ")";
    }

    std::string to_string(const Top_Level_Statement::Expression_Data &data) {
        return to_string(data.root);
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
        case Top_Level_Statement::None: return "none";
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
            if (domain_name.size() == 0) throw Expected_Table_Field_Domain_Name(error_help(full_view, domain_name));
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
                    if (rule.size() == 0) throw Expected_Table_Field_Rule_Name(error_help(full_view, rule));

                    view = { rule.end(), view.end() };
                    if (view.size() == 0 || !is_rule_type(view[0])) {
                        throw Expected_Table_Field_Rule_Type(error_help(full_view, {view.begin(), view.begin()}));
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

        auto domain_name = read_name(view); 
        if (domain_name.size() == 0) {
            throw Expected_Domain_Name(error_help(full_view, domain_name));
        }
        data.domain_name = domain_name;


        view = trim_left({ domain_name.end(), view.end() });
        if (view.size() <= 1 || !view.starts_with(":=")) {
            throw Expected_Domain_Walrus_Operator(
                error_help(full_view, { view.begin(), view.begin() }) );
        }

        view.remove_prefix(2); 
        view = trim_left(view); 

        using Variant = Top_Level_Statement::Domain_Data::Variant;
        Variant curr_variant = Variant::Alias;
        std::vector<Top_Level_Statement::Domain_Data::Field> fields { 0 };

        while (view.size() == 0 || view[0] != ';') {
            auto field = Top_Level_Statement::Domain_Data::Field { };

            auto field_name = read_name(view);
            if (field_name.size() == 0) {
                throw Parsing_Exception("Expected domain name or field name");
            }
            field.name = field_name;
            view = trim_left({ field_name.end(), view.end() });

            if (view.size() > 0 && view[0] == '(') {
                view = trim_left({ view.begin() + 1, view.end() });

                auto domain_name = read_name(view);
                if (domain_name.size() == 0) {
                    throw Parsing_Exception("Expected domain name");
                }
                view = trim_left({ domain_name.end(), view.end() });

                if (view.size() == 0 || view[0] != ')') {
                    throw Parsing_Exception("Expected closing bracket!");
                }

                view = trim_left({ view.begin() + 1, view.end() });
                field.domain = domain_name;
            }

            fields.push_back(field);

            if (view.size() > 0 && view[0] == '|') {
                if (curr_variant == Variant::Mul) {
                    throw Parsing_Exception("Pipe operator in mul domain!");
                }

                curr_variant = Variant::Add;
                view = trim_left({ view.begin() + 1, view.end() });
            }

            if (view.size() > 0 && view[0] == '&') {
                if (curr_variant == Variant::Add) {
                    throw Parsing_Exception("And operator in add domain!");
                }

                curr_variant = Variant::Mul;
                view = trim_left({ view.begin() + 1, view.end() });
            }
        }

        if (fields.size() == 0) {
            throw Expect_Fields(error_help(full_view, {view.begin(), view.begin()}) );
        }

        data.fields = fields;
        data.variant = curr_variant;

        return data;
    }

    std::string_view read_before_closes(std::string_view view, char open, char close) {
        size_t counter = 1, end = 0;

        while (end < view.size() && counter != 0) {
            if (view[end] == open) counter++;
            if (view[end] == close) counter--;
            end++;
        }

        if (counter > 0) throw Expected_Char(close, 
            std::string("\t") + std::to_string(open) + " ... " + std::to_string(close) + "\n" +
            "\t      ^ -- expected `" + std::to_string(close) + "`\n",
            error_help(view, {view.end()-1, view.end()-1})
            );

        return { view.begin(), view.begin() + end };
    }


    std::pair<std::string_view, Top_Level_Statement::Expression_Data::kind>
    read_literal(std::string_view view) {
        using Kind = Top_Level_Statement::Expression_Data::kind;

        view = trim_left(view);
        if (view.size() > 0 && (view[0] == '"' || view[0] == '\'')) {
            return { 
                {view.begin(), read_until({view.begin()+1, view.end()}, view[0]).end()}, 
                view[0] == '"' ? Kind::Str_Literal : Kind::Char_Literal }; 
        }


        if (view.size() > 0 && (view[0] == '+' || view[0] == '-' || std::isdigit(view[0]))) {
            size_t end = 1;
            while (end < view.size() && std::isdigit(view[end])) { end++; }

            if (end == 1 && !std::isdigit(view[0])) {
                return {{ view.begin(), view.begin()}, Kind::Err};
            }
            return {{ view.begin(), view.begin() + end}, Kind::Num_Literal };
        }

        return {{ view.begin(), view.begin() }, Kind::Err };
    }

    struct Operator {
        std::string name;
        size_t order;
        enum Kind { Bynary=0, Prefix, Postfix } kind=Bynary;
    };

    static const std::vector<Operator> operators = {
        { "with", 5 },
        { "==", 1 }, { "!=", 1 }, { "<=", 1 }, { ">=", 1 }, { ":=", 0 },
        { "**", 5 }, { "as", 5 },
        { "@", 6 },
        { "+", 3 }, { "-", 3 }, { "*", 4 }, { "/", 4 }, { "^", 5 }, { "=", 0 },
        { ">", 1}, { "<", 1 }, 
    };

    std::string_view read_operator(std::string_view view) {
        view = trim_left(view);

        auto op = std::find_if(operators.begin(), operators.end(), [&view] (auto &op)
                               { return view.starts_with(op.name); });

        if (op == operators.end()) {
            return { view.begin(), view.begin() };
        }

        return { view.begin(), view.begin() + op->name.size() };
    }

    Operator get_operator(std::string_view view) {
        view = trim_left(view);

        auto op = std::find_if(operators.begin(), operators.end(), [&view] (auto &op)
                               { return view == op.name; });

        if (op == operators.end()) {
            throw Parsing_Exception("Value: `" + std::string(view) + "` -- is not operator!");
        }

        return *op;
    }

    bool operator_expect_args(std::string_view view, size_t args) {

        switch (get_operator(view).kind) {
        case Operator::Bynary: return args < 2;
        case Operator::Prefix:
        case Operator::Postfix: return args < 1;
        }
    }

    struct Bound_Operator {
        enum Variant {
            Once, Multiple, Close
        };
        struct Node {
            std::string name;
            Variant variant;
        };
        std::vector<Node> nodes;
    };


    static const std::vector<Bound_Operator> bound_operators = {
        {{{"if", Bound_Operator::Once}, {"then", Bound_Operator::Once}, {"else", Bound_Operator::Once}}},
        {{{"let", Bound_Operator::Once}, {"in", Bound_Operator::Once}}},
        {{{"[", Bound_Operator::Once}, {",", Bound_Operator::Multiple}, {"]", Bound_Operator::Close}}},
        {{{"(", Bound_Operator::Once}, {",", Bound_Operator::Multiple}, {")", Bound_Operator::Close}}},
        {{{"<", Bound_Operator::Once}, {",", Bound_Operator::Multiple}, {">", Bound_Operator::Close}}},
        {{{"{", Bound_Operator::Once}, {",", Bound_Operator::Multiple}, {"}", Bound_Operator::Close}}},
    };
    // [( | , | ; | )]
    // [( a, b, c, d; a; d; c )]

    std::string_view read_until(std::string_view view, std::function<bool (std::string_view)> pred) {
        std::string_view ret = view;

        while (view.begin() != view.end() && !pred(view)) view.remove_prefix(1);

        return trim_left({ ret.begin(), view.begin() });
    }

    std::string_view read_node(std::string_view view, const Bound_Operator &op, size_t &idx) {
        int level = 0;
        auto start = view.begin();

        while (view.begin() < view.end()) {
            if (level == 0) {
                size_t prev = idx;

                while (++idx < op.nodes.size()) {
                    if (view.starts_with(op.nodes[idx].name)) {
                        if (op.nodes[idx].variant == Bound_Operator::Variant::Multiple) idx--;
                        return { start, view.begin() };
                    }

                    if (op.nodes[idx].variant != Bound_Operator::Variant::Multiple) break;
                }

                idx = prev;
            }

            if (view.starts_with(op.nodes.begin()->name)) level++;
            if (view.starts_with(op.nodes.rbegin()->name)) level--;

            view = { view.begin()+1, view.end() };
        }

        if (level != 0) return { view.begin(), view.begin() };
        idx++;

        return { start, view.begin() };
    }

    std::vector<std::string_view> read_bound_operator(std::string_view view) {
        auto full_view = view;
        view = trim_left(view);
        auto op = std::find_if(bound_operators.begin(), bound_operators.end(),
                               [view](auto op){ return view.starts_with(op.nodes[0].name); });

        if (op == bound_operators.end()) return { };

        std::vector<std::string_view> ret { };

        size_t i = 0;
        while (i < op->nodes.size()) {
            auto node = op->nodes[i];
            ret.push_back({ view.begin(), view.begin() + node.name.size() });
            view = { view.begin() + node.name.size(), view.end() };

            if (op->nodes[i].variant == Bound_Operator::Close) break;

            auto expr = read_node(view, *op, i);

            if (op->nodes[i].variant == Bound_Operator::Once && expr.size() == 0) {
                throw Expected_Bound_Operator_Node_Expr(op->nodes[i].name, error_help(full_view, expr));
            }

            view = { expr.end(), view.end() };

            ret.push_back(expr);
        }

        return ret;
    }

    Top_Level_Statement::Expression_Data parse_call(std::string_view view) {
        using Expression_Data = Top_Level_Statement::Expression_Data;
        std::string_view full_view = view;

        std::vector<Expression_Data::pointer> stack;
        Expression_Data::pointer curr;
        Expression_Data root {};

        const auto push_node = [&root, &curr](auto node) -> void {
            if (root.root->args.size() == 0) {
                root.root->args.push_back(node);
                curr = node;
                return;
            }
            if (node->kind != Expression_Data::kind::Operator) {
                curr->args.push_back(node);
                return;
            }

            auto prev = root.root;
            curr = root.root->args[0];
            while (curr->kind == Expression_Data::kind::Operator
                   && get_operator(curr->name).order < get_operator(node->name).order) {

                if (curr->args.size() == 0) {
                    prev = curr;
                    curr = NULL;
                    break;
                }

                prev = curr;
                curr = *curr->args.rbegin();
            }

            if (curr == NULL) {
                prev->args.push_back(node);
            } else {
                *prev->args.rbegin() = node;
                node->args.push_back(curr);
            }
            curr = node;
        };


        while (view.size() > 0 && view[0] != ';') {
            view = trim_left(view); 

            /*if (view.size() > 0 && view[0] == ')') {
                view.remove_prefix(1); 
                continue;
            }
            if (view.size() > 0 && view[0] == '(') {
                auto sub_expression_view = read_before_closes({view.begin()+1, view.end()}, '(', ')'); 
                auto sub_expression = parse_call(sub_expression_view);

                push_node( sub_expression.root );
                view = { sub_expression_view.end(), view.end() };
                continue;
            }*/

            auto bop_views = read_bound_operator(view);
            if (bop_views.size() > 0) {
                auto node = Expression_Data::make_pointer(
                    Expression_Data::kind::Bound_Operator, 
                    bop_views[0]
                );
                view = { bop_views[0].end(), view.end() };


                bool is_expr = true;
                for (auto expr : std::span(bop_views.begin()+1, bop_views.size()-1)) {
                    if (is_expr) {
                        node->args.push_back( parse_call(expr).root  );
                    } else {
                        node->args.push_back(Expression_Data::make_pointer(
                            Expression_Data::kind::Name,
                            expr
                        ));
                    }
					is_expr = !is_expr;
                    view = { expr.end(), view.end() };
                }


                push_node(node); 
                view = trim_left(view);
                continue;
            }

            auto [literal, kind] = read_literal(view);
            if (kind != Expression_Data::kind::Err) {
                push_node( Expression_Data::make_pointer(kind, literal) );
                view = { literal.end(), view.end() };
                continue;
            }


            auto op = read_operator(view);
            if (op.size() > 0) {
                push_node( Expression_Data::make_pointer(Expression_Data::kind::Operator, op) );
                view = { op.end(), view.end() };
                continue;
            }

            auto name = read_name(view);
            if (name.size() > 0) {
                push_node( Expression_Data::make_pointer(Expression_Data::kind::Name, name) );
                view = { name.end(), view.end() };
                continue;
            }

            view = trim_left(view); 
            if (view.size() != 0 && view[0] != ';')
                throw Unexpected_Call(error_help(full_view, { view.begin(), view.begin() })); 
        }

        return { root };
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
                tree->stmts.push_back(Top_Level_Statement { parse_domain(stmt) });
            } break;

            case Top_Level_Statement::Call: {
                tree->stmts.push_back(Top_Level_Statement { parse_call(stmt) });
            } break;

            case Top_Level_Statement::Function_Define:


            case Top_Level_Statement::None:
                throw Parsing_Exception("Unimplemented statement type");

            }

            code = { stmt.end(), code.end() };
        }

        return tree;
    }
}
