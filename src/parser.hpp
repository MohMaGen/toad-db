#ifndef parser_hpp_INCLUDED
#define parser_hpp_INCLUDED

#include <memory>
#include <string>
#include <vector>
#include <common.hpp>
/**
 * file = [stmt|\n+]
 * 
 * stmt      = <expt_type> <expr>;
 * expt_type = <table | domain | function | insert | display | delete | update>
 * expr      = <<table_def> | <domain_def> | <func_def> | <call>> 
 **/
namespace toad_db::parser {

    struct Top_Level_Statement {
        enum Variant {
            Table_Define, Domain_Define, Function_Define, Call
        } variant;

        struct Table_Data {
            std::string_view table_name;

            struct Field {
                std::string_view name;
                std::string_view type;

                std::vector<std::string_view> rules; 
            };
            std::vector<Field> fields;
        };

        union {
            Table_Data table_data;
        };

        Top_Level_Statement() { }
        Top_Level_Statement(const Top_Level_Statement&& v): variant(v.variant) {
            switch (v.variant) {
            case Table_Define: table_data = std::move(v.table_data); break;
            case Domain_Define:
            case Function_Define:
            case Call:
            }
        }

        Top_Level_Statement(const Table_Data &data): variant(Table_Define), table_data(data) { }

        ~Top_Level_Statement() {
        }
    };

    std::string to_string(Top_Level_Statement::Variant);

    struct Syntax_Tree {
        std::string content;
        std::vector<Top_Level_Statement> stmts;
    };
    std::ostream& operator<<(std::ostream& os, const Syntax_Tree& tree);

    std::unique_ptr<Syntax_Tree> parse(const std::string &source);


    std::string_view trim_left(std::string_view str);

    std::string_view read_until(std::string_view str, char sep='\n');

    std::string_view read_stmt(std::string_view str);

	std::string_view read_name(std::string_view str);

	bool is_name_char(char c);

    Top_Level_Statement::Table_Data parse_table(std::string_view view);


    class Parsing_Exception: public Toad_Exception {
        public:
            Parsing_Exception(const std::string& what): Toad_Exception("Parsing: " + what) {} 
    };

    class Expected_Table_Name: public Parsing_Exception {
        public:
            Expected_Table_Name(): Parsing_Exception (
                "Expected table name. Table define syntax is:\n"
                "\ttable Table_Name {\n"
                "\t      ^^^^^^^^^^ -- there must be name.\n"
                "\t\t ... fields,\n"
                "\t};\n"
            ) {}
    };

}

#endif // parser_hpp_INCLUDED

