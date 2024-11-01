#include <common.hpp>

namespace toad_db {
    std::ostream& operator<<(std::ostream& os, Domain::Variant variant) {
        return os << to_string(variant);
    }

    Table& operator<<(Table& table, const std::vector<Domain_View> &row_value) {
        table.insert_row(row_value); 
        return table;
    }
}
