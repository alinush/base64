/**
 *	File:		Base64.cpp
 *	Author:		Alin Tomescu, tomescu.alin@gmail.com
 *	Website:	http://alinush.org
 *	Date: 		December 22nd, 2011
 *	License:	Free to use and distribute as long as this notice is kept.
 */
#include "Base64.h"

#include <cctype>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <fstream>

/**
 * This table maps every 6-bit number (0 through 63) to an ASCII character.
 * This table stores the base64 alphabet.
 */
const char Base64::_byteToChar[64] = 
{
	/**
	 * Uppercase letters (from 0 to 25)
	 */
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 
	'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	
	/**
	 * Lowercase letters (from 26 to 51)
	 */
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 
	'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	
	/**
	 * Digits (from 52 to 61)
	 */
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	
	/**
	 * Misc. (from 62 to 63)
	 */
	'+', '/'
};

/**
 * The padding character is used to fill the remaining characters in the
 * base64 encoded block, when the input block is less than 3 bytes long.
 */
const char Base64::_paddingChar = '=';

bool Base64::isValidEncoding(const char * buffer, ulong length)
{
	//	Ensure string length is a multiple of 4
	if(length % 4)
		return false;
		
	//	Ensure string is only made up of base64 arguments
	for(uint i = 0; i < length; i++)
	{
		if(!isalnum(buffer[i]) && 
			buffer[i] != '+' && 
			buffer[i] != '/' && 
			buffer[i] != _paddingChar)
			return false;
	}
	
	if(length != 0)
	{
		//	Ensure string only has padding characters on the last 2 positions
		for(uint i = 0; i < length - 2; i++)
		{
			if(buffer[i] == _paddingChar)
				return false;
		}
		
		//	Ensure that the if the second to last character is a padding char, then the 
		//	last character is also a padding char
		if(buffer[length - 2] == _paddingChar && buffer[length - 1] != _paddingChar)
			return false;
	}
	
	return true;
}

void Base64::encodeBlock(const byte in[3], char out[4], uint inLength)
{
	/**
	 * Encoding three bytes works by splitting the 3 x 8 = 24 byte block
	 * into 4 blocks of 6 bytes. This is done using bitwise operations. Each one of the
	 * resulting 4 blocks will represent a number in the 0 to 63 range. 
	 * Each number is associated with a character in the base64 table.
	 * 
	 * Once we transform the 3 bytes into the 4 6-bit numbers, we'll replace the
	 * 4 6-bit numbers by their corresponding characters in the table and get
	 * the final base64 encoded block.
	 *
	 *	Of course, you also have to deal with padding, which involves a few checks.
	 */
	
	out[0] = byteToChar(in[0] >> 2);
	out[2] = out[3] = Base64::_paddingChar;
	
	out[1] = byteToChar(
		((in[0] & 0x03) << 4) |
		(inLength > 1 ? (((in[1] & 0xF0)) >> 4) : 0)
	);
	
	if(inLength >= 2)
	{
		out[2] = byteToChar(
			((in[1] & 0x0F) << 2) | 
			(inLength == 3 ? ((in[2] & 0xC0) >> 6) : 0)
		);
		
		if(inLength == 3)
			out[3] = byteToChar(in[2] & 0x3F);
	}
}

uint Base64::decodeBlock(const char in[4], byte out[3]) throw (std::runtime_error)
{
	/**
	 *	Decoding a 4-character blocks is just the reverse of encoding. You look at each character
	 *	and get its offset in the base64 alphabet. This will be a 6 bit number. You do this for all
	 *	4 characters and you'll get 6 x 4 = 24 bits = 3 bytes. These 3 bytes obtained by concatenating
	 *	all those 6 bits will be the decoded data.
	 *
	 *	Of course, you also have to deal with padding, which involves a few checks.
	 */
	 
	/**
	 * The length of the decoded data will be stored here
	 * and returned when the function exits.
	 */
	uint length = 0;
	
	out[0] = (charToByte(in[0]) << 2) | ((charToByte(in[1]) & 0x30) >> 4);
	out[1] = ((charToByte(in[1]) & 0x0F) << 4);
	length += 2;

	/**
	 * If the 3rd input char is not a padding char, then go ahead and
	 * decode it, storing it in the remaining part of the 2nd output byte.
	 */
	if(in[2] != Base64::_paddingChar)
	{
		out[1] |= ((charToByte(in[2]) & 0x3C) >> 2);
		
		/**
		 * If the 4th input char is also not a padding char, then go ahead and decode it,
		 * storing it in the 3rd output byte.
		 */
		if(in[3] != Base64::_paddingChar)
		{
			out[2] = ((charToByte(in[2]) & 0x03) << 6) | (charToByte(in[3]) & 0x3F);
			length += 1;
		}
	}
	/**
	 * Otherwise, if the 3rd input char is a padding char, 
	 * then make sure the 4th one is also a padding char.
	 */
	else if(in[3] != Base64::_paddingChar)
	{
		std::ostringstream error("Non-padding char encountered immediately after padding char: ");
		error << in[3] << "(ASCII code: " << static_cast<unsigned short>(in[3]) << ")";
		throw std::runtime_error(error.str());
	}
	
	return length;
}

