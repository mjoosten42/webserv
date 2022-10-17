#pragma once

#include "HTTP.hpp"
#include "Request.hpp"

#include <string>

class Server;

class Response: public HTTP {
	public:
		Response();
		Response(Request request, const Server *server);

		bool			   processNextChunk();
		const std::string& getChunk() const;
		bool			   isInitialized() const;
		bool			   isDone() const;

		void clear();

	private:
		void initDefaultHeaders();

		std::string getStatusLine() const;
		std::string getStatusMessage() const;
		std::string getResponseAsString();

		std::string getHeadersAsString() const;

		void handle();
		void handleGet();
		int	 handleGetWithStaticFile();
		void sendFail(int code, const std::string& msg);
		void sendMoved(const std::string& location);

		int	 getFirstChunk();
		void getNextChunk();
		int	 addSingleFileToBody();

	public:
		int m_statusCode;

		enum state { PROCESSING, WRITING, DONE } m_state;

	private:
		std::string	  m_chunk;
		Request		  m_request;
		const Server *m_server;
		int m_readfd; //  the fd of the file to read. The methods who return the chunks are responsible for closing the
					  //  file in time.
		bool m_hasStartedSending; //  if we have made the first chunk, this is true(i.e. we don't have to send the
								  //  headers again)
		bool m_isFinalChunk;	  //  true if every chunk has been read.
};
