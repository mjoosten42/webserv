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

		Request		& getRequest();
		const Server *getServer() const;
		void		  addServer(const Server *server);

		void clear();

	private:
		void checkWetherCGI();

		void initDefaultHeaders();

		std::string getStatusLine() const;
		std::string getStatusMessage() const;
		std::string getHeadersAsString() const;
		std::string getResponseAsString();

		void handleGet();
		void handlePost();
		int	 handleGetWithStaticFile();
		void sendFail(int code, const std::string& msg);
		void sendMoved(const std::string& location);

		int	 getFirstChunk();
		int	 addSingleFileToBody();
		void wrapChunkInChunkedEncoding();

		void getFirstCGIChunk();

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
		bool m_isFinalChunk; //  true if every chunk has been read.
		bool m_isCGI;
};
