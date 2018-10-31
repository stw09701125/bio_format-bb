/**
 *  @file bigbed.hpp
 *  @brief a parser parsing the file in bigbed format
 *  @author JHHlab corp
 */

#pragma once

#include <iostream>
#include <tuple>
#include <vector>
#include <exception>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/pipeline.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <zlib.h>

namespace biovoltron::format{

namespace bigbed
{
    /**
     * @class Exception
     * @brief An exception class which inherit from std::runtime_error
     *
     * The class Exception will be thrown if the class bigbed have any
     * behaviors in excepted situations.
     */
    class Exception : public std::runtime_error
    {
      public:
        Exception(const std::string& msg)
	  : runtime_error(std::string("BigBed Exception: " + msg))
	{
	}    
    };

    /**
     * @brief The enumerators help naming elements of tuple types
     * using in header class and bigbed class.
     */

    /**
     * @brief BBIHeader has 12 entries each stores header information
     * directly from the bigbed file.
     *
     * Although BBIHeader is a tuple type, it can be a struct type
     * combining enumerator BBI_INDEX and tuple BBIHeader.
     *
     * struct BBIHeader
     * {
     *	std::uint32_t MAGIC;
     *	std::uint16_t VERSION;
     *	std::uint16_t ZOOM_LEVELS;
     *	std::uint64_t CHROM_TREE_OFFSET;
     *	std::uint64_t DATA_OFFSET;
     *	std::uint64_t DATA_INDEX_OFFSET;
     *	std::uint16_t FIELD_COUNT;
     *  std::uint16_t DEFINED_FIELD;
     *	std::uint64_t SQL_OFFSET;
     *	std::uint64_t SUMMARY_OFFSET;
     *	std::uint32_t UNCOMPRESS_BUF_SIZE;
     *	std::uint64_t RESERVED;
     * }
     */

    namespace BBI_INDEX
    {
	enum BBI_INDEX
	{
	    MAGIC
	  , VERSION
	  , ZOOM_LEVELS
	  , CHROM_TREE_OFFSET
	  , DATA_OFFSET
	  , DATA_INDEX_OFFSET
	  , FIELD_COUNT
	  , DEFINED_FIELD
	  , SQL_OFFSET
	  , SUMMARY_OFFSET
	  , UNCOMPRESS_BUF_SIZE
	  , RESERVED
	};
    };

    using BBIHeader = std::tuple<
        std::uint32_t
      , std::uint16_t
      , std::uint16_t
      , std::uint64_t
      , std::uint64_t
      , std::uint64_t
      , std::uint16_t
      , std::uint16_t
      , std::uint64_t
      , std::uint64_t
      , std::uint32_t
      , std::uint64_t
    >;
    
    /**
     * @brief Offset is a tuple type stores the offset 
     * of a compressed block data.
     *
     * struct Offset
     * {
     *	std::uint64_t OFFSET;
     *	std::uint64_t SIZE;
     * }
     */

    namespace OFFSET_INDEX
    {
	enum OFFSET_INDEX
	{
	    OFFSET
	  , SIZE
	};
    };
    
    using Offset = std::tuple<
        std::uint64_t
      , std::uint64_t
    >;

    /**
     * @brief Chrom is a tuple type stores informations
     * of each chromosome including name, id, size, and offset_list.
     *
     * struct Chrom
     * {
     *	std::string NAME;
     *	std::uint32_t ID;
     *	std::uint32_t SIZE;
     *	std::vector<Offset> OFFSET_LIST;
     * }
     */
    namespace CHROM_INDEX
    {
	enum CHROM_INDEX
	{
	    NAME
	  , ID
	  , SIZE
	  , OFFSET_LIST
	};
    };

    using Chrom = std::tuple<
        std::string
      , std::uint32_t
      , std::uint32_t
      , std::vector<Offset>
    >;

    /**
     * @brief ChromList is just a vector of Chrom.
     */
    
    using ChromList = std::vector<Chrom>;


    /**
     * @brief HeaderType has all necessary informations
     * using in class Header.
     * 
     * BBIHeader: Header info.
     * ChromList: A vector of Chrom
     * , containing all informations of a chromosome,
     * to extract data from the bigbed file.
     * 
     * struct HeaderType
     * {
     *	BBIHeader HEADER;
     *	ChromList CHROM_LIST;
     * }
     */

