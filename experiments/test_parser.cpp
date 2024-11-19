#include <iostream>
#include <parser.hpp>

int main() {
    using namespace toad_db::parser;

    const std::string hello_table =
        "table Hello_Values {\n"
        "   id(Key),\n"
        "   value(Str): not_null?,\n"
        "}";

    std::cout << "stmt: `" << read_stmt(hello_table) << "`." << std::endl; 
    std::cout << "stmt: `" << read_stmt("        \n\t\t\n     " + hello_table) << "`." << std::endl; 
    std::cout << "stmt: `" << read_stmt(hello_table + ";") << "`." << std::endl; 
    std::cout << "stmt: `" << read_stmt("        \n\t\t\n     " + hello_table + ";\nasfd") << "`." << std::endl; 

    std::cout << "Hello Values table: \n" << *Syntax_Tree::parse(hello_table) << "\nEnd:\n" << std::endl;

    std::cout << "Hello Domain domain: \n" << *Syntax_Tree::parse("domain Hello_domain := aaa;") << "\nEnd:\n" << std::endl;

    std::cout << "Domains:\n" << *Syntax_Tree::parse(
            "domain Vector2 := x(F32) & y(F32);\n"
            "domain Vector3 := x(F32) & y(F32) & z(F32);\n"
            "domain Vector4 := x(F32) & y(F32) & z(F32) & w(F32);\n"
            "domain Vector  := v2(Vector2) | v3(Vector3) | v4(Vector4);\n"
        ) << "\nEnd:" << std::endl;


    std::cout << "Call:\n" << *Syntax_Tree::parse(
            "(a + b);\n"

            "let a = 10 in a + b;\n"

            "let a = 10 in\n"
            "let b = 20 in\n"
            "    a + b;\n"

            "let a = \n"
            "      let b = 10 in b + 20\n"
            "      in a + 5;\n"

            "if a == b then\n"
            "   if b > 0 then 0 else 10\n"
            "else\n"
            "   20\n;"

        ) << "\nEnd:" << std::endl;

    std::cout << "Call:\n" << *Syntax_Tree::parse(
            "display (a + b);\n"
            "a + b + c * g;\n"
            "aboba := a + b;\n"
            "aboba := a + ++b;\n"
            ";\n"
            "a + b + c * g ** e + d;\n"
            "a + b + (c * g) ** e + d;\n"
            "(sum array) / (length array) + 10;\n"
            "ret := a >= c;\n" 
            "index Table1 \"row1\";\n"
            "Table1@ * Table2@;\n"
            "display (filter (column \"name\") (view Table1)) \"wide\"  ;\n"
            "if a == b then 10 else 20;\n"
        ) << "\nEnd:" << std::endl;

    std::cout << "Call:\n" << *Syntax_Tree::parse(
            "[a, b, c];\n"
            "[[1 + 2, 2 + 3], [3 + 4, 4 + 5]];\n"
            "let view_1 = Table1 with (name as name_1, age as age_1, tb2_id) in\n"
            "let view_2 = Table2 with (id, title as title_1) in\n"
            "display (view_1 * view_2)\n;"
        ) << "\nEnd:" << std::endl;


    return 0;
}
