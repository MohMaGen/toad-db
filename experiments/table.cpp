#include "table_view.hpp"
#include <common.hpp>
#include <iostream>

int main(void) {
    using namespace toad_db;
    using namespace types;

    auto domains = Domain::default_domains();

    [[maybe_unused]] Table table { { "name", &domains("Str") }, { "age", &domains("I32") } };

    Domain_Value name { &domains("Str") }; Domain_Value age { &domains("I32") };

    name.view().set_string("Vlad"); age.view().set_basic<I32>(10);
    table.insert_row({ name.view(), age.view() });

    name.view().set_string("Vova"); age.view().set_basic<I32>(12);  
    table.insert_row({ name.view(), age.view() });

    std::cout << table << std::endl;

    std::cout << make_table_cview(table); 

    return 0;
}
