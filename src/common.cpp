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

        auto row_data = table.data.begin();
        while (row_data != table.data.end()) {
            std::cout << indent << "row:" << std::endl;
            auto data = row_data.base();

            for (auto &column_field: table.columns_fields) {
                std::cout << indent << indent << column_field.name << ": ";

                std::cout << to_string(Domain_View { &column_field.domain, data  })
                                << std::endl;

                data += column_field.domain.size_of();
            }

            row_data += table.row_size;
        }


        return os;
    }
}
