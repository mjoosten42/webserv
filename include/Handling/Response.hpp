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

		const std::string &getNextChunk();
		void			   trimChunk(ssize_t bytes_sent);
		void			   setDoneReading();

		short readFromFile();
		short readFromCGI();
		short writeToCGI();

		bool hasProcessedRequest() const;
		bool isCGI() const;
		bool isDone() const;
		bool wantsClose() const;

		Request &getRequest();

		int getWriteFD() const;
		int getReadFD() const;

	private:
		void initialize();
		void setFlags();
		void addDefaultHeaders();

		// File
		void handleFile();
		void openError();
		void addFileHeaders();
		void createIndex(const std::string &path_to_index);
		void addToChunk(ssize_t size);

		// CGI
		void handleCGI();
		void getCGIHeaderChunk();
		void parseCGIHeaders();
		void processCGIHeaders();
		void encodeChunked(std::string &str);

		// Abnormal statuses
		void sendCustomErrorPage();
		void sendFail(int code, const std::string &msg);
		void sendMoved(const std::string &redirect);

		bool isStatus(const std::string &) const;

		std::string getStatusLine() const;
		std::string getStatusMessage() const;
		std::string getResponseAsString();

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
		bool m_isChunked;		 // Send using chunked encoding
		bool m_doneReading;		 // No need to read from source
		bool m_headersDone;		 // Done parsing CGI headers
		bool m_close;			 // Wants to close
};
