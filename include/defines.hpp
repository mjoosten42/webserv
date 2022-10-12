#pragma once

//  HTTP RFC newline
#define CRLF "\r\n"

//  Terminal colors
#define RED "\033[0;31m"
#define DEFAULT "\033[0m"

#define FILE_BUF_SIZE (4096 - 1024)

//  WARNING: CHUNK_MAX_LENGTH CANNOT EXCEED 0xFFF as the length limit is hard coded.
//  However, there is no reason for such a high limit anyways, since browsers do not always support this.
#define CHUNK_MAX_LENGTH 0xFFF

//  3 bytes for hexadecimal size followed by CRLF
//  Then the chunk ended by another CRLF
#define CHUNK_SENDING_SIZE (3 + 2 + CHUNK_MAX_LENGTH + 2)

#define BUFSIZE 2048
