#include <string>
#include <iostream>
#include <tuple>
#include <vector>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/pipeline.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <zlib.h>

namespace io = boost::iostreams;
using namespace io;


class a
{
public:
    
    a()
    : i(0), index(0), data(""), out(zlib_decompressor() | io::back_inserter(data))
    {}

    a(const a& rhs)
    : i(rhs.i), index(rhs.index), data(rhs.data), out(zlib_decompressor() | io::back_inserter(data))
    {}

    a(a&& rhs)
    : i(std::move(rhs.i)), index(std::move(rhs.index)), data(std::move(rhs.data)), out(zlib_decompressor() | io::back_inserter(data)) 
    {}
    
    /*a& operator=(const a& rhs)
    {
	
	return *this;
    };*/
    /*a& operator=(a&& rhs) = default;
    */
    ~a() = default;
    
    //out(zlib_decompressor() | back_inserter(data));
private:
    std::size_t i;
    std::size_t index;
    std::string data;
    filtering_ostream out;
};

int main()
{
    //std::string str = "123";
    //char str[] = "123";
    //array_source asrc(str, str+3);
    //stream<array_source> in(asrc);
    a b;
    a c(b);
    a d(std::move(b));
}
