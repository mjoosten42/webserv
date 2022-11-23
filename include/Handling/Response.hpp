#pragma once

#include "CGI.hpp"
#include "HTTP.hpp"
#include "Request.hpp"
#include "Server.hpp"

#include <string>

class Response: public HTTP {
		friend class CGI;

	public:
		Response();

		void processRequest();
		void addServer(const Server *server);

		std::string &getNextChunk();
		void		 trimChunk(ssize_t bytes_sent);

		std::string readBlock();
		void		writeToCGI();

		bool hasProcessedRequest() const;
		bool isCGI() const;
		bool isDone() const;
		bool wantsClose() const;
		bool hadFD() const;

		Request &getRequest();
		FD		 getSourceFD() const;

	private:
		void initialize();
		void setFlags();
		void addDefaultHeaders();

		// File
		void handleFile();
		void openError();
		void addFileHeaders();
		void createIndex(const std::string &path_to_index);

		// CGI
		void handleCGI();
		void getCGIHeaderChunk();
		void parseCGIHeaders();
		void processCGIHeaders();
		void encodeChunked(std::string &str);

		// Abnormal statuses
		void sendCustomErrorPage();
		void sendFail(unsigned int code, const std::string &msg);
		void sendMoved(const std::string &redirect);

		std::string getStatusLine() const;
		std::string getStatusMessage() const;
		std::string getResponseAsString();

	public:
		std::string m_peer; // Client info, (always 127.0.0.1 in our case)

	private:
		const Server *m_server; // Config options
		Request		  m_request;
		CGI			  m_cgi;

		std::string m_chunk;	// Used to send data to client
		std::string m_filename; // The resource identified by the URI

		FD	m_source_fd;	 // the fd of the file/pipe.
		int m_locationIndex; // number given to server to identify location

		bool m_processedRequest; // One-time lock
		bool m_isCGI;			 // CGI or static file
		bool m_hadFD;			 // Used to remove pollers source if CGI exits early
		bool m_isChunked;		 // Send using chunked encoding
		bool m_doneReading;		 // No need to read from source
		bool m_headersDone;		 // Done parsing CGI headers
		bool m_close;			 // Wants to close
};
