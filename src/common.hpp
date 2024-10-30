#ifndef __COMMON_GUARS_H__
#define __COMMON_GUARS_H__

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <span>
#include <stdexcept>
#include <vector>


namespace toad_db {

    /**
     * Using for basic types for better connection with language.
     **/
    namespace types {
        using U8  = std::uint8_t;
        using U16 = std::uint16_t;
        using U32 = std::uint32_t;
        using U64 = std::uint64_t;
        using I8  = std::int8_t;
        using I16 = std::int16_t;
        using I32 = std::int32_t;
        using I64 = std::int64_t;
        using F32 = float;
        using F64 = double;
        using Bool = bool;

        using Data_Slice = std::span<char, std::dynamic_extent>;
    }

	/**
     * Base class for all exception in the library.
     **/
    class Toad_Exception: public std::runtime_error {
        public:
            Toad_Exception(const std::string &reason):
                std::runtime_error("Toad: `" + reason + "`.") { }
    };

    struct Domain {
        /**
         * Name of the domain, it's way to identify domen.
         **/
        using Domain_Idx = std::size_t;

        /**
         * Collection of all domens;
         **/
        struct Domains {
            std::vector<Domain> data;

            Domains(const std::vector<Domain>& data): data{data} {
                for (auto &domain : this->data) {
                    domain.domains = this;
                }
            }

            /**
             * Throws if try to get domain of invalid (unregisered name).
             **/
            class Invalid_Domain_Name: public Toad_Exception {
                public:
                    Invalid_Domain_Name(const std::string& name):
                        Toad_Exception("Failed to get domain with name `" + name + "`") { } 
            };
            class Domain_Idx_Out_Of_Range: public Toad_Exception {
                public:
                    Domain_Idx_Out_Of_Range(Domain_Idx idx, size_t size):
                        Toad_Exception("Domain idx out of range. (idx = " +
                                        std::to_string(idx) + ", size = " +
                                        std::to_string(size) + ")") {}
            };

            /**
             * Get domain idx by it's name.
             *
             * @param name - name of the domain to get.
             * @return idx of the domain.
             * 
             * @throws Invalid_Domen_Name if there is no such domain
             **/
            Domain_Idx by_name(const std::string& name) const noexcept(false) {
                auto it = std::find_if(data.begin(), data.end(), 
                            [=](const auto &domain) { 
                                return domain.domain_name == name;  
                            });

                if (it == data.end()) throw Invalid_Domain_Name(name);

                return it - data.begin();
            }

            Domain_Idx operator[](const std::string& name) const noexcept(false) {
                return by_name(name);
            }

            Domain& operator[](Domain_Idx idx) noexcept(false) {
                if (idx > data.size()) throw Domain_Idx_Out_Of_Range(idx, data.size());
                return data[idx];
            }

            const Domain& operator[](Domain_Idx idx) const noexcept(false) {
                if (idx > data.size()) throw Domain_Idx_Out_Of_Range(idx, data.size());
                return data[idx];
            }
            Domain& operator()(const std::string& name) noexcept(false) {
                return data[by_name(name)];
            }

            const Domain& operator()(const std::string& name) const noexcept(false) {
                return data[by_name(name)];
            }

            void add(const Domain& domain) {
                Domain cp_domain = domain;
                cp_domain.domains = this;
                data.push_back(cp_domain);
            }
        } *domains;

        /**
         * Name of the domain.
         **/
        std::string domain_name;

        /**
         * # Variant of contained domain .
         *
         * - Just basic domains: U8, U16, U32, U64, I8, I16, I32, I64, F32, F64.
         * 
         * - Alias: variant of alias domain used for domains like ```domain Day = U8;```.
         *   value of alias contains variant and data.
         *
         * - Array: represent array of values of specific domain.
         *
         * - Algebraic_Domen: domain representing sum and or multiplication of other domains.
         * 	 Like in __Haskell__.
         **/
        enum class Variant: char { 
            U8=0, U16, U32, U64, I8, I16, I32, I64, F32, F64, Bool,
            Array, Add, Mul
        } variant; 

