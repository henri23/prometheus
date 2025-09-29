#include "core/logger.hpp"
#include "memory/memory.hpp"
#include "test_manager.hpp"
#include "platform/platform.hpp"

int main() {
    test_manager_init();

    CORE_DEBUG("Starting tests...");

    test_manager_run_tests();

    return 0;
}
