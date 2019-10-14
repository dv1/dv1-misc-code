#ifndef SCOPE_GUARD_HPP
#define SCOPE_GUARD_HPP

#include <functional>


namespace base
{


namespace detail
{


class scope_guard_impl
{
public:
	template < typename Func >
	explicit scope_guard_impl(Func &&p_func)
		: m_func(std::forward < Func > (p_func))
		, m_dismissed(false)
	{
	}

	~scope_guard_impl()
	{
		if (!m_dismissed)
		{
			// Make sure exceptions never exit the destructor, otherwise
			// undefined behavior occurs. For details about this, see
			// https://isocpp.org/wiki/faq/exceptions#dtors-shouldnt-throw

			try
			{
				m_func();
			}
			catch (...)
			{
			}
		}
	}

	scope_guard_impl(scope_guard_impl &&p_other)
		: m_func(std::move(p_other.m_func))
		, m_dismissed(p_other.m_dismissed)
	{
		p_other.m_dismissed = true;
	}

	void dismiss() const throw()
	{
		m_dismissed = true;
	}


private:
	scope_guard_impl(scope_guard_impl const &) = delete;
	scope_guard_impl& operator = (scope_guard_impl const &) = delete;

	std::function < void() > m_func;
	mutable bool m_dismissed;
};


} // namespace detail end


typedef detail::scope_guard_impl scope_guard_type;


template < typename Func >
detail::scope_guard_impl make_scope_guard(Func &&p_func)
{
	return detail::scope_guard_impl(std::forward < Func > (p_func));
}


} // namespace base end


#endif
