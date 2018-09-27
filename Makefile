all:test.cpp
	g++ test.cpp -g -std=c++17 -o test.out -lgtest -lgtest_main -lpthread -lboost_iostreams -lz
