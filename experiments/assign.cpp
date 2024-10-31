#include <common.hpp>
#include <cstddef>
#include <iostream>


int main(void) {
    using namespace toad_db;
    using namespace types;

    auto domains = Domain::default_domains();
    char data[0xFFFF] { '\0' };


    Domain_View
        fst { &domains("I32"), data },
        snd { &domains("I32"), data + sizeof(I32) };


    const auto print = [&]() {
        std::cout << "fst: " << to_string(fst) << ", snd: " << to_string(snd) << std::endl; 
    };

    fst.set_basic<I32>(10); 
    snd.set_basic<I32>(20); 
    print();

    fst.assign(snd); 
    print();
    print();

	domains.add({ "String", Domain::Array_Variant::Array, domains["I8"], 0xFF });

    domains.add({"Person", Domain::Complex_Variant::Mul, { 
                    { "name", domains["String"] }, { "age", domains["U8"] }}});

    domains.add({"CS-Student", Domain::Complex_Variant::Mul, {
                    { "name", domains["String"] }, { "age", domains["U8"] },
                    { "Fav-Lang", domains["String"] }}});

    const auto display_string = [&](Domain_View string) -> std::string {
        std::string res { };
        for (size_t i = 0; i < string.get_length(); i++) {
            res.push_back(string[i].unwrap_basic<I8>());
        }
        return res;
    };

    const auto print_person = [&](Domain_View &person) {
        std::cout << "Person {\n"
                        << "  name: " << display_string(person["name"]) << "\n"
                        << "  raw_name: " << to_string(person["name"]) << "\n" 
                        << "  age: " << to_string(person["age"]) << "\n"
                  << "}\n";
    };

    Domain_View person { &domains("Person"), snd.data + sizeof(I32) + 10 };
    Domain_View cs_student {
        &domains("CS-Student"),
        person.data + domains("Person").size_of() + 1
    };



    std::string vlad = "Vlad";
    cs_student["name"].set_length(0); 
    for (auto c: vlad) cs_student["name"].array_push_basic<I8>(c); 
    cs_student["age"].set_basic<U8>(21); 

    print_person(person);
    print_person(cs_student);

	person.assign(cs_student);

    print_person(person);
    print_person(cs_student);

	return 0;
}
