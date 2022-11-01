#pragma once

#include <map>

class EnvironmentMap {
	public:
		EnvironmentMap();

		std::string& operator[](const std::string& key);

		char **toCharpp() const;

	private:
		std::map<std::string, std::string> m_map;
};

// std::ostream& operator<<(std::ostream& os, const EnvironmentMap& em);
