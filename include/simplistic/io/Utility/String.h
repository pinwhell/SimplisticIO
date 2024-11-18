#pragma once

#include <cstddef>

namespace simplistic {
    namespace io {
        template <typename CharT>
        inline size_t strnlen(const CharT* str, size_t max_len);

        template <>
        inline size_t strnlen<char>(const char* str, size_t max_len) {
            size_t len = 0;
            while (len < max_len && str[len] != '\0') {
                ++len;
            }
            return len;
        }

        template <>
        inline size_t strnlen<char16_t>(const char16_t* str, size_t max_len) {
            size_t len = 0;
            while (len < max_len && str[len] != u'\0') {
                ++len;
            }
            return len;
        }
    }
}