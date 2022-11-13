#pragma once

#include <map>
#include <string>

class EnvironmentMap {
	public:
		EnvironmentMap();

		std::string& operator[](const std::string& key);

		char **toCharpp() const;

	private:
		void addEnv();

		std::map<std::string, std::string> m_map;
};

// std::ostream& operator<<(std::ostream& os, const EnvironmentMap& em);
