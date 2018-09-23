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
        : header_ (HeaderType { 
	  bigbed::BBIHeader {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        , bigbed::ChromList() 
        })
	, is_swapped_( false )
        {}
        
        Header(std::istream& input)
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

        void reset();

        friend std::istream& operator>>(std::istream& input, Header& rhs)
        {
            rhs.preparse(input);
            return input;
        }

        friend std::ostream& operator<<(std::ostream& output, const Header& rhs);

      private:
        HeaderType header_;
        bool is_swapped_;
	void preparse(std::istream& input)
	{
	    read_bbi_data<std::istream, BBIHeader, 0>(input, std::get<e_cast(HEADER_INDEX::HEADER)>(header_));
	    //print_bbi<0>(std::get<e_cast(HEADER_INDEX::HEADER)>(header_));
	    auto chrom_root_offset = std::get<e_cast(BBI_INDEX::CHROM_TREE_OFFSET)>(std::get<e_cast(HEADER_INDEX::HEADER)>(header_));
	    read_chrom_data(input, chrom_root_offset);
	    /*for (auto& i : std::get<e_cast(HEADER_INDEX::CHROM_LIST)>(header_))
	    {
		std::cout << std::get<e_cast(CHROM_INDEX::NAME)>(i) << std::endl; 
		std::cout << std::get<e_cast(CHROM_INDEX::ID)>(i) << std::endl; 
		std::cout << std::get<e_cast(CHROM_INDEX::SIZE)>(i) << std::endl; 
	    }*/
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
        auto get_member() const
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

        static std::istream& get_obj(std::istream& in, BigBed& obj);

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
