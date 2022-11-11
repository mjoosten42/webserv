#include "EnvironmentMap.hpp"

#include "defines.hpp"
#include "logger.hpp"

#include <string.h> // strdup(sighs...)
#include <string>

EnvironmentMap::EnvironmentMap() {
	addEnv();

	m_map["GATEWAY_INTERFACE"] = CGI_VERSION;
	m_map["SERVER_PROTOCOL"]   = HTTP_VERSION;
	m_map["SERVER_SOFTWARE"]   = SERVER_SOFTWARE;
}

std::string& EnvironmentMap::operator[](const std::string& key) {
	return m_map[key];
}

// returns a freeable null-terminated array of c strings to be used with execve().
char **EnvironmentMap::toCharpp() const {
	char **ret;

	try {
		ret = new char *[m_map.size() + 1];
	} catch (std::exception& e) {
		LOG_ERR(e.what());
		return NULL;
	}

	size_t i = 0;

	for (MapIter it = m_map.begin(); it != m_map.end(); it++, i++) {
		std::string entry = it->first + "=" + it->second;
		ret[i]			  = strdup(entry.c_str());
	}
	ret[i] = NULL;
	return ret;
}

void EnvironmentMap::addEnv() {
	extern char **environ;
	std::string	  entry;
	size_t		  eqpos;

	for (char **envp = environ; *envp != NULL; envp++) {
		entry = *envp;
		eqpos = entry.find('=');
		if (eqpos == std::string::npos) {
			LOG_ERR("WTF? Environ entry doesn't contain a '='.");
			continue;
		}
		std::string key	  = entry.substr(0, eqpos);
		std::string value = entry.substr(eqpos + 1);

		m_map[key] = value;
	}
}
