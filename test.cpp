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
    bigbed::BBIHeader {0x8789f2eb, 4, 2, 1142, 2046, 4179, 12, 12, 304, 1038, 16384, 1078}
  , bigbed::ChromList 
    {
	{"chr1", 0, 249250621, {{2054, 87}}}
      , {"chr10", 1, 135534747, {{2141, 74}}}
      ,	{"chr11", 2, 135006516, {{2215, 78}}}
      ,	{"chr12", 3, 133851895, {{2293, 74}}}
      ,	{"chr13", 4, 115169878, {{2367, 75}}}
      , {"chr14", 5, 107349540, {{2442, 49}}}
      , {"chr15", 6, 102531392, {{2491, 47}}}
      , {"chr16", 7, 90354753, {{2538, 78}}}
      , {"chr17", 8, 81195210, {{2616, 45}}}
      , {"chr17_ctg5_hap1", 9, 1680828, {{2661, 44}}}
      ,	{"chr18", 10, 78077248, {{2705, 46}}}
      , {"chr19", 11, 59128983, {{2751, 45}}}
      , {"chr1_gl000192_random", 12, 547496, {{2796, 45}}}
      , {"chr2", 13, 243199373, {{2841, 75}}}
      , {"chr20", 14, 63025520, {{2916, 47}}}
      , {"chr21", 15, 48129895, {{2963, 90}}}
      , {"chr22", 16, 51304566, {{3053, 92}}}
      ,	{"chr3", 17, 198022430, {{3145, 90}}}
      , {"chr4", 18, 191154276, {{3235, 93}}}
      , {"chr5", 19, 180915260, {{3328, 88}}}
      , {"chr6", 20, 171115067, {{3416, 73}}}
      , {"chr7", 21, 159138663, {{3489, 85}}}
      , {"chr8", 22, 146364022, {{3574, 73}}}
      , {"chr9", 23, 141213431, {{3647, 88}}}
      , {"chrM", 24, 16571, {{3735, 54}}}
      , {"chrUn_gl000220", 25, 161802, {{3789, 76}}}
      , {"chrUn_gl000223", 26, 180455, {{3865, 44}}}
      , {"chrUn_gl000228", 27, 129120, {{3909, 46}}}
      , {"chrUn_gl000229", 28, 19913, {{3955, 57}}}
      , {"chrX", 29, 155270560, {{4012, 78}}}
      , {"chrY", 30, 59373566, {{4090, 89}}}
    }
};

bigbed::HeaderType test2_header_ans
{
    bigbed::BBIHeader {0x8789f2eb, 4, 1, 1142, 1190, 1255, 12, 12, 304, 1038, 16384, 1078}
  , bigbed::ChromList { {"chr1", 0, 248956422, {{1198, 57}} } }
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
    "chr1"
  , 1815107
  , 1815204
  , "LSU-rRNA_Hsa\t0\t+\t1815107\t1815204\t0\t1\t97\t0"
};

bigbed::BBMemberType test2_bb_ans
{
    "chr1"
  , 4417098
  , 4417211
  , "LSU-rRNA_Hsa\t0\t-\t4417098\t4417211\t0\t1\t113\t0"
};

bigbed::BBMemberType test3_bb_ans
{
    "chr10"
  , 200793
  , 200880
  , "5S\t0\t-\t200793\t200880\t0\t1\t87\t0"
};

bigbed::BBMemberType test4_bb_ans
{
    "chr10"
  , 327975
  , 328065
  , "5S\t0\t-\t327975\t328065\t0\t1\t90\t0"
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
    static_loop_check_EQ<0>(test1, test1_header_ans);

    
    // istream constructor
    //std::ifstream ifile(test1_bb() , std::ios::binary);
    //Header test1(ifile);
    //static_loop_check_EQ<0>(test1, test1_header_ans);
    
    // copy constructor
    Header copy_test1(test1);
    static_loop_check_EQ<0>(copy_test1, test1_header_ans);

    // move constructor
    Header move_test1(std::move(copy_test1));
    static_loop_check_EQ<0>(move_test1, test1_header_ans);
}


TEST(bb_header, member_function)
{
    using namespace bigbed;

    // get_member()
    // Tested in static_loop_check_EQ()

    // reset()
    std::ifstream ifile("../test.bb", std::ios::binary); 
    //std::ifstream ifile(test1_bb() , std::ios::binary);
    Header test1(ifile);
    test1.reset();
    static_loop_check_EQ<0>(test1, default_header_ans);

    // preparse()
    // Tested in header constructor
}



