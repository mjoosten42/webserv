/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: lindsay <lindsay@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/18 13:30:35 by lindsay       #+#    #+#                 */
/*   Updated: 2022/10/06 15:19:40 by limartin      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response() {
	return;
}

Response::Response(const Response& copy) {
	*this = copy;
	return;
}

Response::~Response() {
	return;
}

Response& Response::operator=(const Response& assignment) {
	if (this != &assignment) {
		this->m_fd = assignment.m_fd;	   //  currently unused
		this->m_server = assignment.m_server; //  currently unused
		this->m_statusLine = assignment.m_statusLine;
		this->m_header = assignment.m_header;
		this->m_body = assignment.m_body;
	}
	return (*this);
}

std::string Response::getResponseAsString(void) const {
	std::stringstream ret;
	std::string		  endline = "\r\n";

	ret << m_statusLine << endline;
	for (const_str_vector_t::iterator it = m_header.begin(); it != m_header.end(); it++)
		ret << *it << endline;
	ret << endline;
	for (const_str_vector_t::iterator it = m_body.begin(); it != m_body.end(); it++)
		ret << *it << endline;
	ret << endline;

	return (ret.str());
}