        friend std::string to_string(Variant variant) {
            switch (variant) {
            case Domain::Variant::U8:    return "U8";
            case Domain::Variant::U16:   return "U16";
            case Domain::Variant::U32:   return "U32";
            case Domain::Variant::U64:   return "U64";
            case Domain::Variant::I8:    return "I8";
            case Domain::Variant::I16:   return "I16";
            case Domain::Variant::I32:   return "I32";
            case Domain::Variant::I64:   return "I64";
            case Domain::Variant::F32:   return "F32";
            case Domain::Variant::F64:   return "F64";
            case Domain::Variant::Bool:  return "Bool";
            case Domain::Variant::Array: return "Array";
            case Domain::Variant::Add:   return "Add";
            case Domain::Variant::Mul:   return "Mul";
            default:
                throw Domain::Invalid_Variant_Value(variant);
            }
        }
        friend std::ostream& operator<<(std::ostream& os, Domain::Variant variant);

        /**
         * Get basic variant for type.
         **/
        template<typename Type> static constexpr Variant type2variant();

        template<> constexpr Variant type2variant<types::U8>() { return Variant::U8; }
        template<> constexpr Variant type2variant<types::U16>() { return Variant::U16; }
        template<> constexpr Variant type2variant<types::U32>() { return Variant::U32; }
        template<> constexpr Variant type2variant<types::U64>() { return Variant::U64; }

        template<> constexpr Variant type2variant<types::I8>() { return Variant::I8; }
        template<> constexpr Variant type2variant<types::I16>() { return Variant::I16; }
        template<> constexpr Variant type2variant<types::I32>() { return Variant::I32; }
        template<> constexpr Variant type2variant<types::I64>() { return Variant::I64; }

        template<> constexpr Variant type2variant<types::F32>() { return Variant::F32; }
        template<> constexpr Variant type2variant<types::F64>() { return Variant::F64; }
        template<> constexpr Variant type2variant<types::Bool>() { return Variant::Bool; }

        friend constexpr bool is_basic(Variant variant) {
            if ((char)variant < (char)Variant::Array) return true;
            return false;
        }

        friend constexpr bool is_array(Variant variant) {
            if ((char)variant == (char)Variant::Array) return true;
            return false;
        }

        friend constexpr bool is_complex(Variant variant) {
            if ((char)variant > (char)Variant::Array) return true;
            return false;
        }

        /**
         * Get size in bytes of the counter.
         * 
         * There counter is the any value representing count, or length, or size
         * of smth.
         * 
         * @param max_value - max value that counter can be.
         * @return size of the counter.
         **/
        static constexpr size_t counter_size_of(size_t max_value) {
            using namespace types;

            if (max_value <= 0xFF)
                return sizeof(U8);

            if (max_value <= 0xFFFF) 
                return sizeof(U16);

            return sizeof(U32);
        }

        /**
         * Set value of the counter int data array.
         * @see counter_size_of
         *
         * @param data - data at start which counter is destinated.
         * @param max_value - max value that counter can be.
         * @param value - new value of the counter.
         **/
        static constexpr void set_counter(char *data, size_t max_value, size_t value) {
            using namespace types;

            if (max_value <= 0xFF)           *(U8*)data = (U8)value;
            else if (max_value <= 0xFFFF)    *(U16*)data = (U16)value;
            else                             *(U32*)data = (U32)value;
        }

        /**
         * Get value of the counter int data array.
         * @see counter_size_of
         *
         * @param data - data at start which counter is destinated.
         * @param max_value - max value that counter can be.
         * @return value of the counter.
         **/
        static constexpr size_t get_counter(char *data, size_t max_value) {
            using namespace types;

            if (max_value <= 0xFF)           return (size_t)*(U8*)data;
            else if (max_value <= 0xFFFF)    return (size_t)*(U16*)data;
            else                             return (size_t)*(U32*)data;
        }


        /**
         * Field of the complex domain (Mul or Add).
         **/
        struct Field {
            std::string field_name;
            bool has_type;
            Domain_Idx domain_idx;

            Field(std::string field_name, Domain_Idx domain_idx):
                field_name(field_name), has_type(true), domain_idx(domain_idx) { }

            Field(std::string field_name):
                field_name(field_name), has_type(false), domain_idx(0) { }

        };

