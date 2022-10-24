#pragma once

#include "CGI.hpp"
#include "HTTP.hpp"
#include "Request.hpp"

#include <string>

class Server;

class Response: public HTTP {
	public:
		Response();

		void		 processRequest();
		void		 trimChunk(ssize_t bytesSent);
		std::string& getNextChunk();

		bool isDone() const;
		bool isInitialized() const;
		bool finishedProcessing() const;

		Request	   & getRequest();
		const Server *getServer() const;
		void		  addServer(const Server *server);

		void clear();

	private:
		static std::string wrapStringInChunkedEncoding(std::string& str);

		void checkWhetherCGI();

		void initDefaultHeaders();

		std::string getStatusLine() const;
		std::string getStatusMessage() const;
		std::string getHeadersAsString() const;
		std::string getResponseHeadersAsString() const;
		std::string getResponseAsString();

		void handleGet();
		void handlePost();
		int	 handleGetWithStaticFile(std::string file = "");
		void sendFail(int code, const std::string& msg);
		void sendMoved(const std::string& location);

		int getFirstChunk();
		int addSingleFileToBody();

		std::string	 readBlockFromFile();
		void		 getCGIHeaderChunk();

	public:
		int m_statusCode;

		enum state { PROCESSING, WRITING, DONE } m_state;

	private:
		std::string	  m_chunk;
		Request		  m_request;
		CGI			  m_cgi;
		const Server *m_server;
		int m_readfd; //  the fd of the file to read. The methods who return the chunks are responsible for closing the
					  //  file in time.
		bool m_isFinalChunk;		   //  true if every chunk has been read.
		bool m_isCGI;				   // true if it is a CGI request, as filled in by checkWetherCGI()
		bool m_isCGIProcessingHeaders; // true if it is still parsing the headers back from the CGI and not yet in CGI
									   // chunked sending mode.
};
