#pragma once

#include <map>
#include <string>

class EnvironmentMap {
	public:
		void addEnv();
	
		std::string& operator[](const std::string& key);

		char **toCharpp() const;

	private:
		std::map<std::string, std::string> m_map;
};

// std::ostream& operator<<(std::ostream& os, const EnvironmentMap& em);
