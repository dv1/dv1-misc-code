#ifndef BASE_KD_TREE_HPP_________
#define BASE_KD_TREE_HPP_________

#include <algorithm>
#include <vector>
#include <iterator>
#include <limits>
#include <type_traits>
#include <assert.h>


namespace base
{


template < typename Value >
struct kd_tree
{
	typedef Value value_type;

	struct node
	{
		Value m_value;
		unsigned int m_level;

		bool m_occupied;

		node()
			: m_occupied(false)
		{
		}
	};

	typedef std::vector < node > nodes;
	typedef typename nodes::const_iterator const_iterator;
	typedef typename nodes::iterator iterator;

	nodes m_nodes;
};


namespace detail
{


template < typename Value >
void alloc_node(kd_tree < Value > &p_kd_tree, std::size_t p_array_index)
{
	if (p_array_index >= p_kd_tree.m_nodes.size())
		p_kd_tree.m_nodes.resize(p_array_index + 1);
}


template < typename Value, typename Iterator, typename ComparePredicate, typename AssignFromInput >
void fill_node(kd_tree < Value > &p_kd_tree, std::size_t p_array_index, Iterator p_begin, Iterator p_end, unsigned int p_level, ComparePredicate &p_compare_predicate, AssignFromInput &p_assign_from_input)
{
	typename std::iterator_traits < Iterator > ::difference_type num_values = std::distance(p_begin, p_end);
	assert(num_values > 0);

	// NOTE: We assume here that the iterators themselves
	// are not invalidated by std::sort(). The standard
	// seems to guarantee this: https://stackoverflow.com/questions/3885482/does-a-vector-sort-invalidate-iterators/3885638#3885638
	std::sort(
		p_begin, p_end,
		[p_level, &p_compare_predicate](Value const &p_first, Value const &p_second) -> bool {
			return p_compare_predicate(p_first, p_second, p_level);
		}
	);

	alloc_node(p_kd_tree, p_array_index);
	auto &cur_node = p_kd_tree.m_nodes[p_array_index];

	auto median_value_iter = p_begin;
	std::advance(median_value_iter, num_values / 2);

	cur_node.m_value = p_assign_from_input(*median_value_iter);
	cur_node.m_level = p_level;
	cur_node.m_occupied = true;

	if (p_begin != median_value_iter)
	{
		std::size_t child0_array_index = 2 * p_array_index + 1;
		fill_node(p_kd_tree, child0_array_index, p_begin, median_value_iter, p_level + 1, p_compare_predicate, p_assign_from_input);
	}

	median_value_iter++;

	if (median_value_iter != p_end)
	{
		std::size_t child1_array_index = 2 * p_array_index + 2;
		fill_node(p_kd_tree, child1_array_index, median_value_iter, p_end, p_level + 1, p_compare_predicate, p_assign_from_input);
	}
}


template < typename Value, typename SearchValue, typename Distance, typename CalcDistanceFunc, typename CalcPlaneDistanceFunc >
void find_nearest_node(kd_tree < Value > const &p_kd_tree, SearchValue const &p_search_value, size_t p_array_index, std::size_t &nearest_node_index, Distance &p_cur_min_kd_distance, CalcDistanceFunc &p_calc_distance_func, CalcPlaneDistanceFunc &p_calc_plane_distance_func)
{
	auto const &cur_node = p_kd_tree.m_nodes[p_array_index];

	if (p_array_index != 0)
	{
		Distance kd_distance = p_calc_distance_func(cur_node.m_value, p_search_value);
		if (kd_distance < p_cur_min_kd_distance)
		{
			p_cur_min_kd_distance = kd_distance;
			nearest_node_index = p_array_index;
		}
	}

	Distance kd_plane_distance = p_calc_plane_distance_func(cur_node.m_value, p_search_value, cur_node.m_level);

	std::size_t child0_array_index = 2 * p_array_index + 1;
	std::size_t child1_array_index = 2 * p_array_index + 2;
	bool has_child0 = (child0_array_index < p_kd_tree.m_nodes.size()) && p_kd_tree.m_nodes[child0_array_index].m_occupied;
	bool has_child1 = (child1_array_index < p_kd_tree.m_nodes.size()) && p_kd_tree.m_nodes[child1_array_index].m_occupied;

	if (kd_plane_distance >= 0)
	{
		if (has_child1)
			find_nearest_node(p_kd_tree, p_search_value, child1_array_index, nearest_node_index, p_cur_min_kd_distance, p_calc_distance_func, p_calc_plane_distance_func);
		if (has_child0 && (kd_plane_distance < p_cur_min_kd_distance))
			find_nearest_node(p_kd_tree, p_search_value, child0_array_index, nearest_node_index, p_cur_min_kd_distance, p_calc_distance_func, p_calc_plane_distance_func);
	}
	else
	{
		if (has_child0)
			find_nearest_node(p_kd_tree, p_search_value, child0_array_index, nearest_node_index, p_cur_min_kd_distance, p_calc_distance_func, p_calc_plane_distance_func);
		if (has_child1 && ((-kd_plane_distance) < p_cur_min_kd_distance))
			find_nearest_node(p_kd_tree, p_search_value, child1_array_index, nearest_node_index, p_cur_min_kd_distance, p_calc_distance_func, p_calc_plane_distance_func);
	}
}


template < typename InputValue, typename OutputValue >
struct default_assign_from_input
{
	OutputValue operator()(InputValue p_input_value) const
	{
		return p_input_value;
	}
};


} // namespace detail end


template < typename Value >
void clear(kd_tree < Value > &p_kd_tree, bool p_free_space = false)
{
	if (p_free_space)
	{
		p_kd_tree.m_nodes.clear();
	}
	else
	{
		for (auto & node : p_kd_tree.m_nodes)
			node.m_occupied = false;
	}
}


template < typename Value, typename Iterator, typename ComparePredicate, typename AssignFromInput = detail::default_assign_from_input < typename std::iterator_traits < Iterator > ::value_type, Value > >
void fill(kd_tree < Value > &p_kd_tree, Iterator p_begin, Iterator p_end, ComparePredicate p_compare_predicate, AssignFromInput p_assign_from_input = detail::default_assign_from_input < typename std::iterator_traits < Iterator > ::value_type, Value > ())
{
	detail::fill_node(p_kd_tree, 0, p_begin, p_end, 0, p_compare_predicate, p_assign_from_input);
}


template < typename Value >
auto begin(kd_tree < Value > &p_kd_tree) -> typename kd_tree < Value > ::iterator
{
	return p_kd_tree.m_nodes.begin();
}


template < typename Value >
auto end(kd_tree < Value > &p_kd_tree) -> typename kd_tree < Value > ::iterator
{
	return p_kd_tree.m_nodes.end();
}


template < typename Value >
auto cbegin(kd_tree < Value > &p_kd_tree) -> typename kd_tree < Value > ::const_iterator
{
	return p_kd_tree.m_nodes.cbegin();
}


template < typename Value >
auto cend(kd_tree < Value > &p_kd_tree) -> typename kd_tree < Value > ::const_iterator
{
	return p_kd_tree.m_nodes.cend();
}


template < typename Value, typename SearchValue, typename CalcDistanceFunc, typename CalcPlaneDistanceFunc >
auto find_nearest(kd_tree < Value > const &p_kd_tree, SearchValue const &p_search_value, CalcDistanceFunc p_calc_distance_func, CalcPlaneDistanceFunc p_calc_plane_distance_func) -> typename kd_tree < Value > ::const_iterator
{
	if (p_kd_tree.m_nodes.empty())
		return p_kd_tree.m_nodes.end();

	std::size_t nearest_node_index = 0;
	auto cur_min_kd_distance = p_calc_distance_func(p_kd_tree.m_nodes[nearest_node_index].m_value, p_search_value);

	detail::find_nearest_node(p_kd_tree, p_search_value, 0, nearest_node_index, cur_min_kd_distance, p_calc_distance_func, p_calc_plane_distance_func);

	return p_kd_tree.m_nodes.cbegin() + nearest_node_index;
}


template < typename Value, typename SearchValue, typename CalcDistanceFunc, typename CalcPlaneDistanceFunc >
auto find_nearest(kd_tree < Value > &p_kd_tree, SearchValue const &p_search_value, CalcDistanceFunc p_calc_distance_func, CalcPlaneDistanceFunc p_calc_plane_distance_func) -> typename kd_tree < Value > ::iterator
{
	if (p_kd_tree.m_nodes.empty())
		return p_kd_tree.m_nodes.end();

	std::size_t nearest_node_index = 0;
	auto cur_min_kd_distance = p_calc_distance_func(p_kd_tree.m_nodes[nearest_node_index].m_value, p_search_value);

	detail::find_nearest_node(p_kd_tree, p_search_value, 0, nearest_node_index, cur_min_kd_distance, p_calc_distance_func, p_calc_plane_distance_func);

	return p_kd_tree.m_nodes.begin() + nearest_node_index;
}


} // namespace base end


#endif // BASE_KD_TREE_HPP_________
