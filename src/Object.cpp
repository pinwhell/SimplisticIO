#include <simplistic/io/Object.h>

using namespace simplistic::io;

Object::Object(const Object& obj)
	: mIO(obj.mIO)
	, mEntry(obj.mEntry)
	, mIs64(obj.mIs64)
{}

Object::Object(Object&& other) noexcept
	: mEntry(other.mEntry), mIO(other.mIO), mIs64(other.mIs64) {
	// Leave 'other' in a safe state
	other.mEntry = 0;
	other.mIO = nullptr;
	other.mIs64 = 0;
}

Object& Object::operator=(Object&& other) noexcept {
	if (this != &other) {
		// Transfer ownership of resources from 'other' to 'this'
		mEntry = other.mEntry;
		mIO = other.mIO;

		// Leave 'other' in a safe state
		other.mEntry = 0;
		other.mIO = nullptr;
	}
	return *this;
}

Object::operator bool() const
{
	return mEntry != 0;
}

bool Object::operator==(const Object& other) const {
	return mEntry == other.mEntry;
}

bool Object::operator!=(const Object& other) const {
	return !(*this == other);
}
