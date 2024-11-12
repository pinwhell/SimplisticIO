#include <simplistic/io/SelfIO.h>
#include <string.h>

using namespace simplistic::io;

std::size_t Self::Read(const std::uint8_t* where, std::uint8_t* out, std::size_t len)
{
    memcpy(out, where, len);
    return len;
}

std::size_t Self::Write(std::uint8_t* where, const std::uint8_t* in, std::size_t len)
{
    memcpy(where, in, len);
    return len;
}