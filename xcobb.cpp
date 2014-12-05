#include <iostream>
#include <fstream>

#include <string.h>

#include <boost/filesystem.hpp>

using namespace std;
namespace fs = boost::filesystem;


int create( char * filename, fs::path working_dir )
{
	cout << "Please implement me" << endl;
	return 0;
}

int extract( char * filename, fs::path working_dir )
{
	ifstream obbfile;
	obbfile.open(filename, ios::in | ios::binary);

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
	for (int i = 0; i < filecount; i++)
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

		listpos = obbfile.tellg();	
	
		for ( int j = 0; j < namelength; j++)
		{
			if (buffer[j] == '\\')
			{
				buffer[j] = '/';
			}
		}

		fs::path path ( working_dir / buffer);
		
		fs::create_directories(path.parent_path());

		obbfile.seekg(position);
		ofstream of;
		of.open(path.string().c_str(), ios::out | ios::binary);
		if( ! of.is_open() )
		{
			cout << "Could not open or create " << path.string() << endl;
			continue;
		}
		
		int buffer_size = 500000;
		int progress = 0;

		buffer = new char[buffer_size];
		while ( progress < length)
		{
			int cbuf = buffer_size;
			if ( progress + buffer_size > length )
				cbuf = length - progress;

			obbfile.read(buffer, cbuf);
			of.write(buffer, cbuf);
			progress += cbuf;
		}
		of.close();
		//reset list
		obbfile.seekg(listpos);
	}

	obbfile.close();
	return 0;
}

int main ( int argc, char *argv[])
{
	if( argc < 3 || (argv[1][0] == 'c' && argc < 4) )
	{
		cout << "xcobb x obb_file [output_dir]" << endl;
		cout << "xcobb c obb_file input_dir" << endl;
		return 0;
	}

	
	fs::path working_dir("./build");
	if( argc > 3 )
	{
		working_dir = argv[3];
		working_dir = working_dir / "build";
	}


	if ( argv[1][0] == 'x' )
		return extract(argv[2], working_dir);
	else if ( argv[1][0] == 'c' )
		return create(argv[2], working_dir);
	else
		cout << "Unkown command '" << argv[1] << "'." << endl;

	return 0;
}
