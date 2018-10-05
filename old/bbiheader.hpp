#ifndef BBI
#define BBI

#include <iostream>
#include <fstream>

class BBIHeader
{
public:
	uint32_t magic;
	uint16_t version;
	uint16_t zoom_levels;
	uint64_t chrom_tree_offset;
	uint64_t full_data_offset;
	uint64_t full_index_offset;
	uint16_t field_count;
	uint16_t defined_field_count;
	uint64_t auto_sql_offset;
	uint64_t total_summary_offset;
	uint32_t uncompress_buf_size;
	uint64_t reserved_part;
	
	BBIHeader(){}
	
	BBIHeader(std::ifstream& file)
	{
		file.seekg(0);
		file.read(reinterpret_cast<char*>(&magic), 4);
		file.read(reinterpret_cast<char*>(&version), 2);
		file.read(reinterpret_cast<char*>(&zoom_levels), 2);
		file.read(reinterpret_cast<char*>(&chrom_tree_offset), 8);
		file.read(reinterpret_cast<char*>(&full_data_offset), 8);
		file.read(reinterpret_cast<char*>(&full_index_offset), 8);
		file.read(reinterpret_cast<char*>(&field_count), 2);
		file.read(reinterpret_cast<char*>(&defined_field_count), 2);
		file.read(reinterpret_cast<char*>(&auto_sql_offset), 8);
		file.read(reinterpret_cast<char*>(&total_summary_offset), 8);
		file.read(reinterpret_cast<char*>(&uncompress_buf_size), 4);
		file.read(reinterpret_cast<char*>(&reserved_part), 8);
	}
	
	void print()
	{
		std::cout << std::hex << "magic: " << magic << std::endl;
		std::cout << std::dec << "version: " << version << std::endl;
		std::cout << std::dec << "zoom_levels: " << zoom_levels << std::endl;
		std::cout << std::dec << "chrom_tree_offset: " << chrom_tree_offset << std::endl;
		std::cout << std::dec << "full_data_offset: " << full_data_offset << std::endl;
		std::cout << std::dec << "full_index_offset: " << full_index_offset << std::endl;
		std::cout << std::dec << "field_count: " << field_count << std::endl;
		std::cout << std::dec << "defined_field_count: " << defined_field_count << std::endl;
		std::cout << std::dec << "auto_sql_offset: " << auto_sql_offset << std::endl;
		std::cout << std::dec << "total_summary_offset: " << total_summary_offset << std::endl;
		std::cout << std::dec << "uncompress_buf_size: " << uncompress_buf_size << std::endl;
		std::cout << std::dec << "reserved_part: " << reserved_part << std::endl;
	}
};

#endif
