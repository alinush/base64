/**
 *	File:		Base64.h
 *	Author:		Alin Tomescu, tomescu.alin@gmail.com
 *	Website:	http://alinush.org
 *	Date: 		December 22nd, 2011
 *	License:	Free to use and distribute as long as this notice is kept.
 */
#pragma once

#include "Core.h"

#include <stdexcept>

/**
 * The Base64 class provides static methods for encoding
 * and decode memory blocks or files using the base64 algorithm.
 *
 * @author		Alin Tomescu
 * @version		0.2
 * @date		12/22/2011
 */
class Base64
{
	public:
		/**
		 *	TODO: Add a calculateBufferSize method for encoding and decoding.
		 */
		static ulong getEncodedSize(ulong inputBufferSize) { return (inputBufferSize / 3) * 4 + (inputBufferSize % 3 > 0 ? 4 : 0); }
		static ulong getDecodedSize(ulong inputBufferSize) { return (inputBufferSize / 4) * 3; }
		
		/**
		 *	Returns true if the bytes in the specified buffer represent a valid base64-encoding.
		 *
		 *	@return true if the buffer stores a valid base64 encoding, false otherwise
		 */
		static bool isValidEncoding(const char * buffer, ulong length);
		
		/**
		 * TODO: Should this null-terminate the string? Maybe it should...
		 * Encodes the specified input buffer in base64 and stores it in the output buffer.
		 *
		 * @param	in			the input buffer to encode
		 * @param	out			the output buffer where the base64-encoded string will be stored
		 * @param	inSize		the length in bytes of the input buffer
		 *
		 * @return	the length in bytes of the encoded data in the output buffer
		 */
		static ulong encodeBuffer(const byte * in, char * out, ulong inSize);

		/**
		 * Decodes a base64-encoded string and stores the result in the output buffer.
		 *
		 * @param	in	the input base64-encoded string to decode
		 * @param	out	the output buffer where the decoded string will be stored
		 * @param	inSize	the length in bytes of the input buffer
		 *
		 * @return	the length in bytes of the decoded data in the output buffer
		 *
		 * @throws	std::runtime_error
		 *				if the input string is not a valid base64-encoded string
		 */
		static ulong decodeBuffer(const char * in, byte * out, ulong inSize) throw (std::runtime_error);
		
		/**
		 * Encodes a file in base64 and stores the result in a different file.
		 *
		 * @param	inFile		path to the input file to be encoded
		 * @param	outFile		path to the destination file where the encoded file will be stored
		 * @param	newline		the newline characters that will be used to separate the base64-encoded lines
		 * @param	lineSize	the size of a base64-encoded line in the file
		 *
		 * @throws	std::runtime_error	
		 *				if there's an I/O error, if the line size is not a multiple of 4, 
		 * 				or if the input file is empty
		 */
		static void encodeFile(const char * inFile, const char * outFile, const char * newline = "\r\n", uint lineSize = 76)
			throw (std::runtime_error);
		
		/**
		 * Decodes a base64-encoded file and stores it in another file.
		 *
		 * @param	inFile		path to the input base64-encoded file to be decoded
		 * @param	outFile		path to the destination file where the decoded file will be stored
		 *
		 * @throws	std::runtime_error	
		 *				if there's an I/O error, if the input file is empty, 
		 *				or if the file is not a valid base64-encoded file
		 */
		static void decodeFile(const char * inFile, const char * outFile) throw (std::runtime_error);
		
	private:
		/**
		 * Returns the offset in the base64 alphabet of the specified character.
		 * This method is used when decoding a 4-byte base64-encoded block.
		 *
		 * @param	ch	the character to look up in the alphabet
		 *
		 * @return	the offset of the specified character in the base64 alphabet
		 *
		 * @throws	std::runtime_error 
		 *				if the character is not in the alphabet
		 */
		static byte charToByte(char ch) throw (std::runtime_error);
		
		/**
		 * Returns the character mapped to the specified number in 
		 * the base64 alphabet.
		 *
		 * @param	number	the offset of the base64 character
		 *
		 * @return	the ASCII character corresponding to that number
		 */
		static char byteToChar(byte number) { return _byteToChar[number]; }
		
		/**
		 * Encodes a block of up to 24 bits (3 Buffer) to 4 characters in base64.
		 *
		 * @param	in			the input block of data (3 Buffer max.)
		 * @param	out			the output buffer where the encoded data will be stored
		 * @param	inLength	size in bytes of the input block, will usually be 3
		 */
		static void encodeBlock(const byte in[3], char out[4], uint inLength);
		
		/**
		 * Decodes a base64-encoded 4-character block to a 3-byte block of data and returns
		 * the length of the decoded block, which could be less than 3 Buffer when padding was
		 * present.
		 *
		 * @param	in	the input base64-encoded 4-character block
		 * @param	out the output buffer where the decoded data will be stored
		 *
		 * @return the length of the decoded block 
		 *
		 * @throws std::runtime_error
		 *				if the characters in the input string are not in the base64
		 *				alphabet
		 */
		static uint decodeBlock(const char in[4], byte out[3]) throw (std::runtime_error);
		
	private:
		
		/**
		 * The base64 alphabet associates a number to each symbol (letter, digit, etc.) in it.
		 */
		static const char _byteToChar[64];
		
		/**
		 * The padding character used for encoding blocks that are less than 3 Buffer long
		 */
		static const char _paddingChar;
};
