#include "MIME.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "defines.hpp"
#include "stringutils.hpp"
#include "utils.hpp"

#include <fcntl.h> // open
#include <sys/socket.h>
#include <unistd.h> // lseek

#define FILE_BUF_SIZE (4096 - 1024) //  the max size of a file we want to load into a buffer in one time.
#define CHUNK_MAX_LENGTH (1024)		//  the desired max length of a HTTP/1.1 Chunked response chunk.

void Response::processRequest() {
	checkWhetherCGI();
	switch (m_request.getMethod()) {
		case GET:
			handleGet();
			break;
		case POST:
			handlePost();
			break;
		case DELETE:
			break;
		default:
			std::cerr << "Invalid method\n";
	}
}

void Response::checkWhetherCGI() {
	// TODO
	m_isCGI = strEndsWith(m_request.getLocation(), ".php");
	// std::cout << "doing CGI: " << m_isCGI << "\n";
}

void Response::sendFail(int code, const std::string& msg) {
	m_statusCode = code;
	if (m_server->getErrorPages().find(code) != m_server->getErrorPages().end())
	{
		std::string file = m_server->getErrorPages().at(code);
		handleGetWithStaticFile(file);
		return;
	}
	initDefaultHeaders();
	addHeader("Content-Type", "text/html");

	addToBody("<h1>" + toString(code) + " " + getStatusMessage() + "</h1>\r\n");
	addToBody("<p>something went wrong somewhere: <b>" + msg + "</b></p>\r\n");

	m_chunk		   = getResponseAsString();
	m_isFinalChunk = true;
}

void Response::sendMoved(const std::string& location) {
	clear(); //  <!-- TODO, also add default server
	initDefaultHeaders();
	m_statusCode			  = 301;
	m_headers["Location"]	  = location;
	m_headers["Connection"]	  = "Close";
	m_headers["Content-Type"] = "text/html";

	addToBody("<h1>" + toString(m_statusCode) + " " + getStatusMessage() + "</h1>\r\n");
	addToBody("<p>The resource has been moved to <a href=\"" + location + "\">" + location + "</a>.</p>");

	m_chunk		   = getResponseAsString();
	m_isFinalChunk = true;
}

void Response::handleGet() {

	initDefaultHeaders();
	if (m_isCGI) {
		// TODO: parse from config
		m_statusCode = m_cgi.start("/usr/bin/perl", "printenv.pl");

		m_headers["Transfer-Encoding"] = "Chunked";
		m_readfd					   = m_cgi.popen.readfd;

		getFirstCGIChunk();
	} else {
		m_statusCode = handleGetWithStaticFile();
	}

	if (m_statusCode != 200)
		sendFail(m_statusCode, m_isCGI ? "CGI BROKE 😂😂😂" : "Page is venting");
}

void Response::handlePost() {
	std::string filename = m_server->getRoot() + m_request.getLocation();
	ssize_t		bytes_written;

	m_readfd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (m_readfd < 0)
		perror("open");
	bytes_written = write(m_readfd, m_request.getBody().c_str(), m_request.getBody().length());
	close(m_readfd);

	m_request.cut(bytes_written);
	m_statusCode = 201;
	addHeader("Location", filename);
	m_chunk		   = getResponseAsString();
	m_isFinalChunk = true;
}

// TODO: Fix this. I added an ugly hacky param in case the file served is supposed ot be an error page.
int Response::handleGetWithStaticFile(std::string file) {
	std::string filename = m_server->getRoot() + m_request.getLocation();
	if (!file.empty())
		filename = file;
		// filename = m_server->getRoot() + "/" + file;
	print("Handle static: " + filename);

	m_readfd = open(filename.c_str(), O_RDONLY);
	if (m_readfd == -1) {
		if (errno == EACCES)
			return 403;
		perror("open");
		return 404;
	}

	addHeader("Connection", "Keep-Alive");
	addHeader("Content-Type", MIME::fromFileName(filename));

	return getFirstChunk();
}

//  this function removes bytesSent amount of bytes from the chunk. Used for instance when send() sent
//  less bytes than the chunk's length.
void Response::trimChunk(ssize_t bytesSent) {
	m_chunk = m_chunk.substr(bytesSent, m_chunk.length());
}

bool Response::isDone() const {
	return m_isFinalChunk;
}

int Response::getFirstChunk() {
	off_t size = lseek(m_readfd, 0, SEEK_END);
	int	  ret;

	//  get file size
	//  TODO: don't do this for CGI pipes!
	if (size == -1)
		size = std::numeric_limits<off_t>().max();
	lseek(m_readfd, 0, SEEK_SET); //  set back to start

	if (size > FILE_BUF_SIZE) { //  send multichunked
		addHeader("Transfer-Encoding", "Chunked");

		m_chunk = getResponseAsString();
		ret		= 200;

	} else { //  send in single buf.
		ret			   = addSingleFileToBody();
		m_chunk		   = getResponseAsString();
		m_isFinalChunk = true;
	}
	return ret;
}

int Response::addSingleFileToBody() {
	static char buf[FILE_BUF_SIZE + 1];
	ssize_t		bytes_read;

	bytes_read = read(m_readfd, buf, FILE_BUF_SIZE);
	if (bytes_read == -1) {
		std::cerr << "Reading infile fd " << m_readfd << " failed!\n";
		return 500;
	}
	buf[bytes_read] = 0;
	addToBody(buf, bytes_read);

	close(m_readfd);

	return 200;
}

void Response::getFirstCGIChunk() {

	// TODO: fix this. It'll hang on read. Besides, we shouldn't read at this point.
	std::string block = readBlockFromFile();

	// TODO: fix newlines, might be CRLF
	size_t loc = block.find_first_of("\n\n"); // end of the headers
	if (loc == std::string::npos) {
		sendFail(501, "Could not find headers in CGI response");
		return;
	}

	std::string headers = block.substr(0, loc + 2);
	block				= block.substr(loc + 2);
	m_chunk				= headers + wrapStringInChunkedEncoding(block);
}

std::string& Response::getNextChunk() {

	if (m_isFinalChunk)
		return m_chunk;

	std::string block = readBlockFromFile();
	m_chunk += wrapStringInChunkedEncoding(block);
	return m_chunk;
}

std::string& Response::wrapStringInChunkedEncoding(std::string& str) {
	// TODO: it might be slow to prepend the chunk with the size and CRLF. The old implementation is faster, but this
	// one is more modular.
	str = toHex(str.length()) + CRLF + str + CRLF;
	return str;
}

// this reads CHUNK_MAX_LENGTH - m_chunk.size() from a file and returns it.
// It has to be modified before put into a chunked response.
std::string Response::readBlockFromFile() {
	static char buf[CHUNK_MAX_LENGTH];
	std::string ret;
	ssize_t		size;

	size = read(m_readfd, buf, CHUNK_MAX_LENGTH - m_chunk.size());

	if (size <= 0) {
		//  TODO: error message and response and such.
		if (size == -1) {
			perror("read");
			std::cerr << "Reading infile failed!\n";
		}

		// size == 0, we reached EOF
		m_isFinalChunk = true;
		close(m_readfd);
	} else {
		ret.append(buf, size);
	}
	return ret;
}
