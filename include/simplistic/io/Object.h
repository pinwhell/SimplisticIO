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
		struct is_string_or_wstring : std::false_type {};

		template <>
		struct is_string_or_wstring<std::string> : std::true_type {};

		template <>
		struct is_string_or_wstring<std::wstring> : std::true_type {};

		// Adding the _v shortcut for convenience
		template <typename T>
		constexpr bool is_string_or_wstring_v = is_string_or_wstring<T>::value;

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
					mObject = std::move(other.mObject);

					// Leave 'other' in a safe state
					other.mEntry = 0;
					other.mIO = nullptr;
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

			template<typename T, std::size_t MAX_BLOB_SIZE_EXP = 10, typename TOff = std::size_t>
			inline std::basic_string<T, std::char_traits<T>, std::allocator<T>> ReadString(TOff off = {}, size_t sz = 0) const
			{
				using String = std::basic_string<T, std::char_traits<T>, std::allocator<T>>;

				if (sz > 0)
				{
					std::vector<T> strArr = ReadArray<T>(sz, off);
					return String(strArr.begin(), strArr.end());
				}

				std::vector<T> buff; buff.reserve(1ull << MAX_BLOB_SIZE_EXP);
				std::size_t currOrder = 0;
				for (;;)
				{
					if (currOrder >= MAX_BLOB_SIZE_EXP)	break;
					std::size_t currSliceSz = 1ull << currOrder;
					std::vector<T> currSlice = ReadArray<T>(currSliceSz, (std::size_t)off + buff.size());
					buff.insert(buff.end(), currSlice.begin(), currSlice.end());
					currOrder++;
					std::size_t probeLen = simplistic::io::strnlen<T>(buff.data(), buff.size());
					if (buff.size() == probeLen) continue;
					// At this point, null-terminator found...
					// Returning the string.
					return String(buff.data(), probeLen);
				}
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
				mIO->ReadRawT(mEntry + off, arr.data(), count * sizeof(TObj));
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
			inline std::enable_if_t<!is_string_or_wstring_v<TObj>, TObj&> operator*()
			{
				mIO->ReadRawT(mEntry, &mObject, sizeof(mObject));
				return mObject;
			}

			template <typename TObj = TObject>
			inline std::enable_if_t<is_string_or_wstring_v<TObj>, TObj> operator*()
			{
				return ReadString<TObject::value_type>();
			}

			inline TObject* operator->()
			{
				mIO->ReadRawT(mEntry, &mObject, sizeof(mObject));
				return &mObject;
			}

			inline void Persist() const
			{
				mIO->WriteRawT(mEntry, &mObject, sizeof(mObject));
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

		using Object = ObjectT<std::uint8_t>;
		using OString = ObjectT<std::string>;
		using OWstring = ObjectT<std::wstring>;
	}
}