#ifndef BPLUS
#define BPLUS

#include <iostream>
#include <fstream>

class BPlusHeader
{
public:
	uint32_t magic;
	uint32_t child_num;
	uint32_t key_num;
	uint32_t val_size;
	uint64_t item_count;
	uint64_t reserved_part;
	BPlusHeader(){}
	BPlusHeader(std::ifstream& file, size_t offset)
	{
		file.seekg(offset);
		file.read(reinterpret_cast<char*>(&magic), 4);
		file.read(reinterpret_cast<char*>(&child_num), 4);
		file.read(reinterpret_cast<char*>(&key_num), 4);
		file.read(reinterpret_cast<char*>(&val_size), 4);
		file.read(reinterpret_cast<char*>(&item_count), 8);
		file.read(reinterpret_cast<char*>(&reserved_part), 8);
	}
	
	void print()
	{
		std::cout << std::hex << "magic: " << magic << std::endl;  
		std::cout << std::dec << "child number per block: " << child_num << std::endl;
		std::cout << std::dec << "key number: " << key_num << std::endl;
		std::cout << std::dec << "value size: " << val_size << std::endl;
		std::cout << std::dec << "item number: " << item_count << std::endl;
		std::cout << std::dec << "reserverd part: " << reserved_part << std::endl;
	}
};



#endif
