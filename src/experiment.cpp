#include <cstddef>
#include <iostream>
#include <span>

int main() {
    int data[] = {0, 1, 2, 3, 4, 5, 6, 7};
    size_t len;
    std::cin >> len;

    for (auto i : std::span{ data, len }) {
        std::cout << i << std::endl;
    }

    return 0;
}
