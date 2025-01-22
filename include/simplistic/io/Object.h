#pragma once

#include <simplistic/io/Utility/String.h>
#include <simplistic/io/IIO.h>
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace simplistic {
	namespace io {
		template <typename T>
		struct is_basic_string : std::false_type {};

		template <typename CharT, typename Traits, typename Allocator>
		struct is_basic_string<std::basic_string<CharT, Traits, Allocator>> : std::true_type {};

		template <typename T>
		constexpr bool is_basic_string_v = is_basic_string<T>::value;

		template<typename TObject = std::uint8_t>
		class ObjectT {
		public:
			using value_type = TObject;

			inline ObjectT() : ObjectT(nullptr, 0, false) {}

			template<typename TWhere>
			inline ObjectT(IIO* io, TWhere entry = {}, bool bIs64 = sizeof(std::uintptr_t) == sizeof(std::uint64_t))
				: mIO(io)
				, mEntry((std::uint64_t)entry)
				, mIs64(bIs64)
				, mObject{}
			{}

			ObjectT& operator=(const ObjectT&) = delete;

			template<typename TObjectT>
			inline ObjectT(const TObjectT& obj)
				: mIO(obj.mIO)
				, mEntry(obj.mEntry)
				, mIs64(obj.mIs64)
			{}

			template<typename TObjectT>
			inline ObjectT(TObjectT&& other) noexcept
				: mEntry(other.mEntry), mIO(other.mIO), mIs64(other.mIs64) {
				// Leave 'other' in a safe state
				other.mEntry = 0;
				other.mIO = nullptr;
				other.mIs64 = 0;
			}

			template<typename TObjectT>
			inline ObjectT& operator=(TObjectT&& other) noexcept {
				if (this != &other) {
					// Transfer ownership of resources from 'other' to 'this'
					mEntry = other.mEntry;
					mIO = other.mIO;

					// Leave 'other' in a safe state
					other.mEntry = 0;
					other.mIO = nullptr;
					if constexpr (std::is_same_v<decltype(mObject), decltype(other.mObject)>)
						mObject = std::move(other.mObject);

				}
				return *this;
			}

			inline operator bool() const
			{
				return mEntry != 0;
			}

			template<typename TObjectT>
			inline bool operator==(const TObjectT& other) const {
				return mEntry == other.mEntry;
			}

			template<typename TObjectT>
			inline bool operator!=(const TObjectT& other) const {
				return !(*this == other);
			}

			template<typename T, typename TOff = std::size_t>
			inline std::enable_if_t<!is_basic_string_v<T>, T>  Read(TOff off = {}) const
			{
				return mIO->Read<T>(mEntry + (std::size_t)off);
			}

			template<typename TBasicStr = TObject,
				std::size_t MAX_BLOB_SIZE_EXP = 10,
				typename TOff = std::size_t,
				typename TChar = TBasicStr::value_type>
			inline std::enable_if_t<is_basic_string_v<TBasicStr>, TBasicStr> Read(TOff off = {}, size_t sz = 0) const
			{
				if (sz > 0)
				{
					std::vector<TChar> strArr = ReadArray<TChar>(sz, off);
					return TBasicStr(strArr.begin(), strArr.end());
				}

				std::vector<TChar> buff; buff.reserve(1ull << MAX_BLOB_SIZE_EXP);
				std::size_t currOrder = 0;
				for (;;)
				{
					if (currOrder >= MAX_BLOB_SIZE_EXP)	break;
					std::size_t currSliceSz = 1ull << currOrder;
					std::vector<TChar> currSlice = ReadArray<TChar>(currSliceSz, (std::size_t)off + buff.size());
					buff.insert(buff.end(), currSlice.begin(), currSlice.end());
					currOrder++;
					std::size_t probeLen = simplistic::io::strnlen<TChar>(buff.data(), buff.size());
					if (buff.size() == probeLen) continue;
					// At this point, null-terminator found...
					// Returning the string.
					return TBasicStr(buff.data(), probeLen);
				}
				return TBasicStr{};
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
				if (mIO->ReadRaw((const std::uint8_t*)(mEntry + (size_t)off), blob.data(), sz) != sz)
					throw IOException();
				return blob;
			}

			template<typename TOff = std::size_t>
			inline uint64_t ReadPtr(TOff off = {}) const
			{
				return mIs64
					? mIO->Read<std::uint64_t>(mEntry + (std::size_t)off)
					: mIO->Read<std::uint32_t>(mEntry + (std::size_t)off);
			}

			template<typename TObjectT = ObjectT, typename TOff = std::size_t>
			inline TObjectT Derref(TOff off = {}) const
			{
				return TObjectT(
					mIO,
					ReadPtr((std::size_t)off),
					mIs64
				);
			}

			template<typename TObjectT = ObjectT, typename TAddress = std::uint8_t, typename TCount = std::size_t>
			inline TObjectT Address(TCount count = {}) const {
				return TObjectT(
					mIO,
					mEntry + ((std::size_t)count) * sizeof(TAddress),
					mIs64
				);
			}

			template<typename TObj = TObject, typename TOff = std::size_t>
			inline std::vector<TObj> ReadArray(std::size_t count, TOff off = {}) const
			{
				std::vector<TObj> arr(count, TObj{});
				auto toReadSz = count * sizeof(TObj);
				if (mIO->ReadRawT(mEntry + off, arr.data(), toReadSz) != toReadSz)
					throw IOException();
				return arr;
			}

			template<typename TCount>
			inline ObjectT operator +(TCount adv) const
			{
				return Address<ObjectT, TObject>(adv);
			}

			template<typename TCount>
			inline void operator+=(TCount adv)
			{
				mEntry += adv * sizeof(TObject);
			}
			
			template <typename TObj = TObject>
			inline std::enable_if_t<!is_basic_string_v<TObj>, TObj&> operator*()
			{
				if (mIO->ReadRawT(mEntry, &mObject, sizeof(mObject)) != sizeof(mObject))
					throw IOException();
				return mObject;
			}

			template <typename TBasicStr = TObject>
			inline std::enable_if_t<is_basic_string_v<TBasicStr>, TBasicStr&> operator*()
			{
				mObject = Read<TBasicStr>();
				return mObject;
			}

			inline TObject* operator->()
			{
				return &operator*();
			}

			inline bool Persist() const
			{
				return mIO->WriteRawT(mEntry, &mObject, sizeof(mObject)) == sizeof(mObject);
			}

			inline std::size_t PtrSz() const
			{
				return mIs64 ? sizeof(std::uint64_t) : sizeof(std::uint32_t);
			}

			std::uint64_t mEntry;
			IIO* mIO;
			bool mIs64;
			TObject mObject;
		};

		// Base Object
		using Object = ObjectT<std::uint8_t>;

		// Object String
		using OString = ObjectT<std::string>;

		// Object Wide String
		using OWstring = ObjectT<std::wstring>;

		// Object Basic String
		template<typename TBasicString, typename = std::enable_if_t<is_basic_string_v<TBasicString>, void>>
		using OBstring = ObjectT<TBasicString>;
	}
}