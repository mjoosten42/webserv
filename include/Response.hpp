#pragma once

#include "CGI.hpp"
#include "HTTP.hpp"
#include "Request.hpp"

#include <string>

class Server;

class Response: public HTTP {
	public:
		Response();

		void processRequest();
		void initialize();

		std::string& getNextChunk();
		void		 trimChunk(ssize_t bytes_sent);

		void appendBodyPiece();

		bool isDone() const;
		bool hasSourceFd() const;
		bool hasProcessedRequest() const;
		bool wantsClose() const;

		void addServer(const Server *server);

		const Server *getServer() const;
		Request		& getRequest();
		int			  getSourceFD() const;

	private:
		void setFlags();
		void addDefaultHeaders();

		void handleFile();
		void handleCGI();
		void handleDelete();

		void		openError(bool isDirectory);
		void		addFileHeaders();
		void		getCGIHeaderChunk();
		void		encodeChunked(std::string		&str);
		std::string getLine(std::string& str);

		std::string readBlockFromFile();

		void serveError(const std::string& str);
		void sendFail(int code, const std::string& msg);
		void sendMoved(const std::string& location);

		void createIndex(std::string path_to_index);

		void writeToCGI();

		std::string getStatusLine() const;
		std::string getStatusMessage() const;
		std::string getResponseAsString();

	public:
		int m_statusCode;

	private:
		const Server *m_server;
		Request		  m_request;
		CGI			  m_cgi;

		std::string m_chunk;
		std::string m_filename;
		std::string m_savedLine; // Used to check if CGI is behaving

		int	   m_source_fd;		// the fd of the file/pipe.
		size_t m_locationIndex; // number given to server to identify location

		bool m_processedRequest;
		bool m_isCGI; // true if it is a CGI request
		bool m_isChunked;
		bool m_doneReading;				  // true if all data from readfd has been read.
		bool m_CGI_DoneProcessingHeaders; // true if done parsing CGI headers
};
