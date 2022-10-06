/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: lindsay <lindsay@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/18 13:30:42 by lindsay       #+#    #+#                 */
/*   Updated: 2022/10/06 15:05:24 by limartin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Server.hpp"

#include <sstream> //	getFormattedResponse
#include <string>
#include <vector>

typedef std::vector<std::string>	   str_vector_t;
typedef std::vector<const std::string> const_str_vector_t;

class Response {
	public:								//  Constructors & Destructors
		Response();						//  Default constructor
		Response(const Response& copy); //  Copy constructor
		~Response();					//  Destructor

	public:												 //  Operator overloads
		Response& operator=(const Response& assignment); //  Assignment operator

	public:					   //  Pubic member variables & methods
		int			 m_fd;	   //  currently unused
		Server		*m_server; //  currently unused
		std::string	 m_statusLine;
		str_vector_t m_header;
		str_vector_t m_body;
		std::string	 getFormattedResponse(void) const;

	protected: //  Protected member variables & methods

	private: //  Private member variables & methods

	public: //  Accessors

	private: //  Nested classes
};

#endif
