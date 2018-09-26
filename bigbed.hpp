/**
 *  @file bigbed.hpp
 *  @brief a parser parsing the file in bigbed format
 *  @author JHHlab corp
 */

#pragma once

#include <iostream>
#include <tuple>
#include <vector>
#include <zlib.h>

namespace biovoltron::format{

namespace bigbed
{
    
    /**
     * @brief to get the underlying type of scoped enum in order to get value from tuple type using scoped enum
     * @param scoped enum member
     * @return underlying type of the scoped enum member
     */

    template<typename E>
    constexpr auto e_cast(E enumerator) noexcept
    {
	return static_cast<std::underlying_type_t<E>>(enumerator);
    }
    
    /**
     * @brief swap bytes from little endian to big endian
     * @param a parameter to be swapped
     * @return the swapped parameter
     */

    template<typename INT>
    INT swap_bytes(INT var)
    {
	char* var_temp = reinterpret_cast<char*>(&var);
	char temp[sizeof(INT)];
	for (std::size_t i = 0; i < sizeof(INT); ++i)
	{
	    temp[i] = var_temp[sizeof(INT) - i - 1];
	}
	return *(reinterpret_cast<INT*>(&temp[0]));
    }
    
    enum class BBI_INDEX
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
    
    enum class OFFSET_INDEX
    {
        OFFSET
      , SIZE
    };
    
    using Offset = std::tuple<
        std::uint64_t
      , std::uint64_t
    >;

    enum class CHROM_INDEX
    {
        NAME
      , ID
      , SIZE
      , OFFSET_LIST
    };

    using Chrom = std::tuple<
        std::string
      , std::uint32_t
      , std::uint32_t
      , std::vector<Offset>
    >;

    using ChromList = std::vector<Chrom>;

    enum class HEADER_INDEX
    {
	HEADER
      , CHROM_LIST
    };

    using HeaderType = std::tuple<
        BBIHeader
      , ChromList
    >;

    enum class MEMBER_INDEX
    {
        NAME
      , START
      , END
      , REST
    };

    using BBMemberType = std::tuple
    <
        std::string
      , std::uint32_t
      , std::uint32_t
      , std::string
    >;

    /**
     *  @brief The header class of bigbed
     *  Each bigbed file share the same header.
     *  The header class is used to constuct a bigbed object.
     * 
     *  If the bigbed objects with the same header 
     *  means they are from the same file. 
     */ 