byte Base64::charToByte(char ch) throw (std::runtime_error)
{
	if(ch == '+')
		return 62;
	else if(ch == '/')
		return 63;
	else if(isdigit(ch))
		return 52 + (ch - '0');
	else if(isalpha(ch))
	{
		if(isupper(ch))
			return ch - 'A';
		else
			return 26 + (ch - 'a');
	}
	else
	{
		std::ostringstream error("Invalid character detected in the base64-encoded input: ");
		error << ch << "(ASCII code: " << static_cast<unsigned short>(ch) << ")";
		throw std::runtime_error(error.str());
	}
	
}

ulong Base64::encodeBuffer(const byte * in, char * out, ulong inSize)
{
	/**
	 * Compute the number of 3 byte chunks and, if the
	 * last chunk is less than 3 bytes, compute its size also.
	 */
	uint nChunks = inSize / 3;
	uint lastChunkSize = inSize % 3;
	
	/**
	 * Get two pointers to the input and output buffers.
	 */
	const byte * inPtr = in;
	char * outPtr = out;
	
	/**
	 * For each chunk of 3 bytes, encode it in base 64,
	 * and advance the input and output pointers into the buffers.
	 */
	for(uint i = 0; i < nChunks; i++)
	{
		encodeBlock(inPtr, outPtr, 3);
		//std::cout << "In: " << inPtr[0] << inPtr[1] << inPtr[2] << std::endl;
		//std::cout << "Out: " << outPtr[0] << outPtr[1] << outPtr[2] << outPtr[3] << std::endl;
		inPtr += 3;
		outPtr += 4;
	}
	
	/**
	 * Deal with the last chunk also.
	 */
	if(lastChunkSize > 0)
	{
		encodeBlock(inPtr, outPtr, lastChunkSize);
		return (nChunks + 1) * 4;
	}
	else
		return nChunks * 4;
}

ulong Base64::decodeBuffer(const char * in, byte * out, ulong inSize) throw (std::runtime_error)
{
	/**
	 * The length of the input base64-encoded line needs to be a multiple of 4.
	 */
	if(inSize % 4)
	{
		std::ostringstream error;
		error << "The length of the base64-encoded line (" << inSize << ") is not a multiple of 4.";
		throw std::runtime_error(error.str());
	}
	
	if(!isValidEncoding(in, inSize))
	{
		throw std::runtime_error("The input string is not a valid base64 encoding");
	}
	
	/**
	 * The number of 4-byte base64-encoded chunks.
	 */
	uint nChunks = inSize / 4;
	/**
	 * The length in bytes of the resulting decoded data.
	 */
	ulong decodedLength = 0;
	
	/**
	 * Get pointers to the input and output buffer.
	 */
	const char * inPtr = in;
	byte * outPtr = out;
	
	/**
	 * Decode each chunk, advance the pointers and keep track of
	 * the length of the decoded data.
	 */
	for(uint i = 0; i < nChunks; i++)
	{
		decodedLength += decodeBlock(inPtr, outPtr);
		inPtr += 4;
		outPtr += 3;
	}
	
	return decodedLength;
}

void Base64::encodeFile(const char * inFile, const char * outFile, const char * newline, uint lineSize) throw (std::runtime_error)
{
	/**
	 * The line size in the out file must be a multiple of 4 (It's simply how base64 works)
	 */
	if(lineSize % 4)
	{
		std::ostringstream error("The output file line size must be a multiple of 4. You provided ");
		error << lineSize << ".";
		throw std::runtime_error(error.str());
	}
	
	/**
	 * Open the file to be encoded and check for errors.
	 */
	std::ifstream fin(inFile, std::ios::binary);
	if(!fin)
	{
		std::ostringstream error("Cannot open input file for reading: ");
		error << inFile;
		throw std::runtime_error(error.str());
	}
	
	/**
	 * Get the file's size and make sure it's not an empty file.
	 */
	fin.seekg(0, std::ios::end);
	ulong fileLength = fin.tellg();
	fin.seekg(0, std::ios::beg);
	
	if(!fileLength)
	{
		fin.close();
		
		std::ostringstream error("Cannot base64 encode an empty file: ");
		error << inFile;
		throw std::runtime_error(error.str());
	}
	
	/**
	 * Open the destination file, where the base64 encoding will be stored.
	 * Do some error checking.
	 */
	std::ofstream fout(outFile, std::ios::binary);
	if(!fout)
	{
		fin.close();
		std::ostringstream error("Cannot open output file for writing: ");
		error << outFile;
		throw std::runtime_error(error.str());
	}
	
	/**
	 * Split the input file into chunks. Each chunk is going to be encoded
	 * into a line. The last line might not have the same size as the others
	 * so it needs to be handled separately.
	 */
	uint outBufferSize = lineSize, inBufferSize = lineSize * 3 / 4;
	uint nChunks = fileLength / inBufferSize;
	uint lastChunkSize = fileLength % inBufferSize;
	
	/**
	 * Allocate temporary buffers for reading a chunk into and for
	 * storing the encoded chunk into.
	 */
	byte * inBuffer = new byte[inBufferSize];
	char * outBuffer = new char[outBufferSize + 1];
	outBuffer[outBufferSize] = '\0';
	
	try 
	{
		/**
		 * Encode the input file by reading a chunk, encoding it
		 * in base64 and storing it as a line in the output file.
		 */
		for(uint i = 0; i < nChunks; i++)
		{	
			fin.read(reinterpret_cast<char *>(inBuffer), inBufferSize);
			
			encodeBuffer(inBuffer, outBuffer, inBufferSize);
			
			fout << outBuffer << newline;
		}
		
		/**
		 * Handle the last chunk, if existent.
		 */
		if(lastChunkSize)
		{
			memset(inBuffer, 0, inBufferSize);
			memset(outBuffer, 0, outBufferSize);
			
			fin.read(reinterpret_cast<char *>(inBuffer), lastChunkSize);		
			
			encodeBuffer(inBuffer, outBuffer, lastChunkSize);
			
			fout << outBuffer << newline;
		}
	}
	catch(...)
	{
		delete [] inBuffer;
		delete [] outBuffer;
		
		fin.close();
		fout.close();
		
		throw;
	}
	
	delete [] inBuffer;
	delete [] outBuffer;
	
	fin.close();
	fout.close();
}

