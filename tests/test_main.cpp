#include "framework.hpp"

int main() {
    int passed = 0, failed = 0;
    for (auto& t : test_registry()) {
        try {
            t.fn();
            std::printf("PASS  %s\n", t.name.c_str());
            ++passed;
        } catch (const std::exception& e) {
            std::printf("FAIL  %s  —  %s\n", t.name.c_str(), e.what());
            ++failed;
        }
    }
    std::printf("\n%d passed, %d failed\n", passed, failed);
    return failed ? 1 : 0;
}
