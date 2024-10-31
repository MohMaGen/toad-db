#include <common.hpp>
#include <cstddef>
#include <functional>
#include <iostream>


std::ostream& operator<<(std::ostream& os, std::function<void (std::ostream&)> fun) {
	fun(os);
	return os;
} 

int main(void) {
    using namespace toad_db;
    using namespace types;

    std::string indent = "";
    const auto endl = [&](std::ostream& os) {
        os << "\n" << indent;
        std::cout.flush();
    };
    const auto endlnf = [&](std::ostream& os) {
        os << "\n" << indent;
    };

    auto domains = Domain::default_domains();
    char data[0xFFFF] { '\0' };


    Domain_View
        fst { &domains("I32"), data },
        snd { &domains("I32"), data + sizeof(I32) };


    const auto print = [&]() {
        std::cout << "fst: " << to_string(fst) << ", snd: " << to_string(snd) << endl; 
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


    const auto print_person = [&](const Domain_View &person) {
        std::cout << "Person {" << endlnf
                        << "  name: " << display_string(person["name"]) << endlnf
                        << "  raw_name: " << to_string(person["name"]) << endlnf 
                        << "  age: " << to_string(person["age"]) << endlnf
                  << "}" << endl;
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

    domains.add({ "Persons", Domain::Array_Variant::Array, domains["Person"], 256}); 
    char *data2 = new char[domains("Persons").size_of()];

    Domain_View persons { &domains("Persons"), data2 };
    const auto print_persons = [&](const Domain_View &persons) {
        std::cout << "Persons " << persons.get_length() << " {" << endl;

        for (size_t i = 0; i < persons.get_length(); i++) {
            indent = "  ";
            std::cout << endl;
            print_person(persons[i]); 
            indent = "";
            std::cout << endl;
        }

        std::cout << "}" << endl;
    };

    print_persons(persons);

    persons.set_length(0); 
    persons.array_push(person); 

    print_persons(persons);

    delete[] data2;
	return 0;
}
