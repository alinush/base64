/**
 *	File:		Core.h
 *	Author:		Alin Tomescu, tomescu.alin@gmail.com
 *	Website:	http://alinush.org
 *	Date: 		December 17th, 2011
 *	License:	Free to use and distribute as long as this notice is kept.
 */
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <map>
#include <vector>
#include <sstream>
#include <string>
using namespace std;

#include "Base64.h"

std::string base64_file_encode(const std::string& filePath);

std::string base64_encode(const std::string& input)
{
	ulong size = Base64::getEncodedSize(input.length()) + 1;
	
	char * buffer = new char[size];
	buffer[size - 1] = '\0';
	
	Base64::encodeBuffer(reinterpret_cast<const byte *>(input.c_str()), buffer, input.length());
	
	std::string result(buffer);
	delete [] buffer;
	
	return result;
}

std::string base64_decode(const std::string& input)
{
	//	Decode the valid encoding string using our Base64 library
	ulong size = Base64::getDecodedSize(input.length()) + 1;
	
	char * buffer = new char[size];
	
	ulong length = Base64::decodeBuffer(reinterpret_cast<const char *>(input.c_str()), reinterpret_cast<byte *>(buffer), input.length());
	buffer[length] = '\0';
	
	std::string result(buffer);
	delete [] buffer;
	
	return result;
}

void bvt_valid_encoding()
{
	const char * valid[] = {
		"", "abcd", "1234", "ab==", "abc=",
		"++++", "////", "a/b+", "+/a+",
		"YmFzaWMgdmlhYmlsaXR5IHRlc3Q=",
	};
	
	const char * invalid[] = {
		"a", "ab", "abc", "a===", "ab=c",
		"abcde", "====", "+--4", "4++)",
		"a=bc", "=", "==", "===", 
	};
	
	for(uint i = 0; i < sizeof(valid)/sizeof(valid[0]); i++)
	{
		if(!Base64::isValidEncoding(valid[i], strlen(valid[i])))
		{
			std::ostringstream error;
			error << "Base64 valid encodings BVT failed: " << "Valid encoding \"" << valid[i] << "\" was deemed invalid.";
			throw std::runtime_error(error.str()); 
		}
	}
	
	for(uint i = 0; i < sizeof(invalid)/sizeof(invalid[0]); i++)
	{
		if(Base64::isValidEncoding(invalid[i], strlen(invalid[i])))
		{
			std::ostringstream error;
			error << "Base64 valid encodings BVT failed: " << "Invalid encoding \"" << invalid[i] << "\" was deemed valid.";
			throw std::runtime_error(error.str()); 
		}
	}
}

void bvt_encode()
{
	//	A input string and its base64 encoding
	string inputStr = "basic viability test";
	string validEncodedStr = "YmFzaWMgdmlhYmlsaXR5IHRlc3Q=";
	
	//	Check that our encoding is indeed correct
	std::string result = base64_encode(inputStr);
	if(result != validEncodedStr)
		throw std::runtime_error("Base64 BVT failed: Expected encoding \"" + validEncodedStr + "\" but library computed \"" + result + "\" instead.");
	
	if(!Base64::isValidEncoding(result.c_str(), result.length()))
		throw std::runtime_error("Base64 BVT failed: Encoding \"" + result + "\" was deemed invalid by Base64::isValidEncoding.");
	
	//	Check that our decoding is indeed correct
	result = base64_decode(validEncodedStr);
	if(result != inputStr)
		throw std::runtime_error("Base64 BVT failed: Expected decoding \"" + inputStr + "\" but library computed \"" + result + "\" instead.");
}

long getRandomNumber(long min, long max)
{
	//	Returns a number in the [min, max) interval
	return (rand() % (max - min)) + min;
}

std::string getRandomString(uint maxLength)
{	
	int length = getRandomNumber(0, maxLength + 1);
	std::string str;
	
	str.reserve(length + 1);
	for(int i = 0; i < length; i++)
	{
		long choice = getRandomNumber(0, 3);
		
		if(!choice) str += getRandomNumber((int)'A', (int)'Z' + 1);
		else if(choice == 1) str += getRandomNumber((int)'a', (int)'z' + 1);
		else if(choice == 2) str += getRandomNumber((int)'0', (int)'9' + 1);
	}
	
	return str;
}

uint getRandomBuffer(byte * buffer, uint maxLength)
{
	uint numBytes = sizeof(byte);
	ulong maxValue = pow(256, numBytes) - 1;
	
	uint length = getRandomNumber(0, maxLength + 1);
	
	for(uint i = 0; i < length; i++)
	{
		buffer[i] = (byte)getRandomNumber(0, maxValue + 1);
	}
	
	return length;
}

