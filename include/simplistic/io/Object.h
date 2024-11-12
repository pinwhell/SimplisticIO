#pragma once

#include <simplistic/io/Utility/String.h>
#include <simplistic/io/IIO.h>
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace simplistic {
	namespace io {
		class Object {
		public:
			Object() : Object(nullptr, 0, false) {}

			template<typename TWhere>
			Object(IIO* io, TWhere entry = {}, bool bIs64 = sizeof(std::uintptr_t) == sizeof(std::uint64_t))
				: mIO(io)
				, mEntry((std::uint64_t)entry)
				, mIs64(bIs64)
			{}

			Object(const Object& obj);
			Object& operator=(const Object&) = delete;
			Object(Object&& other) noexcept;
			Object& operator=(Object&& other) noexcept;
			operator bool() const;
			bool operator==(const Object& other) const;
			bool operator!=(const Object& other) const;

			template<typename T, typename TOff>
			inline T Read(TOff off) const
			{
				return mIO->ReadO<T>(mEntry + (std::size_t)off);
			}

			template<typename T, typename TOff>
			inline std::basic_string<T, std::char_traits<T>, std::allocator<T>> DerrefString(TOff off, size_t sz = 0) const
			{
				return Derref((std::size_t)off)
					.ReadString<T>(0, sz);
			}

			template<typename T, typename TOff>
			inline std::basic_string<T, std::char_traits<T>, std::allocator<T>> ReadString(TOff off, size_t sz = 0) const
			{
				using String = std::basic_string<T, std::char_traits<T>, std::allocator<T>>;
				constexpr size_t MAX_BLOB_SIZE_EXP = 10;  // For 1KB
				std::vector<T> storage;
				uint64_t strBase = mEntry + (std::size_t)off;

				if (sz > 0)
				{
					storage.resize(sz);
					if (!mIO->Read((const std::uint8_t*)strBase, (std::uint8_t*)storage.data(), sz * sizeof(T)))
						return String{};
					return std::string(storage.data(), storage.data() + sz);
				}

				size_t exploredLen = 0;
				size_t currExp = 1;

				do
				{
					size_t currBlobSize = 1ull << currExp;

					storage.resize(currBlobSize);

					if (!mIO->Read((const std::uint8_t*)(strBase + exploredLen), (std::uint8_t*)storage.data() + exploredLen, (currBlobSize - exploredLen) * sizeof(T)))
						return String{};

					size_t localUnexploredLen = currBlobSize - exploredLen;
					size_t localExploredLen = simplistic::io::strnlen<T>(storage.data() + exploredLen, localUnexploredLen);

					if (localExploredLen >= localUnexploredLen)
						continue;

					// We hit a nullterminator!
					// Lets succesfully Return the string!

					size_t strFullLen = exploredLen + localExploredLen;
					return String(storage.data(), storage.data() + strFullLen);
				} while (++currExp <= MAX_BLOB_SIZE_EXP);

				return String{};
			}

			template<typename T>
			inline void Write(size_t off, const T& o) const
			{
				mIO->WriteO<T>(mEntry + off, o);
			}

			template<typename TOff>
			inline std::vector<std::uint8_t> ReadBlob(TOff off, size_t sz) const
			{
				std::vector<std::uint8_t> blob(sz);
				mIO->Read((const std::uint8_t*)(mEntry + (size_t)off), blob.data(), sz);
				return blob;
			}

			template<typename TOff>
			inline uint64_t ReadPtr(TOff off) const
			{
				return mIs64
					? mIO->ReadO<std::uint64_t>(mEntry + (std::size_t)off)
					: mIO->ReadO<std::uint32_t>(mEntry + (std::size_t)off);
			}

			template<typename TOff>
			inline Object Derref(TOff off) const
			{
				return Object(
					mIO,
					ReadPtr((std::size_t)off)
				);
			}

			template<typename TOff>
			inline Object Address(TOff off) const {
				return Object(
					mIO,
					mEntry + (std::size_t)off,
					mIs64
				);
			}

			std::uint64_t mEntry;
			IIO* mIO;
			bool mIs64;
		};
	}
}