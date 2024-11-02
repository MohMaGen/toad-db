#ifndef table_view_hpp_INCLUDED
#define table_view_hpp_INCLUDED
#include <common.hpp>
#include <type_traits>

namespace toad_db {
    /**
     * Table View.
     * View to interact with tables.
     **/
    template<typename _Table_Iter>
    class Table_View;

    /**
     * Make table view.
     * 
     * @param table
     * @return tbale view.
     **/
    Table_View<Table::Iter> make_table_view(Table& table);  

    /**
     * Make constant table view.
     * 
     * @param table
     * @return tbale view.
     **/
    Table_View<Table::Const_Iter> make_table_view(const Table& table);  

    /**
     * Make constant table view.
     * 
     * @param table
     * @return tbale view.
     **/
    Table_View<Table::Const_Iter> make_table_cview(Table& table);  

    template<typename _Table_Iter>
    class Table_View {
        static_assert(
            std::is_base_of<Table::Const_Iter, _Table_Iter>::value == true ||
            std::is_base_of<Table::Iter, _Table_Iter>::value == true );

        _Table_Iter _begin, _end;

        public:
            Table_View(_Table_Iter begin, _Table_Iter end): _begin(begin), _end(end) { }

            _Table_Iter begin(void) const { return _begin; }
            _Table_Iter end(void) const { return _end; }


        friend Table_View<Table::Iter> make_table_view(Table& table);  
        friend Table_View<Table::Const_Iter> make_table_view(const Table& table);  
        friend Table_View<Table::Const_Iter> make_table_cview(Table& table);  
    };

    std::ostream& operator<<(std::ostream& os, const Table_View<Table::Const_Iter>& view);
}

#endif
