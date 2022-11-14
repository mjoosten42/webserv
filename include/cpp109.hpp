// It's CPP98 + 11!
#pragma once

#include <string>
#include "logger.hpp"

template <typename C>
typename C::value_type& my_back(C& cont) {
	typename C::iterator it = cont.end();
	return (*(--it));
}

template <typename C>
void my_pop_back(C& cont) {
	cont.resize(cont.size() - 1);
}