    namespace HEADER_INDEX
    {
	enum HEADER_INDEX
	{
	    HEADER
	  , CHROM_LIST
	};
    };

    using HeaderType = std::tuple<
        BBIHeader
      , ChromList
    >;
    
    /**
     * @brief BBMemberType has a uncompressed bed data
     * which include name, start point, end point, and
     * the string of rest fields.
     * 
     * 
     * struct BBMemberType
     * {
     *	std::string NAME;
     *	std::uint32_t START;
     *	std::uint32_t END;
     *	std::string REST;
     * }
     */

    namespace MEMBER_INDEX
    {
	enum MEMBER_INDEX
	{
	    NAME
	  , START
	  , END
	  , REST
	};
    };

    using BBMemberType = std::tuple
    <
        std::string
      , std::uint32_t
      , std::uint32_t
      , std::string
    >;

    /**
     *	@class Header
     *  @brief The header class of bigbed
     *  Each bigbed file share the same header.
     *  The header class is used to constuct a bigbed object.
     * 
     *  If the bigbed objects with the same header 
     *  means they are from the same file. 
     *
     *  Member variable:
     *	    
     *	    // for bigbed reading
     *	    std::size_t data_count_;
     *	    std::size_t chrom_id_;
     *	    std::size_t offset_index_;
     *	    std::size_t pos_;
     *	    std::string data_buf_;
     *	    boost::iostreams::filtering_ostream un_zout_;
     *	
     *	    // for header reading
     *	    HeaderType header_;
     *	    std::string input_;
     *	    bool is_swapped_;
     *	    bool is_written_;
     *	    
     */ 

    class Header
    {
      public:        

	/** @brief Default constructor */
        
	Header()
        : data_count_( 0 )
	, chrom_id_( 0 )
	, offset_index_( 0 )
	, pos_( 0 )
	, data_buf_( "" )
	, input_( "" )
	, un_zout_( boost::iostreams::zlib_decompressor() )
	, header_ ( HeaderType { 
		    BBIHeader { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0}
		  , ChromList()
		  })
	, is_swapped_( false )
	, is_written_( false )
        {}
        
	/** 
	 * @brief istream constructor 
	 * Preparse header information into HeaderType.
	 */ 
        
	Header(std::istream& input)
        : data_count_( 0 )
	, chrom_id_( 0 )
	, offset_index_( 0 )
	, pos_( 0 )
	, data_buf_( "" )
	, input_( "" )
	, un_zout_( boost::iostreams::zlib_decompressor() )
	, is_swapped_( false )
	, is_written_( false )
	{
            preparse(input);
        }
        
	/** @brief copy constructor */
	
	Header(const Header& h)
        : data_count_( h.data_count_ )
	, chrom_id_( h.chrom_id_ )
	, offset_index_( h.offset_index_ )
	, pos_( h.pos_ )
	, data_buf_( h.data_buf_ )
	, input_( h.input_ )
	, un_zout_( boost::iostreams::zlib_decompressor() )
	, header_( h.header_ )
	, is_swapped_( h.is_swapped_ )
	, is_written_( h.is_written_ )
	{}

	/** @brief move constructor */
        
	Header(Header&& h)
        : data_count_( std::move(h.data_count_) )
	, chrom_id_( std::move(h.chrom_id_) )
	, offset_index_( std::move(h.offset_index_) )
	, pos_( std::move(h.pos_))
	, data_buf_( std::move(h.data_buf_) )
	, input_( std::move(h.input_) )
	, un_zout_( boost::iostreams::zlib_decompressor() )
	, header_( std::move(h.header_) )
	, is_swapped_( std::move(h.is_swapped_) )
	, is_written_( std::move(h.is_written_) )
	{}

	/** 
	 * @brief set copy assignment 
	 * and move assignment deleted.
	 */

        Header& operator=(const Header& rhs) = delete;
        Header& operator=(Header&& rhs) = delete;
        
        ~Header() = default;

	/**
	 * @brief get member from native header type - HeaderType
	 * template parameter n is the index to header.
	 */
        
