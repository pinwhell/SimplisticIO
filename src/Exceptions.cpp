#include <simplistic/io/Exceptions.h>

using namespace simplistic::io;

char const* IOException::what() const noexcept
{
    return "Input/Output Error.";
}
