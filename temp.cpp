#include <iostream>
#include <fstream>

using namespace std;

int main()
{
    ofstream ofile("temp.bb", ios::binary);
    //ofile << "hey";
    ofile.close();
     
    ifstream ifile("temp.bb", ios::binary);
    //char* buf;
    //ifile.read(buf, 100);
    if (ifile.peek() == std::ifstream::traits_type::eof())
    {
	cout << "empty file" << endl;
    }
    //cout << (ifile.peek() == ifile.eof()) << endl;
    return 0;
}