        /**
         * Data for Array variant.
         **/
        struct Array_Data {
            types::U64 capacity;
            Domain_Idx idx;
        };

        /**
         * Data of complex or array domains representing additional value
         * of the domain like len or fields.
         **/
        union {
            Array_Data array;                   // for Variant::Array.
            std::vector<Field> complex_fields;  // For Variant::Mul or Variant::Add.
        };


        /**
         * Subset of Domain's variants that represents basic domains.
         **/
        enum class Basic_Variants: char {
            U8=0, U16, U32, U64, I8, I16, I32, I64, F32, F64, Bool
        };

        /**
         * Subset of Domain's variants that represents Complex domain (Add or Mul).
         **/
        enum class Complex_Variant: char {
            Add=0, Mul
        };

        /**
         * Subset of Domain's variants that represents arrays.
         **/
        enum class Array_Variant: char {
            Array=0
        };

        /**
         * Construct basic domain or alias to a basic domain.
         * 
         * @param name - name of the domain.
         * @param basic_variant - variant of the domain.
         **/
        Domain(const std::string &name, Basic_Variants basic_variant) {
            domain_name = name;
            variant     = Variant((char)basic_variant);
        }

        /**
         * Construct complex domain.
         *
         * @param name - name of the domain.
         * @param complex_variant - variants of complex domain (Add or Mul).
         * @param field - vector of the fields.
         **/
        Domain(const std::string &name, Complex_Variant complex_variant,
                                        const std::vector<Field>& fields) {
            domain_name    = name;
            variant        = Variant((char)Variant::Add + (char)complex_variant);

            std::construct_at(&complex_fields, fields); 
        }

        /**
         * Construct complex domain.
         *
         * @param name - name of the domain.
         * @param complex_variant - variants of complex domain (Add or Mul).
         * @param field - vector of the fields.
         **/
        Domain(const std::string &name, Array_Variant array_variant,
                                        Domain_Idx elem_idx, size_t len) {
            domain_name = name;
            variant     = Variant((char)Variant::Array + (char)array_variant);
            array.capacity   = len;
            array.idx   = elem_idx;
        }

        Domain(const Domain& domain):
            domains(domain.domains),
            domain_name(domain.domain_name),
            variant(domain.variant)
        {
            if (is_complex(variant)) {
                std::construct_at(&complex_fields, domain.complex_fields); 
            } else {
                array = domain.array;
            }
        }


        /**
         * Init domains with default
         **/
        static Domains default_domains(void) {
            Domains domains {{
                /*****************************
                 *      NUMERIC DOMENS       *
                 *****************************/
                Domain { "U8",  Basic_Variants::U8  },
                Domain { "U16", Basic_Variants::U16 },
                Domain { "U32", Basic_Variants::U32 },
                Domain { "U64", Basic_Variants::U64 },
                Domain { "Key", Basic_Variants::U64 },

                Domain { "I8",  Basic_Variants::I8  },
                Domain { "I16", Basic_Variants::I16 },
                Domain { "I32", Basic_Variants::I32 },
                Domain { "I64", Basic_Variants::I64 },

                Domain { "F32", Basic_Variants::F32 },
                Domain { "F64", Basic_Variants::F64 },

                Domain { "Bool", Basic_Variants::Bool },

                /*******************************
                 *        Time and Date        *
                 *******************************/

                Domain { "Month", Complex_Variant::Add, {
                    {"jan"}, {"feb"}, {"mar"}, {"apr"}, 
                    {"may"}, {"jul"}, {"jun"}, {"aug"}, 
                    {"sep"}, {"oct"}, {"nov"}, {"dec"}}},

                Domain { "Day",        Basic_Variants::U8  },
                Domain { "Year",       Basic_Variants::U16 },
                Domain { "Seconds",    Basic_Variants::U32 },
                Domain { "Time_Stamp", Basic_Variants::U64 },
            }};


            domains.add({ "Date", Complex_Variant::Mul, {
                        { "day", domains["Day"]},
                        { "month", domains["Month"]},
                        { "year", domains["Year"]},
                        { "time", domains["Seconds"]}}});

            return domains;
        }


