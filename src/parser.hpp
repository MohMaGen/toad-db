#ifndef parser_hpp_INCLUDED
#define parser_hpp_INCLUDED

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
            std::string table_name;

            struct Rule {
                std::string name;
            };

            struct Field {
                std::string name;
                std::string type;

                Rule gen_rule, display_rule;
                std::vector<Rule> check_rules; 
            };
            std::vector<Field> fields;
        };

        union {
        } data;
    };

    struct Execute_Tree {
        std::vector<toad_db::Domain> domain;
        std::vector<Top_Level_Statement> stmts;
    };

    Execute_Tree parse(const std::string &source);
}

#endif // parser_hpp_INCLUDED

