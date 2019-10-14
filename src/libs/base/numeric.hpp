#ifndef BASE_NUMERIC_HPP
#define BASE_NUMERIC_HPP

#include <cstddef>
#include <type_traits>


namespace base
{


/**
 * Count the number of significant bits in an integer.
 *
 * For example, 00010010 contains 5 significant bits.
 *
 * @param p_value Integer to analyze the bits of.
 * @return Number of significant bits.
 */
template < typename T >
typename std::enable_if < std::is_integral < T > ::value, std::size_t > ::type calculate_num_significant_bits(T p_value)
{
	std::size_t num_bits = 0;
	while (p_value != 0)
	{
		++num_bits;
		p_value >>= 1;
	}
	return num_bits;
}


} // namespace base end


#endif // BASE_NUMERIC_HPP
