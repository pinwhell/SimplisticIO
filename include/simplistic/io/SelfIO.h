#pragma once

#include <simplistic/io/IIO.h>

namespace simplistic {
	namespace io {
		class Self : public IIO {
		public:
			std::size_t ReadRaw(const std::uint8_t* where, std::uint8_t* out, std::size_t len);
			std::size_t WriteRaw(std::uint8_t* where, const std::uint8_t* in, std::size_t len);
		};
	}
}