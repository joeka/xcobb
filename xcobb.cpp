#include <iostream>
#include <fstream>

#include <string.h>

#include <boost/filesystem.hpp>

using namespace std;
namespace fs = boost::filesystem;

int main ()
{
	ifstream obbfile;
	obbfile.open("../main.12.com.tt2kgames.xcomew.obb", ios::in | ios::binary);

	if (! obbfile.is_open() )
	{
		cout << "Could not open file." << endl;
		return 1;
	}

	char * buffer = new char [7];
	obbfile.read (buffer, 7);

	if (strcmp(buffer, "XComOBB") != 0)
	{
		cout << "Input file is not a XCom OBB." << endl;
		return 1;
	}

	int filecount = 0;
	obbfile.read( (char*)&filecount, 4);

	cout << "Number of files: " << filecount << endl;
	cout << "List of files:" << endl;

	int namelength = 0;
	int listpos = 0;
	for (int i = 0; i < 1; i++ )//filecount; i++)
	{
		obbfile.read( (char*)&namelength, 4);
		buffer = new char[namelength];
		obbfile.read( buffer, namelength);
		cout << "  " << buffer << endl;
		//4B file name length | filename (variable) | 8B pos | 4B length
		long position = 0;
		int length = 0;
		obbfile.read( (char*)&position, 8);
		obbfile.read( (char*)&length, 4);
	
		//listpos = obbfile.tellg();	
	
		for ( int j = 0; j < namelength; j++)
		{
			if (buffer[j] == '\\')
			{
				buffer[j] = '/';
			}
		}
		
		fs::path path (buffer);
		if ( ! fs::create_directories(path) )
			cout << "Could not create directory: " << path.string() << endl;

		//obbfile.seekg(listpos);
	}

	obbfile.close();
	return 0;
}
