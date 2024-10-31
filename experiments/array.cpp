#include <common.hpp>
#include <ios>
#include <iostream>

int main() {
    using namespace toad_db;
    using namespace toad_db::types;

    auto domains = Domain::default_domains();
    domains.add({ "U8[10]", Domain::Array_Variant::Array, domains["U8"], 10 }); 

    char data[256] { '\0' };
    Domain_View view { &domains("U8[10]"), data };

    const auto print = [&]() {
        std::cout << " ============ print array ============" << std::endl;
        std::cout << to_string(view) << std::endl;
        std::cout << "len: " << view.get_length() << std::endl;
        std::cout << "[ ";
        for (size_t i = 0; i <= view.get_length(); i++) {
            std::cout << std::hex << std::showbase << std::uppercase 
                      << "'" << (int)data[i]
                      << std::dec << std::noshowbase << "' ";
        }
        std::cout << "]" << std::endl;
    };

    view.set_length(0); 
    print();

    view.array_push_basic<U8>(12); 
    print();

    view.array_push_basic<U8>(2); 
    print();

    view.array_push_basic<U8>(4); 
    print();

    view.array_push_basic<U8>(6); 
    print();

    view.array_push_basic<U8>(8); 
    print();

    view.array_push_basic<U8>(10); 
    print();

    view.array_pop();
    print();

    return 0;
}
