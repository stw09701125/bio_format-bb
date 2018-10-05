#include <iostream>
#include <fstream>
#include <string>


int main()
{
    std::ifstream ifile("../test.bb", std::ios::binary);
    std::string s("");
    
    ifile.seekg(0, std::ios::end);
    std::size_t end_pos = ifile.tellg();
    //std::cout << ifile.tellg() << std::endl;
    ifile.seekg(0, std::ios::beg);
    //std::cout << ifile.tellg() << std::endl;
    s.resize(end_pos);
    ifile.read(s.data(), end_pos);
    
    uint32_t magic;
    uint16_t ver;
    uint16_t z;
    uint64_t ct;
    uint64_t data_o;
    uint64_t i_o;
    uint16_t fc;

    magic = *(reinterpret_cast<uint32_t*>(s.data()));
    ver = *(reinterpret_cast<uint16_t*>(s.data() + 4));
    z = *(reinterpret_cast<uint16_t*>(s.data() + 6));
    ct = *(reinterpret_cast<uint64_t*>(s.data() + 8));
    data_o = *(reinterpret_cast<uint64_t*>(s.data() + 16));
    i_o = *(reinterpret_cast<uint64_t*>(s.data() + 24));
    fc = *(reinterpret_cast<uint16_t*>(s.data() + 32));

    std::cout << magic << std::endl;
    std::cout << ver << std::endl;
    std::cout << z << std::endl;
    std::cout << ct << std::endl;
    std::cout << data_o << std::endl;
    std::cout << i_o << std::endl;
    std::cout << fc << std::endl;
    
    //std::cout << s << std::endl;    
    return 0;
}
