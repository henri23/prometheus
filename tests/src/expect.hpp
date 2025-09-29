#include <core/logger.hpp>

#define expect_should_be(expected, actual)                                                                      \
    {                                                                                                           \
        if (actual != expected) {                                                                               \
            ENGINE_ERROR("--> Expected %lld, but got %lld. File: %s:%d", expected, actual, __FILE__, __LINE__); \
            return false;                                                                                       \
        }                                                                                                       \
    }

#define expect_should_not_be(expected, actual)                                                                            \
    {                                                                                                                     \
        if (actual == expected) {                                                                                         \
            ENGINE_ERROR("--> Expected %d != %d, but they are equal. File: %s:%d", expected, actual, __FILE__, __LINE__); \
            return false;                                                                                                 \
        }                                                                                                                 \
    }

#define expect_float_to_be(expected, actual)                                                                \
    {                                                                                                       \
        if (fabs(actual - expected) > 0.001f) {                                                   \
            ENGINE_ERROR("--> Expected %f, but fot %f. File: %s:%d", expected, actual, __FILE__, __LINE__); \
            return false;                                                                                   \
        }                                                                                                   \
    }

#define expect_true(actual)                                                                                      \
    {                                                                                                            \
        if (actual != true) {                                                                                    \
            ENGINE_ERROR("--> Expected true, but got false. File: %s:%d", expected, actual, __FILE__, __LINE__); \
            return false;                                                                                        \
        }                                                                                                        \
    }

#define expect_false(actual)                                                                                     \
    {                                                                                                            \
        if (actual == true) {                                                                                    \
            ENGINE_ERROR("--> Expected false, but got true. File: %s:%d", expected, actual, __FILE__, __LINE__); \
            return false;                                                                                        \
        }                                                                                                        \
    }