void Base64::decodeFile(const char * inFile, const char * outFile) throw (std::runtime_error)
{
	/**
	 * Open the input file to be decoded and check for errors.
	 */
	std::ifstream fin(inFile, std::ios::binary);
	if(!fin)
	{
		std::ostringstream error("Cannot open input file for reading: ");
		error << inFile;
		throw std::runtime_error(error.str());
	}

	/**
	 * Get the file's size and make sure it's not an empty file.
	 */
	fin.seekg(0, std::ios::end);
	ulong fileLength = fin.tellg();
	fin.seekg(0, std::ios::beg);
	
	if(!fileLength)
	{
		fin.close();
		
		std::ostringstream error("Cannot base64 decode an empty file: ");
		error << inFile;
		throw std::runtime_error(error.str());
	}
	
	/**
	 * Open the ouput file file to store the decoded file in.
	 * Check for errors.
	 */
	std::ofstream fout(outFile, std::ios::binary);
	if(!fout)
	{
		fin.close();
		
		std::ostringstream error("Cannot open output file for writing: ");
		error << outFile;
		throw std::runtime_error(error.str());
	}
	
	/**
	 * Start decoding the file, line by line.
	 */
	uint lineSize = 76;
	ulong lineCount = 0;
	uint outBufferSize = getDecodedSize(lineSize);
	byte * outBuffer = new byte[outBufferSize];
	std::string inBuffer;
	
	do {
		try
		{
			/**
			 * Read in a line.
			 */
			getline(fin, inBuffer);
			lineCount++;
			
			/**
			 * Check if the line is \r\n terminated, and strip the \r away if it is.
			 * TODO: Actually trim the line.
			 */
			if(inBuffer.length() > 0 && inBuffer[inBuffer.length()-1] == '\r')
			{
				inBuffer.erase(inBuffer.length()-1);
			}
			
			/**
			 * Get the line's size and skip empty lines.
			 * TODO: Decide how to properly handle empty lines.
			 */
			lineSize = inBuffer.length();
			if(lineSize == 0)
				continue;
			
			/**
			 * The line size must be a multiple of 4 (otherwise the file is not a correctly encoded one).
			 */
			if(lineSize % 4)
			{
				fin.close();
				fout.close();
				delete [] outBuffer;
				
				std::ostringstream error;
				error << "Line #" << lineCount << " needs to have the size divisible by 4 in output file \""
					<< outFile << "\"";
				throw std::runtime_error(error.str());
			}
			
			/**
			 * If the next line happens to be bigger than the previous line then we need to
			 * reallocate a bigger output buffer. This is just an error tolerance measure.
			 */
			if(lineSize * 3 / 4 > outBufferSize)
			{
				delete [] outBuffer;
				outBuffer = new byte[lineSize * 3 / 4];
			}
			
			/**
			 * Calculate the new buffer size, which might have changed if the current
			 * line size is different than the previous one.
			 */
			outBufferSize = lineSize * 3 / 4;
			
			/**
			 * Check for padding characters at the end of the line.
			 */
			uint numEqualSigns = 0;
			if(inBuffer[lineSize-1] == _paddingChar) numEqualSigns++;
			if(inBuffer[lineSize-2] == _paddingChar) numEqualSigns++;
			
			/**
			 * Decode the line and write the resulting block of data out.
			 */
			ulong decodedLength = Base64::decodeBuffer(inBuffer.c_str(), outBuffer, lineSize);
			
			fout.write(reinterpret_cast<char *>(outBuffer), decodedLength);
		}
		catch(...) 
		{
			fin.close();
			fout.close();
			delete [] outBuffer;
			throw;
		}
	} while(!fin.eof());
	
	/**
	 * Cleanup.
	 */
	delete [] outBuffer;
	
	fin.close();
	fout.close();
}
