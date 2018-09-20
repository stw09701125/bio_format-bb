#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <memory>

using namespace std;

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
	
	BBIHeader(ifstream& file)
	{
		file.seekg(0);
		//char buf[64];
		//memset(buf, 0, sizeof(buf));
		//std::unique_ptr<char[]> buf(new char[64]);
		/*string buf("", 64);
		file.read(&buf[0], 64);
		magic = buf[0];
		version = buf[4];
		zoom_levels = buf[6];
		chrom_tree_offset = buf[8];
		full_data_offset = buf[16];*/
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
		cout << hex << "magic: " << magic << endl;
		cout << dec << "version: " << version << endl;
		cout << dec << "zoom_levels: " << zoom_levels << endl;
		cout << dec << "chrom_tree_offset: " << chrom_tree_offset << endl;
		cout << dec << "full_data_offset: " << full_data_offset << endl;
		/*cout << dec << "full_index_offset: " << full_index_offset << endl;
		cout << dec << "field_count: " << field_count << endl;
		cout << dec << "defined_field_count: " << defined_field_count << endl;
		cout << dec << "auto_sql_offset: " << auto_sql_offset << endl;
		cout << dec << "total_summary_offset: " << total_summary_offset << endl;
		cout << dec << "uncompress_buf_size: " << uncompress_buf_size << endl;
		cout << dec << "reserved_part: " << reserved_part << endl;
		*/
	}
};


int main()
{
	//std::cout << "test gcc" << std::endl;
	ifstream f("../temp.bb", ios::binary);	
	BBIHeader bbi_header(f);
	/*f.seekg(bbi_header.chrom_tree_offset);
	uint32_t magic;
	f.read(reinterpret_cast<char*>(&magic), 4);
	cout << hex << magic << endl;*/
	bbi_header.print();
	//string magic(4, ' ');
	//uint32_t magic;
	//f.read(reinterpret_cast<char*>(&magic), 4);
	//f.read(&magic[0], 4);
	//string temp(64, ' ');
	//f.read(&temp[0], 64);
	//cout << hex << *(reinterpret_cast<uint32_t*>(&magic[0])) << endl;
	//cout << hex << magic << endl;
	//uint16_t version;
	//f.read(reinterpret_cast<char*>(&version), 2);
	//cout << dec << version << endl; 
	
	return 0;
}
