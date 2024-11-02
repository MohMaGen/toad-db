#include <common.hpp>
#include <iostream>
//#include <ncurses.h>

namespace toad_db {
    std::ostream& operator<<(std::ostream& os, Domain::Variant variant) {
        return os << to_string(variant);
    }

    Table& operator<<(Table& table, const std::vector<Domain_View> &row_value) {
        table.insert_row(row_value); 
        return table;
    }

    std::ostream& operator<<(std::ostream& os, const Table& table) {
        std::cout << "Table:" << std::endl;
        const std::string indent = "   ";

        for (auto &row: table) {
            std::cout << indent << "row:" << std::endl;

            for (auto field: row) {
                std::cout << indent << indent << to_string(field) << std::endl;
            }
            std::cout << std::endl;
        }

        return os;
    }
}
