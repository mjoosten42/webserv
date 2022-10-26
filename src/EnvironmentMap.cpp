#include "EnvironmentMap.hpp"

#include "logger.hpp"

#include <iostream>
#include <string.h> // strdup(sighs...)
#include <string>

EnvironmentMap::EnvironmentMap() {
	m_map = std::map<std::string, std::string>();
}

EnvironmentMap::EnvironmentMap(const EnvironmentMap& other) {
	*this = other;
}

EnvironmentMap::~EnvironmentMap() {}

EnvironmentMap& EnvironmentMap::operator=(const EnvironmentMap& rhs) {
	m_map = rhs.m_map; //  is this a shallow copy?
	return *this;
}

//
void EnvironmentMap::initFromEnviron() {
	extern char **environ;

	for (char **envp = environ; *envp != NULL; envp++) {
		std::string entry(*envp);
		size_t		eqpos = entry.find('=');
		if (eqpos == std::string::npos) {
			LOG_ERR("WTF? Environ entry doesn't contain a '='.");
			continue;
		}
		std::string key	  = entry.substr(0, eqpos);
		std::string value = entry.substr(eqpos + 1, std::string::npos);

		m_map[key] = value;
	}
}

std::string& EnvironmentMap::operator[](const std::string& key) {
	return m_map[key];
}

//  returns a freeable null-terminated array of c strings to be used with execve().
char **EnvironmentMap::toCharpp() const {
	std::map<std::string, std::string>::const_iterator it;
	char											 **ret = new char *[m_map.size() + 1];

	size_t i = 0;

	for (it = m_map.begin(); it != m_map.end(); it++, i++) {
		std::string entry = it->first + "=" + it->second;
		ret[i]			  = strdup(entry.c_str());
	}
	ret[i] = NULL;
	return ret;
}