        class Invalid_Variant_Value: public Toad_Exception {
            public:
                Invalid_Variant_Value(Variant variant):
                    Toad_Exception("Somehow invalid value of the Domains's variant." 
                                    + to_string(variant)) { }
        };

        /**
         * Returns size of the types value.
         **/
        size_t size_of(void) const noexcept(false) {
            using namespace types;

            switch (variant) {

            case Variant::U8:   return sizeof(U8);
            case Variant::U16:  return sizeof(U16);
            case Variant::U32:  return sizeof(U32);
            case Variant::U64:  return sizeof(U64);
            case Variant::I8:   return sizeof(I8);
            case Variant::I16:  return sizeof(I16);
            case Variant::I32:  return sizeof(I32);
            case Variant::I64:  return sizeof(I64);
            case Variant::F32:  return sizeof(F32);
            case Variant::F64:  return sizeof(F64);
            case Variant::Bool: return sizeof(Bool); 

            case Variant::Array: {
                size_t size_of_elem = (*domains)[array.idx].size_of();
                return size_of_elem * array.capacity + counter_size_of(array.capacity);
            }

            case Variant::Add: {
                size_t size = 0;
                for (auto &field : complex_fields) {
                    if (!field.has_type) continue;

                    size_t curr = (*domains)[field.domain_idx].size_of();
                    size = curr > size ? curr : size;
                }
                const size_t fields_count = complex_fields.size();
                size += counter_size_of(fields_count); 

                return size;
            }

            case Variant::Mul: {
                size_t size = 0; 

                for (auto field : complex_fields) {
                    size += (*domains)[field.domain_idx].size_of();
                }

                return size;
            }

            default:
                throw Domain::Invalid_Variant_Value(variant);
            }
        }

        ~Domain() {
        }
    };



    /**
     * Contains value of specific domain.
     **/
    struct Domain_Value {
        Domain_Value(Domain* domain) {
            size = domain->size_of();

            if (size > 8) {
                big_data = (char*)std::malloc(size);
            }
        }

        char* data() {
            if (size > 8) {
                return big_data;
            } else {
                return small_data.data();
            }
        }
        private:
        size_t size;
        union {
            std::array<char, 8> small_data { 0 };
            char* big_data;
        };
    };


    /**
     * Interface to interact with domain values.
     * Look at the array of chars and operate them like domain's data.
     * @see Domain_Value.
     *
     * ```cpp
     * auto domains = toad_db::Domain::default_domains();
     * 
     * Domain_Value value;
     *
     * Domain_View view  { &domains("I32"), &value };
     * view.set_basic<I32>(10);
     *
     * std::cout << view.unwrap_basic<I32>(); // return 10;
     * view.unwrap_basic<I32>() = 20;
     * 
     * std::cout << view.unwrap_basic<I32>(); // return 20;
     *
     * view.unwrap_basic<U32>(); // throws Unwrap_Invalid_Variant;
     * ```
     **/
    struct Domain_View {
        Domain *domain;
        char *data;

        Domain_View(Domain *domain, Domain_Value *domain_value):
            domain(domain), data(domain_value->data()) {}

        Domain_View(Domain *domain, char *data):
            domain(domain), data(data) { }

        /**
         * Try to unwrap wrong variant.
         **/
        class Unwrap_Invalid_Variant: public Toad_Exception {
            public:
                Unwrap_Invalid_Variant(Domain::Variant actual, Domain::Variant expected):
                    Toad_Exception(std::string("Unwrap invalid exception:") 
                                        + " given variant " + to_string(actual)
                                        + " excpected " + to_string(expected)) { }
        };

        /**
         * Throws when try to use non complex domen like complex.
         **/
        class Variant_Has_Not_Fields: public Toad_Exception {
            public:
                Variant_Has_Not_Fields(Domain::Variant variant):
                    Toad_Exception("Try to get field of non complex domain variant. ("
                                        + to_string(variant) + ")") { }
        };

        /**
         * When try to get wrong field with wrong name.
         **/
        class Domain_Has_Not_Such_Field: public Toad_Exception {
            public:
                Domain_Has_Not_Such_Field(const std::string name):
                    Toad_Exception("Domain has not such field: `" + name + "`") { } 
        };

