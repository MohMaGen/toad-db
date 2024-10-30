#include <common.hpp>

namespace toad_db {
    std::ostream& operator<<(std::ostream& os, Domain::Variant variant) {
        return os << to_string(variant);
    }
}
