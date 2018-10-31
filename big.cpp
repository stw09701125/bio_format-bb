#include <iostream>
#include <fstream>
#include "bigbed.hpp"

using namespace biovoltron::format;

int main()
{
    std::ifstream ifile("../bigfile/test3.bb", std::ios::binary);
    //std::ifstream ifile("../hg19.bb", std::ios::binary);
    bigbed::Header h(ifile);
    //auto data_count = h.get_data_count(); 
    std::cout << "data_count: " << h.get_data_count() << std::endl;
    //std::vector<bigbed::BigBed> bbv;
    //bbv.reserve(54310218);
    /*for (int i = 0; i < 54310218; ++i)
    {
	bigbed::BigBed temp(h);
	bigbed::BigBed::get_obj(ifile, temp);
	bbv.emplace_back(temp);
    }*/

    bigbed::BigBed temp(h);
    bigbed::BigBed::get_obj(ifile, temp);
    std::ofstream ofile("temp.bb", std::ios::binary);
    ofile << temp;
    /*while(1)
    {
    
    }*/
    return 0;
}