void fuzzyTest()
{
	const uint numInputs = 10000;
	
	//	Test encoding randomly generated alphanumeric strings
	std::string str;
	const uint maxStringLength = 2048;
	
	for(uint i = 0; i < numInputs; i++)
	{
		str = getRandomString(maxStringLength);

		std::string encoded = base64_encode(str);
		if(!Base64::isValidEncoding(encoded.c_str(), encoded.length()))
			throw std::runtime_error("Base64 Fuzzy String test failed: Encoding \"" + encoded + "\" was deemed invalid by Base64::isValidEncoding.");
		
		std::string decoded = base64_decode(encoded);
		
		if(decoded != str)
			throw std::runtime_error("Base64 Fuzzy String test failed: Expected decoding \"" + str + "\" but library computed \"" + decoded + "\" instead.");
	}
	
	//	Test encoding random buffers
	const uint maxBufferLength = 2048;
	byte buffer[maxBufferLength];
	char encodedBuffer[Base64::getEncodedSize(maxBufferLength) + 1];
	byte decodedBuffer[maxBufferLength];
	
	for(uint i = 0; i < numInputs; i++)
	{
		int length = getRandomBuffer(buffer, maxBufferLength);
		
		int encodedLength = Base64::encodeBuffer(buffer, encodedBuffer, length);
		encodedBuffer[encodedLength] = '\0';
		
		if(!Base64::isValidEncoding(encodedBuffer, encodedLength))
		{
			std::ostringstream error;
			error << "Base64 Fuzzy Buffer test failed: Encoding \"" << encodedBuffer << "\" was deemed invalid by Base64::isValidEncoding.";
			throw std::runtime_error(error.str());
		}
		
		Base64::decodeBuffer(encodedBuffer, decodedBuffer, encodedLength);
		
		if(memcmp(buffer, decodedBuffer, length) != 0)
			throw std::runtime_error("Base64 Fuzzy Buffer test failed: Decoding one of the random buffers yielded a different result.");
	}
	
}

struct TestCase {
	const char * encoded;
	const char * decoded;
};

const TestCase g_cases[] = {
	{"U2VuZCByZWluZm9yY2VtZW50cw==", "Send reinforcements" },
	{"Tm93IGlzIHRoZSB0aW1lIGZvciBhbGwgZ29vZCBjb2RlcnMKdG8gbGVhcm4g", "Now is the time for all good coders\nto learn "},
	{"UnVieQ==", "Ruby"},
	{"VGhpcyBpcyBsaW5lIG9uZQpUaGlzIGlzIGxpbmUgdHdvClRoaXMgaXMgbGluZSB0aHJlZQpBbmQgc28gb24uLi4K", "This is line one\nThis is line two\nThis is line three\nAnd so on...\n"},
	{"", ""}
};

void testEncodings()
{
	uint size = sizeof(g_cases)/sizeof(g_cases[0]);
	
	for(uint i = 0; i < size; i++)
	{
		std::string input(g_cases[i].decoded);
		std::string expected(g_cases[i].encoded);
		std::string result(base64_encode(input));
		
		if(result != expected)
			throw std::runtime_error("Base64 sample encodings test failed: Encoding \"" + input + "\" yielded \"" + result + "\" instead of \"" + expected + "\".");
	}
}

void testDecodings()
{
	uint size = sizeof(g_cases)/sizeof(g_cases[0]);
	
	for(uint i = 0; i < size; i++)
	{
		std::string input(g_cases[i].encoded);
		std::string expected(g_cases[i].decoded);
		std::string result(base64_decode(input));
		
		if(result != expected)
			throw std::runtime_error("Base64 sample decodings test failed: Decoding \"" + input + "\" yielded \"" + result + "\" instead of \"" + expected + "\".");
	}
}

/**
 *	Some convenient typedef's, to avoid typical C++ naming clutter.
 */
typedef void (*TestFunc)();
typedef std::map<std::string, TestFunc> TestMap;

/**
 *	Main method just runs all the tests!
 */
int main(int argc, char ** argv)
{
	srand(time(NULL));
	
	TestMap tests;
	TestMap::iterator it;
	
	//	Add all the test cases to our test map.
	tests["1. bvt_encode"] = bvt_encode;
	tests["2. bvt_valid_encoding"] = bvt_valid_encoding;
	tests["3. test_encoding"] = testEncodings;
	tests["4. test_decoding"] = testDecodings;
	tests["5. fuzzy"] = fuzzyTest;
	
	//	Run each test, displaying a success or an error message.
	bool allGood = true;
	bool allBad = true;

	for(it = tests.begin(); it != tests.end(); it++)
	{
		cout << it->first << " test";
		try
		{
			TestFunc test = it->second;
			test();
			
			cout << " finished successfully!" << endl;
			allBad = false;
		}
		catch(std::exception& e)
		{
			allGood = false;
			cout << " FAILED!" << endl;
			cout << e.what() << endl;
		}
	}
	
	//	Print an overall status message, letting the user know which tests failed.
	cout << endl;
	if(allGood)
	{
		cout << "ALL tests finished SUCCESSFULLY!" << endl;
	}
	else
	{
		if(allBad)
			cout << "ALL tests FAILED!" << endl;
		else
			cout << "SOME tests FAILED!" << endl;
	}
	
	return 0;
}
