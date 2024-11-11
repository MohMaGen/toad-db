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

    std::cout << "Hello Values table: \n" << *parse(hello_table) << "\nEnd:\n" << std::endl;

    try { parse_table("table {\n};"); }
    catch (Parsing_Exception& e) { std::cout << "Failed: " << e.what() << std::endl; }

    try { parse_table("table Table_Name [\n};"); }
    catch (Parsing_Exception& e) { std::cout << "Failed: " << e.what() << std::endl; }

    std::cout << "Hello Domain domain: \n" << *parse("domain Hello_domain := aaa;") << "\nEnd:\n" << std::endl;

    std::cout << "Domains:\n" << *parse(
            "domain Vector2 := x(F32) & y(F32);\n"
            "domain Vector3 := x(F32) & y(F32) & z(F32);\n"
            "domain Vector4 := x(F32) & y(F32) & z(F32) & w(F32);\n"
            "domain Vector  := v2(Vector2) | v3(Vector3) | v4(Vector4);\n"
        ) << "\nEnd:" << std::endl;

    return 0;
}
