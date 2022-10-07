/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: lindsay <lindsay@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/08/18 13:30:35 by lindsay       #+#    #+#                 */
/*   Updated: 2022/10/06 15:42:34 by limartin      ########   odam.nl         */
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
		m_fd		 = assignment.m_fd;
		m_server	 = assignment.m_server; //  currently unused
		m_statusLine = assignment.m_statusLine;
		m_header	 = assignment.m_header;
		m_body		 = assignment.m_body;
	}
	return (*this);
}

bool Response::sendResponse(void) const {
	std::string response = getResponseAsCPPString();
	if (send(m_fd, response.c_str(), response.length(), 0) == -1)
		fatal_perror("send");
	return (true);
}

std::string Response::getResponseAsCPPString(void) const {
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
