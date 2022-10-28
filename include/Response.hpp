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

		void clear();

		void		 processRequest();
		void		 trimChunk(ssize_t bytesSent);
		std::string& getNextChunk();

		bool isDone() const;
		bool isInitialized() const;
		bool readfdNeedsPoll() const;
		bool finishedProcessing() const;
		bool hasProcessedRequest() const;

		void addServer(const Server *server);

		bool shouldClose() const;

		Request		& getRequest();
		const Server *getServer() const;
		int			  getReadFD() const;

	private:
		void setFlags();
		void addDefaultHeaders();

		void handleGet();
		void handlePost();
		void handleDelete();

		void handleGetWithFile(std::string file = ""); // TODO
		void handleGetCGI();

		void getFirstChunk();
		void getCGIHeaderChunk();
		void encodeChunked(std::string& str);

		std::string readBlockFromFile();

		void serveError(const std::string& str);
		void sendFail(int code, const std::string& msg);
		void sendMoved(const std::string& location);

		int autoIndex(std::string path_to_index);

		std::string getStatusLine() const;
		std::string getStatusMessage() const;
		std::string getResponseAsString();

	public:
		int m_statusCode;

	private:
		EndOfHeaderFinder m_headerEndFinder;
		std::string		  m_chunk;
		Request			  m_request;
		CGI				  m_cgi;
		const Server	 *m_server;
		int				  m_readfd;		 // the fd of the file/pipe.
		bool			  m_doneReading; // true if all data from readfd has been read.
		bool			  m_isSmallFile;
		bool			  m_isCGI;					   // true if it is a CGI request, as filled in by checkWetherCGI()
		bool			  m_CGI_DoneProcessingHeaders; // true if done parsing the headers back from the CGI and in CGI
													   // chunked sending mode.
		bool m_close;								   // Connection: close, close the connection after response is sent
		bool m_hasReadFDPoller; // true if this response needs to poll the readfd before reading anything.
		bool m_processedRequest;
};
