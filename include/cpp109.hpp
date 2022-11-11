// It's CPP98 + 11!

#include <string>

template <typename C>
typename C::value_type& my_back(C &cont)
{
	typename C::iterator it = cont.end();
	return (*(it--));
}
