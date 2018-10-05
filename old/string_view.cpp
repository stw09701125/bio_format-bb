#include <iostream>
#include <string>

using namespace std;

int main()
{
    string temp("0123456789abcdefghijkl");
    temp = string_view(&temp[10], temp.size() - 10);
    temp = string_view(&temp[12], temp.size() - 12); 
    //temp = string_view(&temp[11], temp.size());
    cout << temp << endl;
    cout << temp.size() << endl;
    return 0;
}