        /**
         * Try to get variant not from Add variant.
         **/
        class Access_Add_Variant_Not_From_Add_Domen: public Toad_Exception {
            public:
                Access_Add_Variant_Not_From_Add_Domen(Domain::Variant variant):
                    Toad_Exception("Try to get variant not from Add variant. ("
                                        + to_string(variant) + ")") {} 
        };

        /**
         * Unwrap and return casted reference to the value.
         *
         * @return value;
         * @throws `Unwrap_Invalid_Variant` if given incorect type or value is not basic type.
         **/
        template<typename Type>
        Type& unwrap_basic() noexcept(false) {
            if (Domain::type2variant<Type>() != domain->variant)
                throw Unwrap_Invalid_Variant(Domain::type2variant<Type>(),  domain->variant);

            return *(Type*)data;
        }

        /**
         * Unwrap and return casted reference to the value.
         *
         * @return value;
         * @throws `Unwrap_Invalid_Variant` if given incorect type or value is not basic type.
         **/
        template<typename Type>
        const Type& unwrap_basic() const noexcept(false) {
            if (Domain::type2variant<Type>() != domain->variant)
                throw Unwrap_Invalid_Variant(Domain::type2variant<Type>(),  domain->variant);

            return *(Type*)data;
        }

        /**
         * Unwrap and set new value to the domen value.
         *
         * @param value - new value.
         * @throws `Unwrap_Invalid_Variant` if given incorect type or value is not basic type.
         **/
        template<typename Type>
        void set_basic(Type value) noexcept(false) {
            if (Domain::type2variant<Type>() != domain->variant)
                throw Unwrap_Invalid_Variant(Domain::type2variant<Type>(),  domain->variant);

            *(Type*)data = value;
        }

        /**
         * Get Domain_View of the field by field name.
         **/
        Domain_View operator[](const std::string &name) {
            if (!is_complex(domain->variant)) throw Variant_Has_Not_Fields(domain->variant);

            size_t offset = 0;
            size_t idx = 0;
            for (auto &field: domain->complex_fields) {
                Domain *field_domain = &(*domain->domains)[field.domain_idx];
                offset += field_domain->size_of();

                if (field.field_name == name) {
                    if (domain->variant == Domain::Variant::Add) {
                        const size_t fields_count = domain->complex_fields.size();
                        Domain::set_counter(data, fields_count, idx);
                    }

                    return Domain_View { field_domain, data + offset, };
                }
                idx++;
            }

            throw Domain_Has_Not_Such_Field(name);
        }

        /**
         * Try to get field with idx bigger or equal to size of fields vector.
         **/
        class Field_Idx_Out_Of_Range: public Toad_Exception {
            public:
                Field_Idx_Out_Of_Range(size_t idx, size_t size):
                    Toad_Exception(std::string("Field idx out of range.") 
                                        + " idx: " + std::to_string(idx)
                                        + " size: " + std::to_string(size)) { } 
        };
        /**
         * Try to get array value at idx bigger or equal to len of an array.
         **/
        class Array_Idx_Out_Of_Range: public Toad_Exception {
            public:
                Array_Idx_Out_Of_Range(size_t idx, size_t len):
                    Toad_Exception(std::string("Array idx out of range.") 
                                        + " idx: " + std::to_string(idx)
                                        + " len: " + std::to_string(len)) { } 
        };

        Domain_View operator[](size_t idx) noexcept(false) {
            if (!is_complex(domain->variant) && !is_array(domain->variant))
                throw Variant_Has_Not_Fields(domain->variant);

            if (is_complex(domain->variant)) {
                if (idx >= domain->complex_fields.size())
                    throw Field_Idx_Out_Of_Range(idx, domain->complex_fields.size());

                return operator[](domain->complex_fields[idx].field_name);
            } 

            if (is_array(domain->variant)) {
                size_t len = Domain::get_counter(data, domain->array.capacity);
                if (idx >= len)
                    throw Array_Idx_Out_Of_Range(idx, len);

                Domain* elem_domain = &(*domain->domains)[domain->array.idx];
                size_t elem_size = elem_domain->size_of();

                return Domain_View {
                    elem_domain,
                    data + Domain::counter_size_of(domain->array.capacity)
                         + elem_size * idx
                };
            }

            throw Domain::Invalid_Variant_Value(domain->variant);
        }