	template <std::size_t n>
        auto get_member() const
        {
            return std::get<n>(header_);
        }
	
	/**
	 * @brief set writing flag to true. 
	 * if flag is true 
	 * means it is ready to output data to the file.
	 */

	void set_written()
	{
	    is_written_ = true;
	}

	/**
	 * @brief decrease data count of the header 
	 * while write a bed data to the file.
	 */

	void decrease_data_count()
	{
	    if (is_written_ && data_count_ > 0)
		--data_count_;
	    //std::cout << data_count_ << std::endl;
	}

	/** 
	 * @brief get data count of the header 
	 * to check if it is equal to 0. 
	 */

	auto get_data_count() const
	{
	    return data_count_;
	}

	/**
	 * @brief reset all members in header. 
	 */

        void reset()
	{
	    data_count_ = 0;
	    chrom_id_ = 0;
	    offset_index_ = 0;
	    pos_ = 0;
	    data_buf_ = "";
	    input_ = "";
	    is_swapped_ = false;
	    is_written_ = false;
	    std::get<HEADER_INDEX::HEADER>(header_) = 
		BBIHeader {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; 
	    std::get<HEADER_INDEX::CHROM_LIST>(header_).clear();
	}
	
	/**
	 *  @brief read a block of data 
	 *  and uncompress the block of data, putting in a data buffer.
	 *  If the data buffer has data, then read from buffer.
	 *  Or read a new block of data and uncompress them.
	 *
	 *  The data of a BigBed object will be read with bb_read,
	 *  storing read data in its member.
	 *   
	 *  @param input The istream to read data from.
	 *  @param bb_member The BigBed member type to store read data.
	 *
	 */

	void bb_read(std::istream& input, BBMemberType& bb_member)
	{
	    auto& chrom_list = 
		std::get<HEADER_INDEX::CHROM_LIST>(header_);
	    
	    if (data_buf_.size() > 0)
	    {
		std::get<MEMBER_INDEX::NAME>(bb_member) = 
		    std::get<CHROM_INDEX::NAME>(chrom_list[chrom_id_]);
		
		read_data_buf(bb_member);
	    }
	    else
	    {
		auto& offset_vector_origin = 
		    std::get<CHROM_INDEX::OFFSET_LIST>(chrom_list[chrom_id_]);
	    
		if (offset_index_ == offset_vector_origin.size())
		{
		    chrom_id_++;
		    offset_index_ = 0;
		}
		
		auto& offset_vector = 
		    std::get<CHROM_INDEX::OFFSET_LIST>(chrom_list[chrom_id_]);
		
		std::get<MEMBER_INDEX::NAME>(bb_member) = 
		    std::get<CHROM_INDEX::NAME>(chrom_list[chrom_id_]);
	    
		auto& offset = offset_vector[offset_index_];
		
		auto& offset_size = 
		    std::get<OFFSET_INDEX::SIZE>(offset);
		
		std::string temp_buf("", offset_size);
		input.seekg(std::get<OFFSET_INDEX::OFFSET>(offset));
		input.read(temp_buf.data(), offset_size);

		//boost::iostreams::filtering_ostream un_zout(boost::iostreams::zlib_decompressor() | boost::iostreams::back_inserter(data_buf_));
		un_zout_.push(boost::iostreams::back_inserter(data_buf_));
		boost::iostreams::copy(boost::make_iterator_range(temp_buf), un_zout_);
		
		read_data_buf(bb_member);
		offset_index_++;
	    }
	}
       
	/**
	 * @brief Actually write all data to the output file.
	 *
	 * @param output The ostream that keep data.
	 */

	void write_to_file(std::ostream& output) const
	{
	    output << input_;
	}

	/**
	 * @brief Operator>> that get header data from the file.
	 *  
	 * @param input The istream to be read.
	 * @param rhs The header to store header information.
	 *
	 * @return The istream.
	 */

	friend std::istream& operator>>(std::istream& input, Header& rhs)
        {
            rhs.preparse(input);
            return input;
        }

	/**
	 * @brief Operator<< that set writting flag to true.
	 *
	 * @param output The ostream parameter 
	 * keep API the same but do nothing.
	 *
	 * @param rhs The header.
	 *
	 * @return The ostream.
	 */

        friend std::ostream& operator<<(std::ostream& output, Header& rhs)
	{
	    if (rhs.is_written_ == false)
		rhs.is_written_ = true;
	    return output;
	}

      private:

	// for bigbed reading
	std::size_t data_count_;
	std::size_t chrom_id_;
	std::size_t offset_index_;
	std::size_t pos_;
	std::string data_buf_;
	boost::iostreams::filtering_ostream un_zout_;
	
	// for header reading
	HeaderType header_;
	std::string input_;
	bool is_swapped_;
	bool is_written_;
	
	void preparse(std::istream& input)
	{
	    
	    if (!input.good())
		throw Exception("ERROR: preparse(): file error\n");
	    if (input.peek() == std::ifstream::traits_type::eof())
		throw Exception("ERROR: preparse(): empty file\n");
	    
	    reset();
	    read_whole_data(input);
	    //std::cout << input_ << std::endl;
	    auto& h = std::get<HEADER_INDEX::HEADER>(header_);
	    
	    read_bbi_data<BBIHeader, 0>(input_, h, pos_);
	    //print_bbi<0>(h);
	    auto& chrom_root_offset = 
		std::get<BBI_INDEX::CHROM_TREE_OFFSET>(h);
	    read_chrom_data(input, chrom_root_offset);
	    
	    /*auto& chrom_list = std::get<HEADER_INDEX::CHROM_LIST>(header_);
	    
	    for (auto i : chrom_list)
	    {
		std::cout << "name: " << std::get<CHROM_INDEX::NAME>(i) << ", chr_id: " << std::get<CHROM_INDEX::ID>(i) << ", chr_size: " << std::get<CHROM_INDEX::SIZE>(i) << std::endl;
	    }*/
	    
	    //auto now = input.tellg();
	    //std::uint32_t dc = 0;
	    //dc = *(reinterpret_cast<std::uint32_t*>(input_.data() + now));
	    //std::cout << "dc: " << dc << std::endl;
	    
	    input.read(reinterpret_cast<char*>(&data_count_), sizeof(std::uint32_t));
	    
	    if (data_count_ == 0)
	    {
		throw Exception(
			"ERROR: preparse(): " 
			"the number of chromosomes is bigger than 256\n"
			);
	    }
	    
	    //std::cout << "data_count" << data_count_ << std::endl;

	    // for ChromList rfind overlapping offset blocks
	    auto& data_index_offset = 
		std::get<BBI_INDEX::DATA_INDEX_OFFSET>(h);
	    read_data_blocks_offset(input, data_index_offset);       
	    
	    auto& chrom_list = std::get<HEADER_INDEX::CHROM_LIST>(header_);
	    
	    /*for (auto i : chrom_list)
	    {
		std::cout << "{ name: " << std::get<CHROM_INDEX::NAME>(i) << ", chr_id: " << std::get<CHROM_INDEX::ID>(i) << ", chr_size: " << std::get<CHROM_INDEX::SIZE>(i) << ", offset_list: ";
		for (auto v : std::get<CHROM_INDEX::OFFSET_LIST>(i))
		{
		    std::cout << "{ " << std::get<OFFSET_INDEX::OFFSET>(v) << ", " << std::get<OFFSET_INDEX::SIZE>(v) << " }";
		}
		std::cout << " }" << std::endl;
	    }*/
	}

	template<typename T>
	const auto compare_overlapping(const T l_hi, const T l_lo, const T r_hi, const T r_lo) const
	{
	    if (l_hi < r_hi) return 1;
	    else if (l_hi > r_hi) return -1;
	    else
	    {
		if (l_lo < r_lo) return 1;
		else if (l_lo > r_lo) return -1;
		else return 0;
	    }
	}

	template<typename T>
	const bool is_overlapped(const T chrom, const T start, const T end
		, const T start_chrom, const T start_base, const T end_chrom
		, const T end_base) const
	{
	    return compare_overlapping(chrom, start, end_chrom, end_base) > 0 
		&& compare_overlapping(chrom, end, start_chrom, start_base) < 0;
	}
	
	void r_read_Rtree(std::istream& file, const std::size_t& offset, Chrom& chrom)
	{
	    file.seekg(offset);    
	    
	    std::uint8_t is_leaf;
	    std::uint8_t reserved;
	    std::uint16_t child_num;
	    
	    auto& offset_list = std::get<CHROM_INDEX::OFFSET_LIST>(chrom);

	    const std::uint32_t start = 0;
	    const std::uint32_t& end = std::get<CHROM_INDEX::SIZE>(chrom);
	    const std::uint32_t& id = std::get<CHROM_INDEX::ID>(chrom);

	    file.read(reinterpret_cast<char*>(&is_leaf), sizeof(is_leaf));
	    file.read(reinterpret_cast<char*>(&reserved), sizeof(reserved));
	    file.read(reinterpret_cast<char*>(&child_num), sizeof(child_num));
	    
	    if (is_leaf)
	    {
		//std::cout << "Name: " << std::get<CHROM_INDEX::NAME>(chrom) << ", ID: " << std::get<CHROM_INDEX::ID>(chrom) << ", List: ";
		for (std::size_t i = 0; i < child_num; ++i)
		{
		    std::uint32_t start_chrom_ix;
		    std::uint32_t start_base;
		    std::uint32_t end_chrom_ix;
		    std::uint32_t end_base;
		    std::uint64_t block_offset;
		    std::uint64_t block_size;
		    
		    file.read(reinterpret_cast<char*>(&start_chrom_ix), sizeof(start_chrom_ix));
		    file.read(reinterpret_cast<char*>(&start_base), sizeof(start_base));
		    file.read(reinterpret_cast<char*>(&end_chrom_ix), sizeof(end_chrom_ix));
		    file.read(reinterpret_cast<char*>(&end_base), sizeof(end_base));
		    file.read(reinterpret_cast<char*>(&block_offset), sizeof(block_offset));
		    file.read(reinterpret_cast<char*>(&block_size), sizeof(block_size));
		    
		    if (is_overlapped(id, start, end, start_chrom_ix, start_base, end_chrom_ix, end_base))
		    {
			offset_list.push_back({block_offset, block_size});
			//std::cout << "{ Name: " << std::get<CHROM_INDEX::NAME>(chrom) << ", ID: " << std::get<CHROM_INDEX::ID>(chrom);
			//std::cout << "{ " << block_offset << ", " << block_size << " } } ";
		    }
		}
		//std::cout << std::endl;
	    }
	    
	    else
	    {
		std::vector<std::uint32_t> start_chrom_ixs;
		std::vector<std::uint32_t> start_bases;
		std::vector<std::uint32_t> end_chrom_ixs;
		std::vector<std::uint32_t> end_bases;
		std::vector<std::uint64_t> offsets;

		start_chrom_ixs.resize(child_num);
		start_bases.resize(child_num);
		end_chrom_ixs.resize(child_num);
		end_bases.resize(child_num);
		offsets.resize(child_num);

		for (std::size_t i = 0; i < child_num; ++i)
		{
		    file.read(reinterpret_cast<char*>(&start_chrom_ixs[i]), sizeof(start_chrom_ixs[i]));
		    file.read(reinterpret_cast<char*>(&start_bases[i]), sizeof(start_bases[i]));
		    file.read(reinterpret_cast<char*>(&end_chrom_ixs[i]), sizeof(end_chrom_ixs[i]));
		    file.read(reinterpret_cast<char*>(&end_bases[i]), sizeof(end_bases[i]));
		    file.read(reinterpret_cast<char*>(&offsets[i]), sizeof(offsets[i]));
		}
		
		for (std::size_t i = 0; i < child_num; ++i)
		{
		    if (is_overlapped(id, start, end, start_chrom_ixs[i], start_bases[i], end_chrom_ixs[i], end_bases[i]))
		    {
			r_read_Rtree(file, offsets[i], chrom);
		    }
		}
	    }
	}

	void read_data_blocks_offset(std::istream& file, const std::size_t& offset)
	{
	    file.seekg(offset);
	    std::uint32_t magic;
	    std::uint32_t block_size;
	    std::uint64_t item_count;
	    std::uint32_t start_chrom_ix;
	    std::uint32_t start_base_offset;
	    std::uint32_t end_chrom_ix;
	    std::uint32_t end_base_offset;
	    std::uint64_t data_end_offset;
	    std::uint32_t items_per_slot;
	    std::uint32_t reserved;

	    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
	    file.read(reinterpret_cast<char*>(&block_size), sizeof(block_size));
	    file.read(reinterpret_cast<char*>(&item_count), sizeof(item_count));
	    file.read(reinterpret_cast<char*>(&start_chrom_ix), sizeof(start_chrom_ix));
	    file.read(reinterpret_cast<char*>(&start_base_offset), sizeof(start_base_offset));
	    file.read(reinterpret_cast<char*>(&end_chrom_ix), sizeof(end_chrom_ix));
	    file.read(reinterpret_cast<char*>(&end_base_offset), sizeof(end_base_offset));
	    file.read(reinterpret_cast<char*>(&data_end_offset), sizeof(data_end_offset));
	    file.read(reinterpret_cast<char*>(&items_per_slot), sizeof(items_per_slot));
	    file.read(reinterpret_cast<char*>(&reserved), sizeof(reserved));
	    //std::cout << "item count from R tree: " << item_count << std::endl;
	    std::size_t root_offset = file.tellg();
	    auto& chrom_list = std::get<HEADER_INDEX::CHROM_LIST>(header_);
	    
	    for (auto& i : chrom_list)
	    {
		//std::cout << "Name: " << std::get<CHROM_INDEX::NAME>(i) << ", ID: " << std::get<CHROM_INDEX::ID>(i) << std::endl;
		std::get<CHROM_INDEX::OFFSET_LIST>(i).reserve(items_per_slot);
		r_read_Rtree(file, root_offset, i);
	    }
	}
	
	void r_read_bpt(std::istream& file, const std::size_t& offset, const std::uint32_t& key_size, const std::uint32_t& val_size)
	{
	    file.seekg(offset);
	    
	    std::uint8_t is_leaf;
	    std::uint8_t reserved;
	    std::uint16_t child_num;

	    file.read(reinterpret_cast<char*>(&is_leaf), sizeof(is_leaf));
	    file.read(reinterpret_cast<char*>(&reserved), sizeof(reserved));
	    file.read(reinterpret_cast<char*>(&child_num), sizeof(child_num));

	    char temp_name[key_size];
	    
	    if (is_leaf)
	    {
		for (std::size_t i = 0; i < child_num; ++i)
		{
		    file.read(temp_name, key_size);
		    temp_name[key_size] = '\0';

		    auto& chrom_list = std::get<HEADER_INDEX::CHROM_LIST>(header_);
		    std::uint32_t cid;
		    file.read(reinterpret_cast<char*>(&cid), val_size);
		    std::get<CHROM_INDEX::ID>(chrom_list[cid]) = cid;
		    //file.read(reinterpret_cast<char*>(&(std::get<CHROM_INDEX::ID>(chrom_list[i]))), val_size);
		    file.read(reinterpret_cast<char*>
			(&(std::get<CHROM_INDEX::SIZE>(chrom_list[cid]))), val_size);
		    
		    std::get<CHROM_INDEX::NAME>(chrom_list[cid]) = temp_name;
		    
		    //std::cout << "{ name: " << std::get<CHROM_INDEX::NAME>(chrom_list[i]) << ", chr_id: " << std::get<CHROM_INDEX::ID>(chrom_list[i]) << ", chr_size: " << std::get<CHROM_INDEX::SIZE>(chrom_list[i]) << " }" << std::endl;
		}
	    }
	    
	    else
	    {
		std::vector<std::uint64_t> child_offsets;
		child_offsets.resize(child_num);
		for (std::size_t i = 0; i < child_num; ++i)
		{
		    file.read(temp_name, key_size);
		    file.read(reinterpret_cast<char*>(&child_offsets[i]), sizeof(child_offsets[i]));
		}
		
		for (std::size_t i = 0; i < child_num; ++i)
		{
		    r_read_bpt(file, child_offsets[i], key_size, val_size);
		}
	    }
	}

	void read_chrom_data(std::istream& file, const std::size_t& offset)
	{
	    file.seekg(offset);
	    std::uint32_t magic;
	    std::uint32_t block_size;
	    std::uint32_t key_size;
	    std::uint32_t val_size;
	    std::uint64_t item_count;
	    std::uint64_t reserved;

	    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
	    file.read(reinterpret_cast<char*>(&block_size), sizeof(block_size));
	    file.read(reinterpret_cast<char*>(&key_size), sizeof(key_size));
	    file.read(reinterpret_cast<char*>(&val_size), sizeof(val_size));
	    file.read(reinterpret_cast<char*>(&item_count), sizeof(item_count));
	    file.read(reinterpret_cast<char*>(&reserved), sizeof(reserved));
	    std::get<HEADER_INDEX::CHROM_LIST>(header_).resize(item_count);
	    //std::cout << "item_count: " << item_count << std::endl;
	    std::size_t root_offset = file.tellg();
	    
	    r_read_bpt(file, root_offset, key_size, val_size >> 1);
	}
	
	void read_data_buf(BBMemberType& bb_member)
	{
	    std::get<MEMBER_INDEX::START>(bb_member) = 
		*(reinterpret_cast<std::uint32_t*>(data_buf_.data() + 4)); 
	    std::get<MEMBER_INDEX::END>(bb_member) = 
		*(reinterpret_cast<std::uint32_t*>(data_buf_.data() + 8));
	    std::size_t i = 12;
	    while(data_buf_[i] != '\0')
	    {
		std::get<MEMBER_INDEX::REST>(bb_member).push_back(data_buf_[i]); 
		++i;
	    }
	    data_buf_ = std::string_view(&data_buf_[i+1], data_buf_.size() - i - 1);
	}

	void read_whole_data(std::istream& input)
	{
	   input.seekg(0, std::ios::end);
	   std::size_t end_pos = input.tellg();
	   input.seekg(0, std::ios::beg);
	   input_.resize(end_pos);
	   input.read(input_.data(), end_pos);
	}

	template<typename T, size_t n>
	static void read_bbi_data(std::string& file, T& data, std::size_t& pos)
	{
	    using TUPLETYPE = std::remove_reference_t<decltype(std::get<n>(data))>;
	    std::get<n>(data) = 
		*(reinterpret_cast<TUPLETYPE*>(file.data() + pos));
	    pos += sizeof(TUPLETYPE);
	    if constexpr (n != 11)
		read_bbi_data<T, n+1>(file, data, pos);
	}

	/*template<typename F, typename T, size_t n>
	static void read_bbi_data(F& file, T& data)
	{
	    file.read(reinterpret_cast<char*>(&(std::get<n>(data)))
		    , sizeof(decltype(std::get<n>(data))));
	    if constexpr (n != 11)
		read_bbi_data<F, T, n+1>(file, data);
	}*/
	
	template<size_t n>
	static void print_bbi(const BBIHeader& bbi)
	{
	    if constexpr (n == 0)
		std::cout << std::hex << std::get<n>(bbi) << std::endl;
	    if constexpr (n != 0)
		std::cout << std::dec << std::get<n>(bbi) << std::endl;
	    if constexpr (n != 11)
		print_bbi<n+1>(bbi);
	}	
	
	template<size_t n>
	static void print_mem(const BBMemberType& bb_member)
	{
	    std::cout << std::get<n>(bb_member) << std::endl;
	    if constexpr (n != 3)
		print_mem<n+1>(bb_member);
	}	
    };
    
