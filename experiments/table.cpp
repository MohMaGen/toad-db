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

    table.insert_row({ name.view(), age.view() });

    Domain_Value name2 = make_domain_value(&domains("Str"), "Vlas");
    table.insert_row(name2, age2); 

    return 0;
}
