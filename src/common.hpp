#ifndef __COMMON_GUARS_H__
#define __COMMON_GUARS_H__

#include <algorithm>
#include <span>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <tuple>
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

            const Domain& at(size_t idx) const noexcept(false) {
                return operator[](idx);
            }

            Domain& at(size_t idx) noexcept(false) {
                return operator[](idx);
            }
            const Domain& at(const std::string &idx) const noexcept(false) {
                return operator()(idx);
            }
            Domain& at(const std::string &idx) noexcept(false) {
                return operator()(idx);
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

        static constexpr bool is_basic(Variant variant) {
            return (char)variant < (char)Variant::Array;
        }

        static constexpr bool is_array(Variant variant) {
            return (char)variant == (char)Variant::Array;
        }

        static constexpr bool is_complex(Variant variant) {
            return (char)variant > (char)Variant::Array;
        }

        static constexpr bool is_sint(Variant variant) {
            return (char)variant > (char)Variant::U64 && (char)variant < (char)Variant::F32;
        }

        static constexpr bool is_uint(Variant variant) {
            return (char)variant < (char)Variant::I8;
        }

        static constexpr bool is_float(Variant variant) {
            return (char)variant > (char)Variant::I64 && (char)variant < (char)Variant::Bool;
        }

        static constexpr bool is_bool(Variant variant) {
            return variant == Variant::Bool;
        }

        constexpr bool is_string(void) {
            return is_array(variant) && domains->at(array.idx).variant == Variant::I8;
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
        static constexpr void set_counter(types::U8 *data, size_t max_value, size_t value) {
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
        static constexpr size_t get_counter(types::U8 *data, size_t max_value) {
            using namespace types;

            if (max_value <= 0xFF)           return (size_t)*(U8*)data;
            else if (max_value <= 0xFFFF)    return (size_t)*(U16*)data;
            else                             return (size_t)*(U32*)data;
        }




        /**
         * Check if first domain competible to the second.
         *
         * If domains both basic and of the same variant, then they are competible.
         * If domains both array and their element are competible, then they are competible.
         * If domains both complex and second fields starts with the fields that all
         *      corresponding first domain's fields competible to seconds, then they are competible.
         *
         * Otherwise first domain are not competible to the second.
         *
         * @param fst - first domain.
         * @param snd - second domain.
         * @return result.
         **/
        static bool is_competible(const Domain& fst, const Domain& snd) noexcept(true) {
            if (fst.domains != snd.domains) return false;

            if (is_basic(fst.variant) && is_basic(snd.variant))
                return fst.variant == snd.variant;

            if (is_array(fst.variant) && is_array(snd.variant))
                return is_competible(fst.domains->at(fst.array.idx),
                                        snd.domains->at(snd.array.idx));

            if (is_complex(fst.variant) && is_complex(snd.variant)) {
                if (fst.variant != snd.variant) return false;

                auto fst_field = fst.complex_fields.begin();
                for (auto &snd_field: snd.complex_fields) {
                    auto &fst_field_domain = fst.domains->at(fst_field->domain_idx);
                    auto &snd_field_domain = snd.domains->at(snd_field.domain_idx);

                    if (!is_competible(fst_field_domain, snd_field_domain))
                        return false;

                    fst_field++;
                    if (fst_field == fst.complex_fields.end()) return true;
                }

                return false;
            }  

            return false;
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
                                        Domain_Idx elem_idx, size_t capacity) {
            domain_name = name;
            variant     = Variant((char)Variant::Array + (char)array_variant);
            array.capacity   = capacity;
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

           	domains.add({ "Str",     Array_Variant::Array, domains["I8"], 64 });
           	domains.add({ "String",  Array_Variant::Array, domains["I8"], 0xFF });
           	domains.add({ "Text",    Array_Variant::Array, domains["I8"], 1024 });
           	domains.add({ "BigText", Array_Variant::Array, domains["I8"], 0xFFFF });

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
        types::U8 *data;

        Domain_View(Domain *domain, char *data):
            domain(domain), data((types::U8*)data) { }

        Domain_View(Domain *domain, types::U8 *data):
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
        class Not_Complex_Variant: public Toad_Exception {
            public:
                Not_Complex_Variant(Domain::Variant variant):
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
         * Try to call add variant function on not add variant domain view.
         **/
        class Not_Add_Variant: public Toad_Exception {
            public:
                Not_Add_Variant(Domain::Variant variant):
                    Toad_Exception("Try to call add variant function on not add variant domain view. ("
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
            if (!Domain::is_complex(domain->variant)) throw Not_Complex_Variant(domain->variant);

            size_t offset = 0;
            size_t idx = 0;
            for (auto &field: domain->complex_fields) {
                Domain *field_domain = &(*domain->domains)[field.domain_idx];

                if (field.field_name == name) {
                    if (domain->variant == Domain::Variant::Add) {
                        const size_t fields_count = domain->complex_fields.size();
                        Domain::set_counter(data, fields_count, idx);
                        offset += Domain::counter_size_of(fields_count); 
                    }
                    return Domain_View { field_domain, data + offset, };
                }
                offset += field_domain->size_of();
                idx++;
            }

            throw Domain_Has_Not_Such_Field(name);
        }
        /**
         * Get Domain_View of the field by field name.
         **/
        const Domain_View operator[](const std::string &name) const {
            if (!Domain::is_complex(domain->variant)) throw Not_Complex_Variant(domain->variant);

            size_t offset = 0;
            size_t idx = 0;
            for (auto &field: domain->complex_fields) {
                Domain *field_domain = &(*domain->domains)[field.domain_idx];

                if (field.field_name == name) {
                    if (domain->variant == Domain::Variant::Add) {
                        const size_t fields_count = domain->complex_fields.size();
                        Domain::set_counter(data, fields_count, idx);
                        offset += Domain::counter_size_of(fields_count); 
                    }
                    return Domain_View { field_domain, data + offset, };
                }
                offset += field_domain->size_of();
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

        /**
         * Array length out of bounds.
         **/
        class Array_Length_Out_Of_Bounds: public Toad_Exception {
            public:
                Array_Length_Out_Of_Bounds(size_t len, size_t cap):
                	Toad_Exception(std::string("Array length out of bounds.")
                                        + " len: " + std::to_string(len)
                                        + " cap: " + std::to_string(cap)) { } 
        };

        /**
         * Try to call array function from not array domain view.
         **/
        class Not_Array_Variant: public Toad_Exception {
            public:
                Not_Array_Variant(Domain::Variant variant):
                    Toad_Exception("Try to call array function from not array domain view. ("
                                                    + to_string(variant) + ")") { }
        };

        class Pop_From_Empty_Array: public Toad_Exception {
            public:
                Pop_From_Empty_Array():
                	Toad_Exception("Try to pop on empty array.") { } 
        };


        Domain_View operator[](size_t idx) noexcept(false) {
            if (!Domain::is_complex(domain->variant) && !Domain::is_array(domain->variant))
                throw Not_Complex_Variant(domain->variant);

            if (Domain::is_complex(domain->variant)) {
                if (idx >= domain->complex_fields.size())
                    throw Field_Idx_Out_Of_Range(idx, domain->complex_fields.size());

                return operator[](domain->complex_fields[idx].field_name);
            } 

            if (Domain::is_array(domain->variant)) {
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

        const Domain_View operator[](size_t idx) const noexcept(false) {
            if (!Domain::is_complex(domain->variant) && !Domain::is_array(domain->variant))
                throw Not_Complex_Variant(domain->variant);

            if (Domain::is_complex(domain->variant)) {
                if (idx >= domain->complex_fields.size())
                    throw Field_Idx_Out_Of_Range(idx, domain->complex_fields.size());

                return operator[](domain->complex_fields[idx].field_name);
            } 

            if (Domain::is_array(domain->variant)) {
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
                throw Not_Add_Variant(domain->variant);

            size_t size = domain->size_of();
            size_t variant = 0;

            if (size <= 0xFF) {
                variant = *(types::U8*)data;
            } else if (size <= 0xFFFF) {
                variant = *(types::U16*)data;
            }

            return variant;
        }

        /**
         * Get length of the array.
         *
         * @return length of the array.
         * @throws Access_Length_Not_From_Array_Domain if domain is not an array variant.
         **/
        constexpr size_t get_length(void) const noexcept(false) {
            if (!Domain::is_array(domain->variant))
                throw Not_Array_Variant(domain->variant);

            return Domain::get_counter(data, domain->array.capacity); 
        }

        /**
         * Set new length of the array.
         * Value must be lesser or equal to capacity of the array.
         *
         * @param length - new length of the array.
         * @throws Array_Length_Out_Of_Bounds if length greater than capacity.
         * @throws Not_Array_Variant if domain is not an array variant.
         **/
        constexpr void set_length(size_t length) noexcept(false) {
            if (!Domain::is_array(domain->variant))
                throw Not_Array_Variant(domain->variant);

            if (length > domain->array.capacity)
                throw Array_Length_Out_Of_Bounds(length, domain->array.capacity);

            Domain::set_counter(data, domain->array.capacity, length); 
        }

        /**
         * Push basic element to the array.
         *
         * @param value - value to push.
         * @throws Unwrap_Invalid_Variant if type not suited to array element variant.
         * @throws like @see set_length & @see get_length
         **/
        template<typename Type>
        void array_push_basic(Type value) noexcept(false) {
            if (!Domain::is_array(domain->variant))
                throw Not_Array_Variant(domain->variant);

            if (Domain::type2variant<Type>() != (*domain->domains)[domain->array.idx].variant)
                throw Unwrap_Invalid_Variant(Domain::type2variant<Type>(),  domain->variant);

            set_length(get_length()+1); // will throw if overflow.


            ((Type*)(data + Domain::counter_size_of(domain->array.capacity)))
                    [get_length()-1] = value;
        }


        /**
         * Array pop.
         **/
        constexpr void array_pop(void) noexcept(false) {
            if (!Domain::is_array(domain->variant))
                throw Not_Array_Variant(domain->variant);

            if (get_length() == 0)
                throw Pop_From_Empty_Array();

            set_length(get_length()-1);
        }

        class Assign_Incompetible_Domains: public Toad_Exception {
            public:
                Assign_Incompetible_Domains(const std::string& dest, const std::string& orig):
                    Toad_Exception("Can't assign value of domain `" + orig + "`"
                                        + " to the value of domain `"+ dest + "`") { }
        };

        void assign(const Domain_View &new_value) noexcept(false) {
            Domain::Variant orig_variant = new_value.domain->variant;
            Domain::Variant dest_variant = domain->variant;
            const std::string &orig_name = new_value.domain->domain_name;
            const std::string &dest_name = new_value.domain->domain_name;

            if (!Domain::is_competible(*domain, *new_value.domain))
                throw Assign_Incompetible_Domains(dest_name, orig_name);

            if (Domain::is_basic(orig_variant)) { //TODO: support for the different sized values.
                std::memcpy(data, new_value.data, domain->size_of());
                return;
            }

            if (Domain::is_array(dest_variant)) {
                if (new_value.get_length() > domain->array.capacity)
                    throw Array_Length_Out_Of_Bounds(new_value.get_length(), domain->array.capacity);

                Domain *elem_domain = &domain->domains->at(domain->array.idx);
                Domain *new_elem_domain = &domain->domains->at(new_value.domain->array.idx);

                size_t elem_size = elem_domain->size_of();
                size_t new_elem_size = new_elem_domain->size_of();

                Domain_View elem { elem_domain,
                            data + Domain::counter_size_of(domain->array.capacity) };
                Domain_View new_elem { new_elem_domain,
                            new_value.data + Domain::counter_size_of(new_value.domain->array.capacity) };

                Domain::set_counter(data, domain->array.capacity, new_value.get_length()); 

                for (size_t i = 0; i < new_value.get_length(); i++) {
                    elem.assign(new_elem); 
                    elem.data += elem_size;
                    new_elem.data += new_elem_size;
                }
            } 

            if (dest_variant == Domain::Variant::Add) {
                size_t new_variant = Domain::get_counter(new_value.data,
                                                         new_value.domain->complex_fields.size());
                Domain::set_counter(data, domain->complex_fields.size(), new_variant);

                const auto &field = domain->complex_fields[new_variant];
                const auto &new_field = new_value.domain->complex_fields[new_variant];

                Domain_View elem { &domain->domains->at(field.domain_idx), 
                                   data + Domain::counter_size_of(domain->complex_fields.size()) };
                Domain_View new_elem { &domain->domains->at(new_field.domain_idx),
                                       data + Domain::counter_size_of(new_value.domain->complex_fields.size()) }; 

                elem.assign(new_elem); 
            }

            if (dest_variant == Domain::Variant::Mul) {
                Domain_View elem { nullptr, data}; 
                Domain_View new_elem { nullptr, new_value.data};

                for (size_t i = 0; i < domain->complex_fields.size(); i++) {
                    const auto &field = domain->complex_fields[i];
                    const auto &new_field = new_value.domain->complex_fields[i];

                    elem.domain = &domain->domains->at(field.domain_idx);
                    new_elem.domain = &domain->domains->at(new_field.domain_idx);

                    elem.assign(new_elem);

                    elem.data += elem.domain->size_of();
                    new_elem.data += new_elem.domain->size_of();
                }
            }
        }
        /**
         * Push value to the array.
         *
         * @param new value.
         **/
        void array_push(const Domain_View &value) noexcept(false) {
            if (!Domain::is_array(domain->variant))
                throw Not_Array_Variant(domain->variant);


            const auto &elem_domain = domain->domains->at(domain->array.idx);
            const auto &new_elem_domain = *value.domain;

            if (!Domain::is_competible(elem_domain, new_elem_domain))
                throw Assign_Incompetible_Domains(elem_domain.domain_name, 
                                                new_elem_domain.domain_name);

            set_length(get_length()+1); // will throw if overflow.

            operator[](get_length()-1).assign(value); 
        }

        friend std::string to_string(Domain_View view) {
            if (Domain::is_basic(view.domain->variant)) {

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
            if (Domain::is_array(view.domain->variant)) {
                size_t len = view.get_length(); 

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

        /**
         * Display as string domains that are array of I8 (char).
         **/
        std::string unwrap_string() {
            if (!Domain::is_array(domain->variant))
                throw Not_Array_Variant(domain->variant);

            if (domain->domains->at(domain->array.idx).variant == Domain::Variant::I8)
                throw Toad_Exception("Not a string");

            std::string res { };

            for (size_t i = 0; i < get_length(); i++) {
                res.push_back(operator[](i).unwrap_basic<types::I8>());
            }

            return res;
        }
    };


    /**
     * Contains value of specific domain.
     *
     * This container owns value of the domain and
     * allows to interct with that through the Domain_View.
     **/
    template<size_t Min_Size_To_Alloc = 8>
    class Domain_Value {
        Domain *domain;
        size_t size;
        union {
            std::array<types::U8, Min_Size_To_Alloc> small_data { 0 };
            types::U8* big_data;
        };

        public:
            Domain_Value<Min_Size_To_Alloc>(Domain* domain): domain(domain) {
                size = domain->size_of();

                if (size > Min_Size_To_Alloc) {
                    big_data = (types::U8*)std::malloc(size);
                }
            }

            ~Domain_Value<Min_Size_To_Alloc>(void) {
                if (size > Min_Size_To_Alloc) {
                    free(big_data);
                } 
            }

            class Wrong_Domain_Value_Constructor: public Toad_Exception {
                public:
                    Wrong_Domain_Value_Constructor(const std::string &domain_name,
                                                   const std::string& why):
                        Toad_Exception("Failed to construct value of domain `"
                                + domain_name + "`: " + why) {} 
            };


            /**
             * Gets domain view to the value.
             *
             * @return Domain_View to the owning value.
             **/
            Domain_View view(void) {
                return Domain_View { domain, size > Min_Size_To_Alloc ? big_data : small_data.data() };
            }
    };

    template<size_t Min_Size_To_Alloc = 8, typename T>
    make_domain_value(Domain *domain, const T& value);

	template<size_t Min_Size_To_Alloc>
    Domain_Value<Min_Size_To_Alloc>
    make_domain_value<Min_Size_To_Alloc, std::string>(Domain *domain, const std::string& string) {
        if (!domain->is_string())
            throw Domain_Value<Min_Size_To_Alloc>::Wrong_Domain_Value_Constructor(domain->domain_name,
                                                "Try to construct string value not to for string domain");

        Domain_Value<Min_Size_To_Alloc> ret { domain };
        for (auto c: string) {
            ret.view().template array_push_basic<types::I8>((types::I8)c);
        }

        return ret;
    }




    /**
     * Table.
     **/
    struct Table {
        struct Collumn_Field {
            std::string name;
            Domain &domain;
        };

        std::vector<Collumn_Field> columns_fields;
        size_t row_size;

        std::vector<char> data { };

        class Failed_To_Insert_Row: public Toad_Exception {
            public:
                Failed_To_Insert_Row(Toad_Exception& problem):
                    Toad_Exception(std::string("Failed to insert row: ") + problem.what()) {} 
        };


        Table(std::initializer_list<Collumn_Field> fields): columns_fields(fields)  { 
            row_size = 0;

            for (auto collumn_field: columns_fields) {
                row_size += collumn_field.domain.size_of();
            }
        }

        /**
         * Insert Row
         *
         * @row_value - vector of domain value to insert to the row.
         **/
        void insert_row(const std::vector<Domain_View> &row_value) {
            auto row_field = row_value.begin();

            auto row_data = std::unique_ptr<types::U8>(new types::U8[row_size]);

            Domain_View out { nullptr, row_data.get() };
            Domain_View in { row_field->domain, row_field->data };

            try {
                for (auto &collumn_domain: columns_fields) {
                    out.domain = &collumn_domain.domain;

                    out.assign(in); 

                    out.data += out.domain->size_of();
                    in.data   = (++row_field)->data;
                    in.domain = row_field->domain;
                }
            } catch (Toad_Exception& te) {
                throw Failed_To_Insert_Row(te);
            }

            data.assign(row_data.get(), row_data.get() + row_size);
        }

        friend  Table& operator<<(Table& table, const std::vector<Domain_View> &row_value);
    };
}

#endif