    /**
     *	@class BigBed
     *  @brief a bigbed object has a bed data
     *  in its data member.
     *
     *
     *  Member variable:
     *	    
     *	    bool has_data_;
     *	    Header& header_;
     *	    BBMemberType data_members_;
     *	    
     */ 

    class BigBed
    {
      public:

	/** @brief default constructor */
        
	BigBed(Header& header)
        : header_       ( header )
        , has_data_     ( false )
        , data_members_ ( BBMemberType {
                            std::string()
                          , 0
                          , 0
                          , std::string()   
                        })
        {}

	/**
	 * @brief set copy & move constructor to default.
	 * set copy & move assignment to delete.
	 */
        
	BigBed(const BigBed& bb) = default;
        BigBed(BigBed&& bb) = default;
        BigBed& operator=(const BigBed& rhs) = delete;
        BigBed& operator=(BigBed&& rhs) = delete;
        ~BigBed() = default;

	/**
	 * @brief get an entry of data members in index n.
	 */

        template<std::size_t n>
        const auto& get_member() const
        {
            return std::get<n>(data_members_);
        }

	/**
	 * @brief assign a value to a specific data member.
	 *
	 * @param value Same type with the data member in index n.
	 */

        template <size_t n>
        void set_member(const std::tuple_element_t<n, BBMemberType>& value)
        {
            std::get<n>(data_members_) = value;
        }

