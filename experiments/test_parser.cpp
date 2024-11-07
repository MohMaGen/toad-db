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

    try { parse_table("table { };"); }
    catch (Parsing_Exception& e) { std::cout << "Failed: " << e.what() << std::endl; }

    return 0;
}
