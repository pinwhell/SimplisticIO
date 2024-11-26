#pragma once

#include <simplistic/io/Utility/String.h>
#include <simplistic/io/IIO.h>
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace simplistic {
	namespace io {
		template<typename TObject = std::uint8_t>
		class ObjectT {
		public:
			ObjectT() : ObjectT(nullptr, 0, false) {}

			template<typename TWhere>
			ObjectT(IIO* io, TWhere entry = {}, bool bIs64 = sizeof(std::uintptr_t) == sizeof(std::uint64_t))
				: mIO(io)
				, mEntry((std::uint64_t)entry)
				, mIs64(bIs64)
				, mObject{}
			{}

			ObjectT(const ObjectT& obj);
			ObjectT& operator=(const ObjectT&) = delete;
			ObjectT(ObjectT&& other) noexcept;
			ObjectT& operator=(ObjectT&& other) noexcept;
			operator bool() const;
			bool operator==(const ObjectT& other) const;
			bool operator!=(const ObjectT& other) const;

			template<typename T, typename TOff = std::size_t>
			inline T Read(TOff off = {}) const
			{
				return mIO->Read<T>(mEntry + (std::size_t)off);
			}

			template<typename T, typename TOff = std::size_t>
			inline std::basic_string<T, std::char_traits<T>, std::allocator<T>> DerrefString(TOff off = {}, size_t sz = 0) const
			{
				return Derref((std::size_t)off)
					.ReadString<T>(0, sz);
			}

			template<typename T, typename TOff = std::size_t>
			inline std::basic_string<T, std::char_traits<T>, std::allocator<T>> ReadString(TOff off = {}, size_t sz = 0) const
			{
				using String = std::basic_string<T, std::char_traits<T>, std::allocator<T>>;
				constexpr size_t MAX_BLOB_SIZE_EXP = 10;  // For 1KB
				std::vector<T> storage;
				uint64_t strBase = mEntry + (std::size_t)off;

				if (sz > 0)
				{
					storage.resize(sz);
					if (!mIO->ReadRaw((const std::uint8_t*)strBase, (std::uint8_t*)storage.data(), sz * sizeof(T)))
						return String{};
					return std::string(storage.data(), storage.data() + sz);
				}

				size_t exploredLen = 0;
				size_t currExp = 1;

				do
				{
					size_t currBlobSize = 1ull << currExp;

					storage.resize(currBlobSize);

					if (!mIO->ReadRaw((const std::uint8_t*)(strBase + exploredLen), (std::uint8_t*)storage.data() + exploredLen, (currBlobSize - exploredLen) * sizeof(T)))
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

			template<typename T, typename TOff = std::size_t>
			inline void Write(TOff off = {}, const T& o = {}) const
			{
				mIO->Write<T>(mEntry + off, o);
			}

			template<typename TOff = std::size_t>
			inline std::vector<std::uint8_t> ReadBlob(size_t sz = 0, TOff off = {}) const
			{
				std::vector<std::uint8_t> blob(sz);
				mIO->ReadRaw((const std::uint8_t*)(mEntry + (size_t)off), blob.data(), sz);
				return blob;
			}

			template<typename TOff = std::size_t>
			inline uint64_t ReadPtr(TOff off = {}) const
			{
				return mIs64
					? mIO->Read<std::uint64_t>(mEntry + (std::size_t)off)
					: mIO->Read<std::uint32_t>(mEntry + (std::size_t)off);
			}

			template<typename TOff = std::size_t>
			inline ObjectT Derref(TOff off = {}) const
			{
				return ObjectT(
					mIO,
					ReadPtr((std::size_t)off)
				);
			}

			template<typename TAddress = TObject, typename TCount = std::size_t>
			inline ObjectT Address(TCount count = {}) const {
				return ObjectT(
					mIO,
					mEntry + ((std::size_t)count) * sizeof(TAddress),
					mIs64
				);
			}

			template<typename TObject = std::uint8_t, typename TOff = std::size_t>
			inline std::vector<TObject> ReadArray(std::size_t count, TOff off = {}) const
			{
				std::vector<TObject> arr(count, TObject{});
				mIO->ReadRawT(mEntry + off, arr.data(), count * sizeof(TObject));
				return arr;
			}

			template<typename TCount>
			inline ObjectT operator +(TCount adv) const
			{
				return Address(adv);
			}

			template<typename TCount>
			inline void operator+=(TCount adv)
			{
				mEntry += adv * sizeof(TObject);
			}

			TObject& operator*()
			{
				mIO->ReadRawT(mEntry, &mObject, sizeof(mObject));
				return mObject;
			}

			TObject* operator->()
			{
				mIO->ReadRawT(mEntry, &mObject, sizeof(mObject));
				return &mObject;
			}

			void Persist() const
			{
				mIO->WriteRawT(mEntry, &mObject, sizeof(mObject));
			}

			std::uint64_t mEntry;
			IIO* mIO;
			bool mIs64;
			TObject mObject;
		};

		using Object = ObjectT<std::uint8_t>;
	}
}