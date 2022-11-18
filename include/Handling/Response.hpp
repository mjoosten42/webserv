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
		void initialize();

		std::string& getNextChunk();
		void		 trimChunk(ssize_t bytes_sent);

		void		writeToCGI();
		std::string readBlock();

		bool hasProcessedRequest() const;
		bool isCGI() const;
		bool isDone() const;
		bool wantsClose() const;
		bool hadFD() const;

		const Server *getServer() const;
		Request		& getRequest();
		int			  getSourceFD() const;

	private:
		void setFlags();
		void addDefaultHeaders();

		void handleFile();
		void handleCGI();
		void handleDelete();

		void openError(const std::string& dir, bool isDirectory);

		void addFileHeaders();
		void getCGIHeaderChunk();
		void encodeChunked(std::string& str);

		void sendCustomErrorPage();
		void sendFail(int code, const std::string& msg);
		void sendMoved(const std::string& redirect);

		void createIndex(std::string path_to_index);

		std::string getStatusLine() const;
		std::string getStatusMessage() const;
		std::string getResponseAsString();

	public:
		std::string m_peer; // 127.0.0.1

	private:
		const Server *m_server;
		Request		  m_request;
		CGI			  m_cgi;

		std::string m_chunk;
		std::string m_filename;

		FD	m_source_fd;	 // the fd of the file/pipe.
		int m_locationIndex; // number given to server to identify location

		bool m_processedRequest;
		bool m_isCGI; // true if it is a CGI request
		bool m_hadFD; // Necessary to remove pollers source_fd if CGI misbehaves
		bool m_isChunked;
		bool m_doneReading;				  // true if all data from source fd has been read.
		bool m_CGI_DoneProcessingHeaders; // true if done parsing CGI headers
		bool m_close;					  // Wants to close
};