	/**
	 * @brief get header.
	 *
	 * @return Header reference of header_.
	 */

        const Header& get_header() const
        {
            return header_;
        }

	/**
	 * @brief check if this object has data.
	 *
	 * @return bool has_data_.
	 */

        bool is_valid() const
        {
            return has_data_;
        }

	/**
	 * @brief set has_data_ to value.
	 *
	 * @param bool value assign the bool value to has_data_.
	 */

        void set_valid(bool value)
        {
            has_data_ = value;
        }

	/** @brief reset data */

        void reset()
	{
	    has_data_ = false;
	    data_members_ = BBMemberType { std::string(), 0, 0, std::string() };
	}

	/**
	 * @brief convert the bed data in the object
	 * to string.
	 *
	 * @return the string of a bed data.
	 */

        std::string to_string() 
	{
	    if (!has_data_) return std::string();
	    std::string result;
	    to_string_impl<0>(result);
	    return result;
	}

	/**
	 * @brief get data from input bigbed file and 
	 * set has_data_ to true.
	 * 
	 * @param istream in: input bigbed file
	 * @param BigBed obj: destination to store data
	 *
	 * @return istream reference
	 */

        static std::istream& get_obj(std::istream& in, BigBed& obj)
	{
	    if (obj.is_valid()) obj.reset();
	    obj.header_.bb_read(in, obj.data_members_);
	    obj.set_valid(true);
	    return in;
	}

