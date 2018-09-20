all:test.cpp
	g++ test.cpp -std=c++17 -o test.out -lgtest -lgtest_main -lpthread