TEST(bb_header, operators)
{
    using namespace bigbed;

    // operator>>
    std::ifstream ifile("../test.bb", std::ios::binary);
    //std::ifstream ifile(test1_bb() , std::ios::binary);
    Header testh1;
    ifile >> testh1;
    static_loop_check_EQ<0>(testh1, test1_header_ans);
    ifile.close();

    // operator<<
    /*std::ofstream ofile("output.bb", std::ios::binary);
    ofile << testh1;
    ofile.close();
    ifile.open("output.bb", std::ios::binary);
    Header result(ifile);
    static_loop_check_EQ<0>(result, default_header_ans);
    ifile.close();*/
}


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

TEST(BigBed, member_function)
{
    using namespace bigbed;
    
    // pre-settings
    std::ifstream ifile("../test.bb", std::ios::binary);
    Header testh1(ifile);
    BigBed test1(testh1);
    BigBed test2(testh1);
    BigBed test3(testh1);
    BigBed test4(testh1);

    // get_obj()
    BigBed::get_obj(ifile, test1);
    BigBed::get_obj(ifile, test2);
    BigBed::get_obj(ifile, test3);
    BigBed::get_obj(ifile, test4);
    
    static_loop_check_EQ<0>(test1.get_header(), test1_header_ans);
    static_loop_check_EQ<0>(test1, test1_bb_ans);
    static_loop_check_EQ<0>(test2.get_header(), test1_header_ans);
    static_loop_check_EQ<0>(test2, test2_bb_ans);
    static_loop_check_EQ<0>(test3.get_header(), test1_header_ans);
    static_loop_check_EQ<0>(test3, test3_bb_ans);
    static_loop_check_EQ<0>(test4.get_header(), test1_header_ans);
    static_loop_check_EQ<0>(test4, test4_bb_ans);
    
    // get_member()
    // Tested in static_loop_check_EQ()
    
    // set_member()
    auto start_pos = test1.get_member<MEMBER_INDEX::START>();
    test1.set_member<MEMBER_INDEX::START>(100000);
    auto ans = test1.get_member<MEMBER_INDEX::START>();
    EXPECT_EQ(ans, 100000);
    test1.set_member<MEMBER_INDEX::START>(start_pos);
    
    // get_header()
    // Tested in header constructor
    
    // is_valid()
    EXPECT_TRUE(test1.is_valid());
    
    // to_string()
    std::string temp = 
    "chr1\t1815107\t1815204\tLSU-rRNA_Hsa\t0\t+\t1815107\t1815204\t0\t1\t97\t0\n";
    EXPECT_EQ(test1.to_string(), temp); 
    
    // dump()
    std::cout << "dump start" << std::endl;
    std::ifstream ifile2("../two_line.bb", std::ios::binary);
    Header testh2(ifile2);
    BigBed dump1(testh2);
    BigBed dump2(testh2);
    ifile2 >> dump1 >> dump2;
    ifile2.close();
    std::ofstream ofile("output.bb", std::ios::binary);
    std::vector<BigBed> out_bb {dump1, dump2};
    BigBed::dump(ofile, out_bb);
    ofile.close();
    ifile2.open("output.bb");
    Header result_h(ifile2);
    BigBed result(result_h);
    BigBed::get_obj(ifile2, result);
    static_loop_check_EQ<0>(result, test1_bb_ans);
    BigBed::get_obj(ifile2, result);
    static_loop_check_EQ<0>(result, test3_bb_ans); 
}

TEST(BigBed, operators)
{
    using namespace bigbed;

    // opearator>>
    std::ifstream ifile("../one_line.bb", std::ios::binary);
    //std::ifstream ifile(test1_bb() , std::ios::binary);
    Header test_h;
    ifile >> test_h;
    BigBed test2(test_h);
    ifile >> test2;
    static_loop_check_EQ<0>(test_h, test2_header_ans);
    static_loop_check_EQ<0>(test2, test2_bb_ans);
    ifile.close();

    // operator<<
    std::ofstream ofile("output.bb", std::ios::binary);
    ofile << test_h;
    ofile << test2;
    ofile.close();

    ifile.open("output.bb");
    Header result_h(ifile);
    BigBed result(result_h);
    ifile >> result;
    static_loop_check_EQ<0>(result, test2_bb_ans);
    ifile.close();
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