        /**
         * Get variant of Add Domain.
         **/
        size_t get_variant(void) {
            if (domain->variant != Domain::Variant::Add)
                throw Access_Add_Variant_Not_From_Add_Domen(domain->variant);

            size_t size = domain->size_of();
            size_t variant = 0;

            if (size <= 0xFF) {
                variant = *(types::U8*)data;
            } else if (size <= 0xFFFF) {
                variant = *(types::U16*)data;
            }

            return variant;
        }

        friend std::string to_string(Domain_View view) {
            if (is_basic(view.domain->variant)) {

                switch (view.domain->variant) {
                case Domain::Variant::U8:
                    return view.domain->domain_name
                            + "(" + std::to_string(view.unwrap_basic<types::U8>()) + ")"; 
                case Domain::Variant::U16:
                    return view.domain->domain_name
                            + "(" + std::to_string(view.unwrap_basic<types::U16>()) + ")"; 
                case Domain::Variant::U32:
                    return view.domain->domain_name
                            + "(" + std::to_string(view.unwrap_basic<types::U32>()) + ")"; 
                case Domain::Variant::U64:
                    return view.domain->domain_name
                            + "(" + std::to_string(view.unwrap_basic<types::U64>()) + ")"; 
                case Domain::Variant::I8:
                    return view.domain->domain_name
                            + "(" + std::to_string(view.unwrap_basic<types::I8>()) + ")"; 
                case Domain::Variant::I16:
                    return view.domain->domain_name
                            + "(" + std::to_string(view.unwrap_basic<types::I16>()) + ")"; 
                case Domain::Variant::I32:
                    return view.domain->domain_name
                            + "(" + std::to_string(view.unwrap_basic<types::I32>()) + ")"; 
                case Domain::Variant::I64:
                    return view.domain->domain_name
                            + "(" + std::to_string(view.unwrap_basic<types::I64>()) + ")"; 
                case Domain::Variant::F32:
                    return view.domain->domain_name
                            + "(" + std::to_string(view.unwrap_basic<types::F32>()) + ")"; 
                case Domain::Variant::F64:
                    return view.domain->domain_name
                            + "(" + std::to_string(view.unwrap_basic<types::F64>()) + ")"; 
                case Domain::Variant::Bool:
                    return view.domain->domain_name
                            + "("
                            + std::string(view.unwrap_basic<types::Bool>() ? "true" : "false")
                            + ")"; 
                default:
                    throw Toad_Exception("Unreachable!");
                }

            }
            if (is_array(view.domain->variant)) {
                size_t len = Domain::get_counter(view.data, view.domain->array.capacity); 

                std::string ret = view.domain->domain_name
                    + " " + std::to_string(view.domain->array.capacity)
                    + ":" + std::to_string(len) + " [ ";

                for (types::U64 i = 0; i < len; i++) {
                    ret += to_string(view[i]) + (i != len -1 ? ", " : "");
                }

                return ret + " ]";
            }

            if (view.domain->variant == Domain::Variant::Add) {
                size_t variant = view.get_variant();
                Domain::Field field = view.domain->complex_fields[variant];

                return view.domain->domain_name + "::" + field.field_name
                        + (field.has_type ? "(" + to_string(view[field.field_name]) + ")" : "");
            }

            if (view.domain->variant == Domain::Variant::Add) {
                size_t variant = view.get_variant();
                Domain::Field field = view.domain->complex_fields[variant];

                if (field.has_type) {
                    return field.field_name + "(" + to_string(view[field.field_name]) + ")";
                } else {
                    return field.field_name;
                }
            }

            if (view.domain->variant == Domain::Variant::Mul) {
                std::string ret = view.domain->domain_name + "{ ";

                for (auto &field: view.domain->complex_fields) {
                    ret += field.field_name + ": " + to_string(view[field.field_name]) + ", ";
                }

                return ret + "}";
            }

            throw Domain::Invalid_Variant_Value(view.domain->variant);
        }
    };


    class Table {
        std::vector<Domain> columns_domains;
        std::vector<Domain_Value> data;
    };
}

#endif
