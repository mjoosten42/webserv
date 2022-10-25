#pragma once

#include "CGI.hpp"
#include "EndOfHeaderFinder.hpp"
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

		bool shouldClose() const;

		void clear();

	private:
		static std::string wrapStringInChunkedEncoding(std::string& str);

		void setFlags();

		void addDefaultHeaders();

		std::string getStatusLine() const;
		std::string getStatusMessage() const;
		std::string getResponseAsString();

		void handleGet();
		void handlePost();
		int	 handleGetWithStaticFile(std::string file = ""); // TODO
		void sendFail(int code, const std::string& msg);
		void sendMoved(const std::string& location);

		int getFirstChunk();
		int addSingleFileToBody();

		std::string readBlockFromFile();
		void		getCGIHeaderChunk();

	public:
		int m_statusCode;

	private:
		EndOfHeaderFinder m_headerEndFinder;
		std::string		  m_chunk;
		Request			  m_request;
		CGI				  m_cgi;
		const Server	 *m_server;
		int m_readfd; //  the fd of the file/pipe. The methods who return the chunks are responsible for closing the
					  //  file in time.
		bool m_isFinalChunk;		   //  true if every chunk has been read.
		bool m_isCGI;				   // true if it is a CGI request, as filled in by checkWetherCGI()
		bool m_isCGIProcessingHeaders; // true if it is still parsing the headers back from the CGI and not yet in CGI
									   // chunked sending mode.
		bool m_close;				   // Connection: close, close the connection after response is sent
};
