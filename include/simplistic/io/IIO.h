#pragma once

#include <simplistic/io/Exceptions.h>
#include <cstdint>
#include <cstddef>

namespace simplistic {
	namespace io {
        class IIO {
        public:
            virtual ~IIO() = default;

            virtual std::size_t ReadRaw(const std::uint8_t* where, std::uint8_t* out, std::size_t len) = 0;
            virtual std::size_t WriteRaw(std::uint8_t* where, const std::uint8_t* in, std::size_t len) = 0;

            template<typename TWhere, typename TBuffOut>
            inline std::size_t ReadRawT(TWhere where, TBuffOut out, std::size_t len)
            {
                return ReadRaw((const std::uint8_t*)where, (std::uint8_t*)out, len);
            }

            template<typename TWhere, typename TBuffIn>
            inline std::size_t WriteRawT(TWhere where, TBuffIn in, std::size_t len)
            {
                return WriteRaw((std::uint8_t*)where, (const std::uint8_t*)in, len);
            }

            template<typename TObj, typename TWhere = std::uint64_t>
            inline TObj Read(TWhere where = 0)
            {
                TObj obj{}; if (ReadRaw((const std::uint8_t*)where, (std::uint8_t*)&obj, sizeof(TObj)) != sizeof(TObj))
                    throw IOException();
                return obj;
            }

            template<typename TObj, typename TWhere = std::uint64_t>
            inline void Write(TWhere where, const TObj& what)
            {
                if (WriteRaw((std::uint8_t*)where, (const std::uint8_t*)&what, sizeof(TObj)) != sizeof(TObj))
                    throw IOException();
            }
        };
	}
}