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
            Table_Define, Domain_Define, Function_Define, Call, None
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

        struct Domain_Data {
            std::string_view domain_name;
            enum Variant { Alias, Mul, Add } variant;

            struct Field {
                std::string_view name;
                std::string_view domain;
            };
            std::vector<Field> fields;
        };

        union {
            Table_Data table_data;
            Domain_Data domain_data;
        };

        Top_Level_Statement(): variant(None) { }
        Top_Level_Statement(const Top_Level_Statement& v): variant(v.variant) {
            switch (v.variant) {
            case Table_Define: std::construct_at(&table_data, v.table_data); break;
            case Domain_Define: std::construct_at(&domain_data, v.domain_data); break; 
            case Function_Define:
            case Call:
            case None:
            }
        }
        Top_Level_Statement(const Top_Level_Statement&& v): variant(v.variant) {
            switch (v.variant) {
            case Table_Define: std::construct_at(&table_data, v.table_data); break;
            case Domain_Define: std::construct_at(&domain_data, v.domain_data); break; 
            case Function_Define:
            case Call:
            case None:
            }
        }

        Top_Level_Statement(const Table_Data &data): variant(Table_Define) {
            std::construct_at(&table_data, data);
        }

        Top_Level_Statement(const Domain_Data &data): variant(Domain_Define) {
            std::construct_at(&domain_data, data);
        }

        ~Top_Level_Statement() {
            switch (variant) {
            case Table_Define: table_data.~Table_Data(); break;
            case Domain_Define: domain_data.~Domain_Data(); break;
            case Function_Define:
            case Call:
            case None:
            }
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
            Expected_Table_Name(const std::string &error_help): Parsing_Exception (
                "Expected table name. Expected:\n"
                "Table define syntax is:\n"
                "\ttable Table_Name {\n"
                "\t      ^^^^^^^^^^ -- there must be name.\n"
                "\t\t ... fields,\n"
                "\t};\n"
                "But get:\n" + error_help
            ) {}
    };

    class Expected_Char: public Parsing_Exception {
        public:
            Expected_Char(char c, const std::string& expected, const std::string& error_help):
                Parsing_Exception(std::string("Expected `") + c +
                    "`. Expected:\n" + expected +
                    "But get:\n" + error_help
                    ) {} 
    };

    class Expected_Feild_Name: public Parsing_Exception {
        public:
            Expected_Feild_Name(const std::string &error_help): Parsing_Exception (
                "Expected field name. Expected:\n"
                "Table define syntax is:\n"
                "\ttable Table_Name {\n"
                "\t\tfield_name(Type): query? query@ query!,\n"
                "\t\t^^^^^^^^^^ -- there mut be name.\n"
                "\t};\n"
                "But get:\n" + error_help
            ) {}
    };

    class Expected_Table_Field_Domain_Name: public Parsing_Exception {
        public:
            Expected_Table_Field_Domain_Name(const std::string &error_help): Parsing_Exception (
                "Expected domain name. Expected:\n"
                "Table define syntax is:\n"
                "\ttable Table_Name {\n"
                "\t\tfield_name(Type): some_validation_rule? some_display_rule@ some_gen_rule!,\n"
                "\t\t           ^^^^ -- there mut be name.\n"
                "\t};\n"
                "But get:\n" + error_help
            ) {}
    };
    class Expected_Table_Field_Rule_Name: public Parsing_Exception {
        public:
            Expected_Table_Field_Rule_Name(const std::string &error_help): Parsing_Exception (
                "Expected rule name. Expected:\n"
                "Table define syntax is:\n"
                "\ttable Table_Name {\n"
                "\t\tfield_name(Type): some_validation_rule? some_display_rule@ some_gen_rule!,\n"
                "\t\t                  ^^^^^^^^^^^^^^^^^^^^^ -- there mut be name.\n"
                "\t};\n"
                "But get:\n" + error_help
            ) {}
    };


    class Expected_Table_Field_Rule_Type: public Parsing_Exception {
        public:
            Expected_Table_Field_Rule_Type(const std::string &error_help): Parsing_Exception (
                "Expected rule type. Expected:\n"
                "Table define syntax is:\n"
                "\ttable Table_Name {\n"
                "\t\tfield_name(Type): some_validation_rule? some_display_rule@ some_gen_rule!,\n"
                "\t\t                                      ^ -- There must be rule type:\n"
                "\t\t                                          '!' -> for generation rule.\n"
                "\t\t                                          '?' -> for validation rule.\n"
                "\t\t                                          '@' -> for display rule.\n"
                "\t};\n"
                "But get:\n" + error_help
            ) {}
    };

    class Expected_Domain_Name: public Parsing_Exception {
        public:
            Expected_Domain_Name(const std::string& error_help): Parsing_Exception(
                "Expected domain name. Expected:\n"
                "Domain define syntax is:\n"
                "\tdomain Domain_Name := ...domain_fields;\n"
                "\t       ^^^^^^^^^^^ -- expected name there.\n"
                "But get:\n" + error_help
            )  {}
    };

    class Expected_Domain_Walrus_Operator: public Parsing_Exception {
        public:
            Expected_Domain_Walrus_Operator(const std::string& error_help): Parsing_Exception(
                "Expected walrus operator. Expected:\n"
                "Domain define syntax is:\n"
                "\tdomain Domain_Name := ...domain_fields;\n"
                "\t                   ^^ -- expected operator there.\n"
                "But get:\n" + error_help
            ) {} 
    };

    class Expect_Fields: public Parsing_Exception {
        public:
            Expect_Fields(const std::string& error_help): Parsing_Exception(
                "Expected fields. Expected:\n"
                "Domain define syntax is:\n"
                "\tdomain Domain_Name := field(Domain)...;\n"
                "But get:\n" + error_help
            ) {}
    };
}

#endif // parser_hpp_INCLUDED