	/**
	 * @brief dump a vector of BigBed objects 
	 * which from the same file.
	 *
	 * @param ostream out: output file to write to.
	 * @param vector<BigBed> obj: a set of data to be wrote.
	 *
	 */

        static void dump(std::ostream& out, std::vector<BigBed>& obj)
	{
	    obj.front().header_.set_written();
	    for (auto& i : obj)
	    {
		out << i;
	    }
	}

	/**
	 * @brief operator>>: get data from input bigbed file with get_obj function.
	 *
	 * @param istream input: input file to read from.
	 * @param BigBed rhs: destination to store bed data.
	 *
	 * @return istream reference
	 */

        friend std::istream& operator>>(std::istream& input, BigBed& rhs)
        {
	    return get_obj(input, rhs);
        }

	/**
	 * @brief operator<<: write data to output bigbed file from rhs.
	 * However, only if whole data are written to the file, it writes.
	 * It decrease data count in the header and check if data count 
	 * equal to 0, then writes to file.
	 *
	 * @param ostream output: output file to write to.
	 * @param BigBed rhs: data to be wrote.
	 *
	 * @return ostream reference output
	 */

        friend std::ostream& operator<<(std::ostream& output, BigBed& rhs)
	{
	    if (rhs.has_data_) 
	    {
		rhs.header_.decrease_data_count();
		//rhs.reset();
	    }
	    if (rhs.header_.get_data_count() == 0) 
		rhs.header_.write_to_file(output);
	    return output;
	}

      private:
        
	template<std::size_t n>
	void to_string_impl(std::string& result)
	{
	    if constexpr (n == MEMBER_INDEX::NAME)
	    {
		result.append(get_member<n>());
		result.append("\t");
	    }
	    else if constexpr (n == MEMBER_INDEX::START || n == MEMBER_INDEX::END)
	    {
		result.append(std::to_string(get_member<n>()));
		result.append("\t");
	    }
	    else
	    {
		result.append(get_member<n>());
		result.append("\n");
	    }
	    if constexpr (n != 3) 
		to_string_impl<n+1>(result);
	}
	
	bool has_data_;
        Header& header_;
        BBMemberType data_members_;
    };
}; 

}
