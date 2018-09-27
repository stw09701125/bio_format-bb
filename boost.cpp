#include <string>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>

namespace io = boost::iostreams;
using namespace io;

int main()
{
    //std::string str = "123";
    char str[] = "123";
    array_source asrc(str, str+3);
    stream<array_source> in(asrc);
}
