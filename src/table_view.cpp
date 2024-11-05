//#include <ostream>
//#include <string>
#include <table_view.hpp>

namespace toad_db {
    std::ostream& operator<<(std::ostream& os, const Table_View<Table::Const_Iter>& view) {
        os << "Table View" << std::endl;

        for (auto field = view.begin()->begin(); field != view.begin()->end(); field++) {
            auto field_cp = field;
            os << field->domain->domain_name
                << (++field_cp != view.begin()->end() ? ";" : "");
        }
        os << std::endl;

        for (auto row: view) {
            for (auto field = row.begin(); field != row.end(); field++) {
                auto field_cp = field;
                os << to_string(*field)
                    << (++field_cp != row.end() ? ";" : "");
            }
            os << std::endl;
        }

        return os;
    }
    Table_View<Table::Iter> make_table_view(Table& table) {
        return Table_View<Table::Iter>(table.begin(), table.end()); 
    } 
    Table_View<Table::Const_Iter> make_table_view(const Table& table) {
        return Table_View<Table::Const_Iter>(table.begin(), table.end()); 
    }  
    Table_View<Table::Const_Iter> make_table_cview(Table& table) {
        return Table_View<Table::Const_Iter>(table.cbegin(), table.cend()); 
    }
}
