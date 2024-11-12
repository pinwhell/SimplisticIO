#pragma once

#include <simplistic/io/Exceptions.h>
#include <cstdint>
#include <cstddef>

namespace simplistic {
	namespace io {
        class IIO {
        public:
            virtual ~IIO() = default;

            virtual std::size_t Read(const std::uint8_t* where, std::uint8_t* out, std::size_t len) = 0;
            virtual std::size_t Write(std::uint8_t* where, const std::uint8_t* in, std::size_t len) = 0;

            template<typename TObj, typename TWhere = std::uint64_t>
            inline TObj ReadO(TWhere where)
            {
                TObj obj{}; if (Read((const std::uint8_t*)where, (std::uint8_t*)&obj, sizeof(TObj)) != sizeof(TObj))
                    throw IOException();
                return obj;
            }

            template<typename TObj, typename TWhere = std::uint64_t>
            inline void WriteO(TWhere where, const TObj& what)
            {
                if (Write((std::uint8_t*)where, (const std::uint8_t*)&what, sizeof(TObj)) != sizeof(TObj))
                    throw IOException();
            }
        };
	}
}