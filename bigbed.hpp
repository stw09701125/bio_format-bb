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
        : header_ (HeaderType 
        { bigbed::BBIHeader {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        , bigbed::ChromList() 
        })
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
        void preparse(std::istream& input);

        friend std::istream& operator>>(std::istream& input, Header& rhs)
        {
            rhs.preparse(input);
            return input;
        }

        friend std::ostream& operator<<(std::ostream& output, const Header& rhs);

      private:
        HeaderType header_;
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
