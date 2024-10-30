#include <common.hpp>
#include <iostream>

int main() {
    using namespace toad_db::types;
    auto domains = toad_db::Domain::default_domains();

    for (auto domain: domains.data) {
        std::cout << 
            "name: " << domain.domain_name << 
            ", size: " << domain.size_of() << std::endl;
    }

    toad_db::Domain_Value value { &domains("Bool") };
    toad_db::Domain_View view { &domains("Bool"), &value };

    view.set_basic(Bool(false)); 
    view.unwrap_basic<Bool>() = true; 
    std::cout << "value is: `" << to_string(view) << "`." << std::endl;

    std::cout << "Mul is " << (int)(char)toad_db::Domain::Variant::Mul << std::endl;

    toad_db::Domain_Value date { &domains("Date") };
    view = { &domains("Date"), &date };

    view["day"].set_basic<U8>(26); 

    view["month"]["jan"];
    std::cout << "month: `" << to_string(view["month"]) << "`." << std::endl;

    view["year"].set_basic<U16>(2004);

    view["time"].set_basic<U32>(0);

    std::cout << "Time: " << to_string(view) << std::endl;

    using Variant = toad_db::Domain::Complex_Variant;
    domains.add({ "Vector2", Variant::Mul, {
                    { "x", domains["F32"] }, { "y", domains["F32"] }}}); 

    domains.add({ "Vector3", Variant::Mul, {
                    { "x", domains["F32"] }, { "y", domains["F32"] }, { "z", domains["F32"]}}}); 

    domains.add({ "Vector4", Variant::Mul, {
                    { "x", domains["F32"] }, { "y", domains["F32"] },
                    { "z", domains["F32"] }, { "w", domains["F32"] }}}); 

    domains.add({ "Vector", Variant::Add, {
                            { "v2", domains["Vector2"]},
                            { "v3", domains["Vector3"]},
                            { "v4", domains["Vector4"]}}});

    toad_db::Domain_Value vector { &domains("Vector") };
    view = { &domains("Vector"), &vector };
    view["v2"]["x"].set_basic<F32>(2.5);
    view["v2"]["y"].set_basic<F32>(4.5);

    std::cout << "vector: " << to_string(view) << std::endl;

    view["v3"]["x"].set_basic<F32>(2.5);
    view["v3"]["y"].set_basic<F32>(4.5);
    view["v3"]["z"].set_basic<F32>(6.5);
    std::cout << "vector: " << to_string(view) << std::endl;




    return 0;
}
