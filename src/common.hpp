#ifndef __COMMON_GUARS_H__
#define __COMMON_GUARS_H__

#include <cstdint>
#include <stdexcept>


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
    }

	/**
     * Base class for all exception in the library.
     **/
    class Toad_Exception: public std::runtime_error {
        public:
            Toad_Exception(const std::string &reason):
                std::runtime_error("Toad: `" + reason + "`.") { }
    };


	/**
     * Dynamic typed domen.
     *
     * Domens can be different variants, actual data of domen allocated on
     * the heap.
     *
     * Domen owning value, which means it's free value in destructor.
     **/
    struct Domen {
        /**
         * # Variant of contained domen .
         *
         * - Just basic types: U8, U16, U32, U64, I8, I16, I32, I64, F32, F64.
         * 
         * - Alias: variant of alias domen used for domens like ```domen Day = U8;```.
         *   value of alias contains variant and data.
         *
         * - Array: represent array of values of specific domen.
         *
         * - Algebraic_Domen: domen representing sum and or multiplication of other domains.
         * 	 Like in __Haskell__.
         **/
        enum class Variant: char { 
            U8=0, U16, U32, U64, I8, I16, I32, I64, F32, F64,
            Alias, Array,  Algebraic_Domen,
        } variant; 

        /**
         * Data of the domen.
         **/
        void* value;

        class Domen_Unwrap_Exception: public Toad_Exception {
            public: Domen_Unwrap_Exception(Variant expect, Variant real);
        };

        struct Alias_Data {

        };

        size_t size_of() const noexcept {
            using namespace types;
            switch (variant) {
            case Variant::U8: return sizeof(U8);
            case Variant::U16: return sizeof(U16);
            case Variant::U32: return sizeof(U32);
            case Variant::U64: return sizeof(U64);
            case Variant::I8: return sizeof(I8);
            case Variant::I16: return sizeof(I16);
            case Variant::I32: return sizeof(I32);
            case Variant::I64: return sizeof(I64);
            case Variant::F32: return sizeof(F32);
            case Variant::F64: return sizeof(F64);
            case Variant::Alias: {
            } break;
            case Variant::Array: {
            } break;
            case Variant::Algebraic_Domen: {
            } break;
            }
        }

        Domen(const Domen& domen) {
            variant = domen.variant;
        }


        types::U8 get_u8() const noexcept(false) {
            if (variant != Variant::U8) throw Domen_Unwrap_Exception(Variant::U8, variant);
            return *(types::U8*)value;
        }

        types::U16 get_u16() const noexcept(false) {
            if (variant != Variant::U16) throw Domen_Unwrap_Exception(Variant::U16, variant);
            return *(types::U16*)value;
        }

        types::U32 get_u32() const noexcept(false) {
            if (variant != Variant::U32) throw Domen_Unwrap_Exception(Variant::U32, variant);
            return *(types::U32 *)value;
        }

        types::U64  get_u64() const noexcept(false) {
            if (variant != Variant::U64) throw Domen_Unwrap_Exception(Variant::U64, variant);
            return *(types::U64*)value;
        }

        types::I8 get_i8() const noexcept(false) {
            if (variant != Variant::I8) throw Domen_Unwrap_Exception(Variant::I8, variant);
            return *(types::I8*)value;
        }

        types::I16 get_i16() const noexcept(false) {
            if (variant != Variant::I16) throw Domen_Unwrap_Exception(Variant::I16, variant);
            return *(types::I16*)value;
        }

        types::I32 get_i32() const noexcept(false) {
            if (variant != Variant::I32) throw Domen_Unwrap_Exception(Variant::I32, variant);
            return *(types::I32 *)value;
        }

        types::I64  get_i64() const noexcept(false) {
            if (variant != Variant::I64) throw Domen_Unwrap_Exception(Variant::I64, variant);
            return *(types::I64*)value;
        }

        types::F32 get_f32() const noexcept(false) {
            if (variant != Variant::F32) throw Domen_Unwrap_Exception(Variant::F32, variant);
            return *(types::F32 *)value;
        }

        types::F64  get_f64() const noexcept(false) {
            if (variant != Variant::F64) throw Domen_Unwrap_Exception(Variant::F64, variant);
            return *(types::F64*)value;
        }

    };

    class Table {

    };
}

#endif
