
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
