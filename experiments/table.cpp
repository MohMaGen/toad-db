#include <common.hpp>
#include <iostream>


int main(void) {
    using namespace toad_db;
    using namespace types;

    auto domains = Domain::default_domains();

    std::cout << "======== Tables ========" << std::endl;

    [[maybe_unused]] Table table { { "name", domains("Str") }, { "age", domains("I32") } };

    Domain_Value name { &domains("Str") };
    Domain_Value age { &domains("I32") };
    name.view().set_string("Bob");
    age.view().set_basic<I32>(23); 

    table.insert_row({ name.view(), age.view() });

    name.view().set_string("Bbib");
    age.view().set_basic<I32>(32); 

    table.insert_row({ name.view(), age.view() });

    return 0;
}
