#include <iostream>

#define Sig_o 0x8789F2EB
#define Sig 0x8789F2EB12345678


template<typename INT>
INT swap_bytes(INT sig)
{
    char* sig_temp = reinterpret_cast<char*>(&sig);
    char temp[sizeof(INT)];
    for (std::size_t i = 0; i < sizeof(INT); ++i)
    {
	temp[i] = sig_temp[sizeof(INT) - i - 1];
    }
    return *(reinterpret_cast<INT*>(&temp[0]));
}

int main()
{
    std::cout << "origin sig: " << std::hex << Sig << std::endl;
    std::cout << "swapped sig: " << std::hex << swap_bytes<uint64_t>(Sig) << std::endl;
}