    class Header
    {
      public:        
        Header()
        : chrom_id_( 0 )
	, offset_index_( 0 )
	, data_buf_(nullptr)
	, header_ (HeaderType { 
	  BBIHeader {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        , ChromList() 
        })
	, is_swapped_( false )
        {}
        
        Header(std::istream& input)
        : chrom_id_( 0 )
	, offset_index_( 0 )
	, data_buf_( nullptr )
	, is_swapped_( false )
	{
            preparse(input);
        }
        
        Header(const Header& rhs) = default;
        Header(Header&& rhs) = default;
        Header& operator=(const Header& rhs) = default;
        Header& operator=(Header&& rhs) = default;
        ~Header() = default;

        template <std::size_t n>
        auto get_member() const
        {
            return std::get<n>(header_);
        }

        void reset()
	{
	    chrom_id_ = 0;
	    offset_index_ = 0;
	    data_buf_ = nullptr;
	    is_swapped_ = false;
	    std::get<e_cast(HEADER_INDEX::HEADER)>(header_) = BBIHeader {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	    std::get<e_cast(HEADER_INDEX::CHROM_LIST)>(header_).clear();
	}
	
	void bb_read(std::istream& input, BBMemberType& bb_member)
	{
	    //if (data_buf_)
	    {
		// read from buffer
		// if data_buf_ >= buf_end(= data_buf_ + unc_size or offset_size)
		// then data_buf_ = nullptr
	    }
	    //else
	    {
		auto& chrom_list = std::get<e_cast(HEADER_INDEX::CHROM_LIST)>(header_);
		auto& chrom = chrom_list[chrom_id_];
		auto& offset_vector = std::get<e_cast(CHROM_INDEX::OFFSET_LIST)>(chrom);
		unsigned char* temp_buf;
		
		std::get<e_cast(MEMBER_INDEX::NAME)>(bb_member) = std::get<e_cast(CHROM_INDEX::NAME)>(chrom);

		if (offset_index_ == offset_vector.size())
		{
		    chrom_id_++;
		    chrom = chrom_list[chrom_id_];
		    offset_vector = std::get<e_cast(CHROM_INDEX::OFFSET_LIST)>(chrom);
		}
	    
		auto& offset = offset_vector[offset_index_];
		auto& offset_size = std::get<e_cast(OFFSET_INDEX::SIZE)>(offset);

		input.seekg(std::get<e_cast(OFFSET_INDEX::OFFSET)>(offset));
		input.read(reinterpret_cast<char*>(&temp_buf), offset_size);

		// uncompress data_buf_ (uncompressed buf and uncompressed size) 
		auto& h = std::get<e_cast(HEADER_INDEX::HEADER)>(header_);
		std::size_t data_buf_size = std::get<e_cast(BBI_INDEX::UNCOMPRESS_BUF_SIZE)>(h);
		std::size_t real_unc_size = zUncompress(temp_buf, offset_size, data_buf_, data_buf_size);
		//auto err = uncompress(unc_buf, &unc_size, data_buf_, offset_size);
		read_data_buf(bb_member, data_buf_);
		print_mem<0>(bb_member);
	    }
	}
        
	friend std::istream& operator>>(std::istream& input, Header& rhs)
        {
            rhs.preparse(input);
            return input;
        }

        friend std::ostream& operator<<(std::ostream& output, const Header& rhs);

      private:

	// for bigbed reading
	std::size_t chrom_id_;
	std::size_t offset_index_;
	unsigned char* data_buf_;
	
	// for header reading
	HeaderType header_;
        bool is_swapped_;
	void preparse(std::istream& input)
	{
	    read_bbi_data<std::istream, BBIHeader, 0>(input, std::get<e_cast(HEADER_INDEX::HEADER)>(header_));
	    //print_bbi<0>(std::get<e_cast(HEADER_INDEX::HEADER)>(header_));
	    auto& chrom_root_offset = std::get<e_cast(BBI_INDEX::CHROM_TREE_OFFSET)>(std::get<e_cast(HEADER_INDEX::HEADER)>(header_));
	    read_chrom_data(input, chrom_root_offset);
	    /*for (auto& i : std::get<e_cast(HEADER_INDEX::CHROM_LIST)>(header_))
	    {
		std::cout << std::get<e_cast(CHROM_INDEX::NAME)>(i) << std::endl; 
		std::cout << std::get<e_cast(CHROM_INDEX::ID)>(i) << std::endl; 
		std::cout << std::get<e_cast(CHROM_INDEX::SIZE)>(i) << std::endl; 
	    }*/
	    
	    // for ChromList rfind overlapping offset blocks
	    auto& data_index_offset = std::get<e_cast(BBI_INDEX::DATA_INDEX_OFFSET)>(std::get<e_cast(HEADER_INDEX::HEADER)>(header_));
	    read_data_blocks_offset(input, data_index_offset);       
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
	const bool is_overlapped(const T chrom, const T start, const T end, const T start_chrom, const T start_base, const T end_chrom, const T end_base) const
	{
	    return compare_overlapping(chrom, start, end_chrom, end_base) > 0 && compare_overlapping(chrom, end, start_chrom, start_base) < 0;
	}
	
	void r_read_Rtree(std::istream& file, const std::size_t& offset, Chrom& chrom)
	{
	    file.seekg(offset);    
	    
	    std::uint8_t is_leaf;
	    std::uint8_t reserved;
	    std::uint16_t child_num;
	    
	    auto& offset_list = std::get<e_cast(CHROM_INDEX::OFFSET_LIST)>(chrom);

	    const std::uint32_t start = 0;
	    const std::uint32_t& end = std::get<e_cast(CHROM_INDEX::SIZE)>(chrom);
	    const std::uint32_t& id = std::get<e_cast(CHROM_INDEX::ID)>(chrom);

	    file.read(reinterpret_cast<char*>(&is_leaf), sizeof(is_leaf));
	    file.read(reinterpret_cast<char*>(&reserved), sizeof(reserved));
	    file.read(reinterpret_cast<char*>(&child_num), sizeof(child_num));
	    
	    if (is_leaf)
	    {
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
		    }
		}
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

	    std::size_t root_offset = file.tellg();
	    auto& chrom_list = std::get<e_cast(HEADER_INDEX::CHROM_LIST)>(header_);
	    
	    for (auto& i : chrom_list)
	    {
		std::get<e_cast(CHROM_INDEX::OFFSET_LIST)>(i).reserve(items_per_slot);
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

		    auto& chrom_list = std::get<e_cast(HEADER_INDEX::CHROM_LIST)>(header_);
		    
		    file.read(reinterpret_cast<char*>(&(std::get<e_cast(CHROM_INDEX::ID)>(chrom_list[i]))), val_size);
		    file.read(reinterpret_cast<char*>(&(std::get<e_cast(CHROM_INDEX::SIZE)>(chrom_list[i]))), val_size);
		    
		    std::get<e_cast(CHROM_INDEX::NAME)>(chrom_list[i]) = temp_name;
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
	    std::get<e_cast(HEADER_INDEX::CHROM_LIST)>(header_).resize(item_count);
	    std::size_t root_offset = file.tellg();
	    
	    r_read_bpt(file, root_offset, key_size, val_size >> 1);
	}
	
	void read_data_buf(BBMemberType& bb_member, unsigned char* data_buf)
	{
	    std::get<e_cast(MEMBER_INDEX::START)>(bb_member) = *(reinterpret_cast<std::uint32_t*>(&data_buf[4])); 
	    std::get<e_cast(MEMBER_INDEX::END)>(bb_member) = *(reinterpret_cast<std::uint32_t*>(&data_buf[8]));
	    std::size_t len = strlen(reinterpret_cast<const char*>(data_buf));
	    std::get<e_cast(MEMBER_INDEX::REST)>(bb_member).append(reinterpret_cast<const char*>(data_buf)); 
	    /*std::size_t i = 9;
	    while(data_buf[i] != '\0')
	    {
		std::get<e_cast(MEMBER_INDEX::REST)>(bb_member).append() 
		++i;
	    }*/
	    data_buf += len;
	}

	std::size_t zUncompress(const unsigned char* c_buf, std::size_t c_size, unsigned char* unc_buf, std::size_t unc_size)
	{
	    uLongf uncomp_size = unc_size;
	    auto err = uncompress(unc_buf, &uncomp_size, c_buf, c_size);
	    if (err != 0) { std::cerr << "uncompress failed" << std::endl; }
	    return uncomp_size;
	}

	template<typename F, typename T, size_t n>
	static void read_bbi_data(F& file, T& data)
	{
	    file.read(reinterpret_cast<char*>(&(std::get<n>(data))), sizeof(decltype(std::get<n>(data))));
	    if constexpr (n != 11)
		read_bbi_data<F, T, n+1>(file, data);
	}
	
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

    class BigBed
    {
      public:
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

        BigBed(const BigBed& rhs) = default;
        BigBed(BigBed&& rhs) = default;
        BigBed& operator=(const BigBed& rhs) = default;
        BigBed& operator=(BigBed&& rhs) = default;
        ~BigBed() = default;

        template<std::size_t n>
        const auto& get_member() const
        {
            return std::get<n>(data_members_);
        }

        template <size_t n>
        void set_member(const std::tuple_element_t<n, BBMemberType>& rhs)
        {
            std::get<n>(data_members_) = rhs;
        }

        const Header& get_header() const
        {
            return header_;
        }

        bool is_valid() const
        {
            return has_data_;
        }

        void set_valid(bool value)
        {
            has_data_ = value;
        }

        void reset();

        std::string to_string();

        static std::istream& get_obj(std::istream& in, BigBed& obj)
	{
	    obj.header_.bb_read(in, obj.data_members_);
	    return in;
	}

        static void dump(std::ostream& out, std::vector<BigBed>& obj);

        friend std::istream& operator>>(std::istream& input, BigBed& rhs)
        {
            return get_obj(input, rhs);
        }

        friend std::ostream& operator<<(std::ostream& output, const BigBed& rhs);

      private:
        bool has_data_;
        Header& header_;
        BBMemberType data_members_;
    };
}; 

}
