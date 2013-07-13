/**
 *	File:		main.cpp
 *	Author:		Alin Tomescu, tomescu.alin@gmail.com
 *	Website:	http://alinush.org
 *	Date: 		February 20th, 2010
 *	License:	Free to use and distribute as long as this notice is kept.
 */
#include <iostream>
#include <stdexcept>
#include <cstring>
using namespace std;

#include "Base64.h"

int main(int argc, char ** argv)
{
	if(argc < 4)
	{
		cout << argv[0] << " usage: " << endl;
		cout << argv[0] << " [/encode | /decode] <input_file> <output_file>" << endl;
		return -1;
	}
	else
	{
		try
		{
			if(strcmp(argv[1], "/encode") == 0)
			{
				Base64::encodeFile(argv[2], argv[3]);
			}
			else if(strcmp(argv[1], "/decode") == 0)
			{
				Base64::decodeFile(argv[2], argv[3]);
			}
		}
		catch(std::exception& e)
		{
			cerr << "Exception occurred: " << e.what() << endl;
			return -1;
		}
	}
	
	return 0;
}
