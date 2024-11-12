#pragma once

#include <stdexcept>

namespace simplistic {
	namespace io {
		class IOException : public std::exception {
		public:
			char const* what() const noexcept override;
		};
	}
}
