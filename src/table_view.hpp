#ifndef table_view_hpp_INCLUDED
#define table_view_hpp_INCLUDED

#include <common.hpp>

namespace toad_db {
    template<typename _Iter>
    concept _Table_Iter = std::bidirectional_iterator<_Iter>;


    /**
     * Table View.
     * View to interact with tables.
     **/
    template<_Table_Iter _Iter>
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

    template<_Table_Iter _Iter>
    class Table_View {
        _Iter _begin, _end;

        public:
            Table_View(_Iter begin, _Iter end): _begin(begin), _end(end) { }

            _Iter begin(void) const { return _begin; }
            _Iter end(void) const { return _end; }


        friend Table_View<Table::Iter> make_table_view(Table& table);  
        friend Table_View<Table::Const_Iter> make_table_view(const Table& table);  
        friend Table_View<Table::Const_Iter> make_table_cview(Table& table);  
    };

    std::ostream& operator<<(std::ostream& os, const Table_View<Table::Const_Iter>& view);

    template<typename _Table_Iter>
    class Table_Mul_View {

    };
}

#endif
