#include <gtest/gtest.h>
#include <fstream>
#include "bigbed.hpp"

using namespace biovoltron::format;

/*std::string test1_bb()
{
    return (nucleona::test::data_dir() / "test1.bb").string();
}

std::string test2_bb()
{
    return (nucleona::test::data_dir() / "test2.bb").string();
}*/

template <std::size_t n>
void static_loop_check_EQ(const bigbed::Header& lhs
    , const bigbed::HeaderType& rhs)
{
    EXPECT_EQ(lhs.get_member<n>(), std::get<n>(rhs));
    if constexpr(n != 1)
        static_loop_check_EQ<n + 1>(lhs, rhs);
}

template <std::size_t n>
void static_loop_check_EQ(const bigbed::BigBed& lhs
    , const bigbed::BBMemberType& rhs)
{
    EXPECT_EQ(lhs.get_member<n>(), std::get<n>(rhs));
    if constexpr(n != 3)
        static_loop_check_EQ<n + 1>(lhs, rhs);
}

/* default header answer */

bigbed::HeaderType default_header_ans
{
    bigbed::BBIHeader {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
  , bigbed::ChromList()
};

/* test header answer */

bigbed::HeaderType test1_header_ans
{

};

bigbed::HeaderType test2_header_ans
{

};

/* default bigbed answer */

bigbed::BBMemberType default_bb_ans
{
    std::string()
  , 0
  , 0
  , std::string()
};

/* test bigbed answer */

bigbed::BBMemberType test1_bb_ans
{

};

bigbed::BBMemberType test2_bb_ans
{

};

// TEST start

TEST(bb_header, constructor)
{
    using namespace bigbed;

    // default constructor
    Header def;
    static_loop_check_EQ<0>(def, default_header_ans);
    
    std::ifstream ifile("../test.bb", std::ios::binary);
    Header test1(ifile);

    /*
    // istream constructor
    std::ifstream ifile(test1_bb() , std::ios::binary);
    Header test1(ifile);
    static_loop_check_EQ<0>(test1, test1_header_ans);
    
    // copy constructor
    Header copy_test1(test1);
    static_loop_check_EQ<0>(copy_test1, test1_header_ans);

    // move constructor
    Header move_test1(std::move(copy_test1));
    static_loop_check_EQ<0>(move_test1, test1_header_ans);
    */
}

/*
TEST(bb_header, member_function)
{
    using namespace bigbed;

    // get_member()
    // Tested in static_loop_check_EQ()

    // reset()
    std::ifstream ifile(test1_bb() , std::ios::binary);
    Header test1(ifile);
    test1.reset();
    static_loop_check_EQ<0>(test1, default_header_ans);

    // preparse()
    // Tested in header constructor
}
*/

/*
TEST(bb_header, operators)
{
    using namespace bigbed;

    // operator>>
    std::ifstream ifile(test1_bb() , std::ios::binary);
    Header testh1;
    ifile >> testh1;
    static_loop_check_EQ<0>(testh1, test1_header_ans);
    ifile.close();

    // operator<<
    std::ofstream ofile("output.bb", std::ios::binary);
    ofile << testh1;
    ofile.close();
    ifile.open("output.bb", std::ios::binary);
    Header result(ifile);
    static_loop_check_EQ<0>(result, test1_header_ans);
    ifile.close();
}
*/

TEST(BigBed, constructor)
{
    using namespace bigbed;

    // header constructor
    Header def_h;
    BigBed def_bb(def_h);
    static_loop_check_EQ<0>(def_bb.get_header(), default_header_ans);
    static_loop_check_EQ<0>(def_bb, default_bb_ans);

    // copy constructor
    BigBed copy_bb(def_bb);
    static_loop_check_EQ<0>(copy_bb.get_header(), default_header_ans);
    static_loop_check_EQ<0>(copy_bb, default_bb_ans);

    // move constructor
    BigBed move_bb(std::move(copy_bb));
    static_loop_check_EQ<0>(move_bb.get_header(), default_header_ans);
    static_loop_check_EQ<0>(move_bb, default_bb_ans);
}

/*
TEST(BigBed, member_function)
{
    using namespace bigbed;

    // pre-settings
    std::ifstream ifile1(test1_bb() , std::ios::binary);
    Header testh1;
    ifile1 >> testh1;
    BigBed test1(testh1);

    std::ifstream ifile2(test2_bb() , std::ios::binary);
    Header testh2;
    ifile2 >> testh2;
    BigBed test2(testh2);

    // get_obj()
    BigBed::get_obj(ifile1, test1);
    static_loop_check_EQ<0>(test1, test1_bb_ans);
    ifile1.close();
    BigBed::get_obj(ifile2, test2_bb);
    static_loop_check_EQ<0>(test2, test2_bb_ans);
    ifile2.close();
    
    // get_member()
    // Tested in static_loop_check_EQ()

    // set_member()
    auto start_pos = test1.get_member<BB_MEMBER_INDEX::START_POS>()
    test1.set_member<BB_MEMBER_INDEX::START_POS>(100000);
    auto ans = test1.get_member<BB_MEMBER_INDEX::START_POS>();
    EXPECT_EQ(ans, 100000);
    test1.set_member<BB_MEMBER_INDEX::START_POS>(start_pos);

    // get_header()
    // Tested in header constructor

    // is_valid()
    EXPECT_TRUE(test1.is_valid());

    // toString()
    std::string temp = ""; // TO DO
    EXPECT_EQ(test1.to_string(), temp); 

    // dump()    
    std::ofstream ofile("output.bb", std::ios::binary);
    std::vector<BigBed> out_bb {test1, test2};
    BigBed::dump(ofile, out_bb);
    ofile.close();
    ifile1.open("output.bb");
    Header result_h(ifile1);
    BigBed result(result_h);
    BigBed::get_obj(ifile1, result);
    static_loop_check_EQ<0>(result, test1_bb_ans);
    BigBed::get_obj(ifile1, result);
    static_loop_check_EQ<0>(result, test2_bb_ans);
};
*/

/*
TEST(BigBed, operators)
{
    using namespace bigbed;

    // opearator>>
    std::ifstream ifile(test1_bb() , std::ios::binary);
    Header test_h;
    ifile >> test_h;
    BigBed test1(test_h);
    ifile >> test1;
    static_loop_check_EQ<0>(test1, test1_bb_ans);
    ifile.close();

    // operator<<
    std::ofstream ofile("output.bb", std::ios::binary);
    outfile << test_h;
    outfile << test1;
    outfile.close();

    ifile.open("output.bb");
    Header result_h(ifile);
    BigBed result(result_h);
    ifile >> result;
    static_loop_check_EQ<0>(result, test1_bb_ans);
    ifile.close();
}
*/

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
