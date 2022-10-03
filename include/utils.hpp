#pragma once

#include <iostream>

template <typename T>
void	print(const T& value) {
	std::cout << value << "\n";
}

void	fatal_perror(const char *msg);
