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

    /**
     * # The Top_Level_Statement
     *
     * Class that in fact tagged union of different possible
     * top level statements. [ Table Definition, Function Definition, Function Call ]
     *
     * The top level statements do not owns names
     * and literals. So you must strore original
     * string somewhere.
     */
    struct Top_Level_Statement {
        /**
         * Variant for tagged union.
         */
        enum Variant {
            /**
             * # Table definition.
             * 
             * The syntax:
             * 
             *    table <Table_Name> {
             *        field_name(Field_Domain): @display_rule,
             *        field_name2(Field_Domain2): @display_rule ?check_rule,
             *    };
             *
             * Example:
             *    table Person_Data {
             *         id(Key),
             *         name(Str): ?not_null ?unique,
             *         age(I8),
             *    };
             */
            Table_Define = 0, 

            /**
             * # Domain definition.
             * 
             * New domains can be one of three
             * types:  Alias, Mul, and Add.
             * The example:
             *    domain Alias_Domain := I32;
             * 
             *    domain Vector2 := x(F32) & y(F32);
             *    domain Vector3 := x(F32) & y(F32) & z(F32);
             *    domain Vector4 := x(F32) & y(F32) & z(F32) & w(F32);
             * 
             *    domain Vector  := v2(Vector2) | v3(Vector3) | v4(Vector4); 
             */
            Domain_Define,

            /**
             * # Function Definition.
             *
             * Function can be just top level global function. That
             * can be called like (function_name args...) --- than
             * define syntax would be following:
             *
             *    function function_name ( Type args,... ) -> Type
             *        body...;
             *
             * > info
             * > function arguments can be not only domains, but also
             * > of other types (like refs and function pointers).
             *
             * Also functions can be rules. Rule are assosiated with
             * domains and can be of different types.
             */
            Function_Define,


            /**
             * # Top level expression.
             * Example:
             *    let person_view = Person_Data with (name, age) in
             *    display person_view;
             */
            Call,

            /**
             * For type save error handling.
             * :)
             */
            None
        } variant;

        /**
         * # Table_Data.
         */
        struct Table_Data {
            /**
             * Name of the table.
             */
            std::string_view table_name;

            struct Field {
                /* The name of the field */
                std::string_view name;

                /* The domain fo the field */
                std::string_view type;

                /* vector of field's rules. */
                std::vector<std::string_view> rules; 
            };
            /*
             * Table's fields.
             */
            std::vector<Field> fields;
        };

        /**
         * # Domain_Data
         *
         * Domain's definitions can be one of thre
         * kinds: Alias, Mul, and Add. This kinds
         * represented in Domain_Data::Variant enumeration.
         */
        struct Domain_Data {
            /**
             * Name of the domain.
             * 
             * It's suggested to use domain names
             * in combined camal Pascal case. like this:
             * `Domain_Name` or `My_Domain`.
             */
            std::string_view domain_name;

            enum Variant {
                /**
                 * Alias domain.
                 * Can be only for basic types.
                 * example:
                 *     domain Key := U64;
                 */
                Alias,

                /**
                 * Multiplication domain.
                 * Fields must have names.
                 *
                 * example:
                 *     domain Vec2 := x(F32) & x(F32);
                 */
                Mul,

                /**
                 * Addition domain.
                 * Some fields may not have domain names.
                 * example:
                 *     domain Shape := none | cirle(Circle) | square(Square);
                 */
                Add
            } variant;

            struct Field {
                /* Name of domains fields */
                std::string_view name;
                /* Name of domain fo the field */
                std::string_view domain;
            };
            /**
             * Fields. In case of Alias domain.
             * Fields will consist only of one field
             * with empty `.domain` adn with `.name`
             * equals to actual domain of this alias.
             */
            std::vector<Field> fields;
        };

        /**
         * # Expression_Data
         * data of expression.
         *
         * This is tree of function and applyed arguments.
         * .root = (func arg1 arg2 arg3)
         */
        struct Expression_Data {

            /**
             * Node value.
             */
            struct Expression_Node {
                /**
                 * pointer to the node.
                 */
                using pointer = std::shared_ptr<Expression_Node>; 


                /**
                 * Kind of the node.
                 */
                enum Kind {
                    /**
                     * Can be both: name of the function or
                     * name of the variable.
                     */
                    Name = 0,

                    /**
                     * Literals (Str_Literal, Char_Literal, Num_Literal)
                     * used to reprsent literals like: 12312 '123123' "23123"
                     */
                    Str_Literal, Char_Literal, Num_Literal,

                    /**
                     * Operators.
                     */
                    Operator, Bound_Operator,

                    /**
                     * Kind of node that just only one argument and have no names.
                     * It's basicly the wrapper. Equvalent to (expr).
                     */
                    Expression,

                    /**
                     * For error handling.
                     */
                    Err,
                } kind;

                /**
                 * Name of the called function or operator.
                 *
                 * In case of .kind = Expression will just empty.
                 * It's not guarateed that this empty view will
                 * be at original string.
                 */
                std::string_view name;

                /**
                 * The args applyed to the function or operator [to the name].
                 */
                std::vector<pointer> args;

                /**
                 * Build node with no arguments.
                 */
                Expression_Node(Kind kind, std::string_view name):
                    kind(kind), name(name), args({}) {}

                /**
                 * Build node with provided arguments.
                 */
                Expression_Node(Kind kind, std::string_view name, std::vector<pointer> args):
                    kind(kind), name(name), args(args) {}
            };

            /* Node kind using. */
            using kind = Expression_Node::Kind;

            /* Node's pointer using. */
            using pointer = Expression_Node::pointer;

            /**
             * Node's pointer constructor.
             */
            template<typename... _Args>
            static pointer make_pointer(_Args... args) {
                return std::make_shared<Expression_Node>(Expression_Node(args...));
            }


			/*
            static pointer most_right(pointer node) {
                if (node->args.size() == 0) return node;
                return most_right(*node->args.rbegin());
            }
            */

            /**
             * The root of expression tree.
             */
            pointer root; 

            /**
             * Construct empty expression tree.
             */
            Expression_Data() {
                root = make_pointer(
                    kind::Expression, 
                    std::string_view { }
                );
            }

            /**
             * Construct expression tree from root element.
             */
            Expression_Data(pointer ptr) {
                root = make_pointer(
                    kind::Expression, 
                    std::string_view{ ptr->name.begin(), ptr->name.begin() },
                    std::vector{ ptr }
                );
            }
        };

        /**
         * Union part of tagged union.
         * Which of variants is used defined by .variant.
         */
        union {
            Table_Data table_data;
            Domain_Data domain_data;
            Expression_Data call_data;
        };

        /**
         * Default statement with None variant.
         */
        Top_Level_Statement(): variant(None) { }

        /**
         * Copy constructor.
         */
        Top_Level_Statement(const Top_Level_Statement& v): variant(v.variant) {
            switch (v.variant) {
            case Table_Define: std::construct_at(&table_data, v.table_data); break;
            case Domain_Define: std::construct_at(&domain_data, v.domain_data); break; 
            case Function_Define:
            case Call: std::construct_at(&call_data, v.call_data); break; 
            case None:
            }
        }

        /**
         * Move constructor.
         */
        Top_Level_Statement(const Top_Level_Statement&& v): variant(v.variant) {
            switch (v.variant) {
            case Table_Define: std::construct_at(&table_data, std::move(v.table_data)); break;
            case Domain_Define: std::construct_at(&domain_data, std::move(v.domain_data)); break; 
            case Function_Define:
            case Call: std::construct_at(&call_data, std::move(v.call_data)); break;
            case None:
            }
        }

        /**
         * Construct statement from Table_Data [would be Table_Define variant].
         */
        Top_Level_Statement(const Table_Data &data): variant(Table_Define) {
            std::construct_at(&table_data, data);
        }

        /**
         * Construct statement from Domain_Data [would be Domain_Define variant].
         */
        Top_Level_Statement(const Domain_Data &data): variant(Domain_Define) {
            std::construct_at(&domain_data, data);
        }

        /**
         * Construct statement from Expression_Data [would be Call variant].
         */
        Top_Level_Statement(const Expression_Data &data): variant(Call) {
            std::construct_at(&call_data, data);
        }

        /**
         * Destructor
         */
        ~Top_Level_Statement() {
            switch (variant) {
            case Table_Define: table_data.~Table_Data(); break;
            case Domain_Define: domain_data.~Domain_Data(); break;
            case Function_Define:
            case Call: call_data.~Expression_Data(); break;
            case None:
            }
        }
    };

    std::string to_string(Top_Level_Statement::Variant);
    std::string to_string(const Top_Level_Statement::Expression_Data& data);

    struct Syntax_Tree {
        std::string content;
        std::vector<Top_Level_Statement> stmts;

        struct Operator {
            std::string name;
            size_t order;
        };

        std::vector<Operator> operators = {
            { "with", 5 },
            { "==", 1 }, { "!=", 1 }, { "<=", 1 }, { ">=", 1 }, { ":=", 0 },
            { "**", 5 }, { "as", 5 },
            { "@", 6 },
            { "+", 3 }, { "-", 3 }, { "*", 4 }, { "/", 4 }, { "^", 5 }, { "=", 0 },
            { ">", 1}, { "<", 1 }, 
        };

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


        std::vector<Bound_Operator> bound_operators = {
            {{{"if", Bound_Operator::Once}, {"then", Bound_Operator::Once}, {"else", Bound_Operator::Once}}},
            {{{"let", Bound_Operator::Once}, {"in", Bound_Operator::Once}}},
            {{{"[", Bound_Operator::Once}, {",", Bound_Operator::Multiple}, {"]", Bound_Operator::Close}}},
            {{{"(", Bound_Operator::Once}, {",", Bound_Operator::Multiple}, {")", Bound_Operator::Close}}},
            {{{"<", Bound_Operator::Once}, {",", Bound_Operator::Multiple}, {">", Bound_Operator::Close}}},
            {{{"{", Bound_Operator::Once}, {",", Bound_Operator::Multiple}, {"}", Bound_Operator::Close}}},
        };

        Top_Level_Statement::Variant read_variant(std::string_view stmt);

        Top_Level_Statement::Table_Data parse_table(std::string_view view);

        Top_Level_Statement::Domain_Data parse_domain(std::string_view view);

        Top_Level_Statement::Expression_Data parse_call(std::string_view view);

        static std::unique_ptr<Syntax_Tree> parse(const std::string &source);

        private:
            std::pair<std::string_view, Top_Level_Statement::Expression_Data::kind>
                read_literal(std::string_view view);

            std::string_view read_operator(std::string_view view);

            Operator get_operator(std::string_view view);

            std::string_view read_node(std::string_view view, const Bound_Operator &op, size_t &idx);

            std::vector<std::string_view> read_bound_operator(std::string_view view);
    };


    std::ostream& operator<<(std::ostream& os, const Syntax_Tree& tree);


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


    class Expected_Left_Operand: public Parsing_Exception {
        public:
            Expected_Left_Operand(std::string op, std::string error_help): Parsing_Exception(
                "Expect left operand of operator `" + op + "`.\n"
                "Get:\n" + error_help
            ) {  }
    };

    class Expected_Right_Operand: public Parsing_Exception {
        public:
            Expected_Right_Operand(std::string op, std::string error_help): Parsing_Exception(
                "Expect right operand of operator `" + op + "`.\n"
                "Get:\n" + error_help
            ) {  }
    };

    class Unexpected_Call: public Parsing_Exception {
        public:
            Unexpected_Call(std::string error_help): Parsing_Exception(
                "Unexpected chars in function:\n" + error_help
            ) {} 
    };


    class Expected_Bound_Operator_Node_Expr: public Parsing_Exception {
        public:
            Expected_Bound_Operator_Node_Expr(std::string node_name, std::string help_error): Parsing_Exception(
                "Expected expression of right hand of operator node `" + node_name + "`;\n"
                "But get:\n" + help_error
            ) {}
    };
}

#endif // parser_hpp_INCLUDED

