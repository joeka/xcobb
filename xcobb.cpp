#include <iostream>
#include <fstream>

#include <string.h>
#include <iomanip>

#include <boost/filesystem.hpp>

using namespace std;
namespace fs = boost::filesystem;


int create( char * filename, fs::path working_dir )
{
	fs::recursive_directory_iterator it(working_dir);
	fs::recursive_directory_iterator end;
	vector <fs::path> files;
	while (it != end)
	{
		if( ! fs::is_directory(it->path()) )
			files.push_back( it->path() );
		it++;
	}

	int filecount = files.size();
	int namelength[filecount];
	long filepos[filecount];
	int filesize[filecount];
	vector<string> filepath;

	fstream obbfile;
	obbfile.open( filename, ios::out | ios::binary);
	obbfile.write("XComOBB",7);
	obbfile.write((char*)&filecount,4);

	int listpos = obbfile.tellg();

	char * cbuf = new char[ 256 ];
	char * zeros =new char [12];
	fill( zeros, zeros+12, 0);

	for (int i = 0; i < filecount; i++)
	{
		strcpy( cbuf, "..");
		strcpy( cbuf+2, files[i].string().c_str() + working_dir.string().length() -1);
		namelength[i] = strlen(cbuf) + 1;
		for ( int j = 0; j < namelength[i]; j++)
		{
			if (cbuf[j] == '/')
			{
				cbuf[j] = '\\';
			}
		}
		filepath.push_back (cbuf);

		obbfile.write((char*)&namelength[i],4);
		obbfile.write(cbuf,namelength[i]);
		obbfile.write(zeros, 12);
	}

	for (int i = 0; i < filecount; i++)
	{
		filepos[i] = obbfile.tellg();
		filesize[i] = file_size(files[i]);
		ifstream currentfile;
		currentfile.open(files[i].string().c_str(), ios::in | ios::binary);
		cout << files[i] << endl;
		if( ! currentfile.is_open() )
		{
			cout << "Could not open or create " << files[i].string() << endl;
			continue;
		}
		
		int buffer_size = 500000;
		int progress = 0;
		
		char * buffer = new char[buffer_size];
		
		while ( currentfile.read(buffer, buffer_size) )
		{
			progress += currentfile.gcount();
			obbfile.write(buffer, currentfile.gcount());
		}
		if (currentfile.eof() && currentfile.gcount() > 0)
		{
			obbfile.write(buffer, currentfile.gcount());
		}
		
		currentfile.close();
	}

	obbfile.seekg(listpos);

	for (int i = 0; i < filecount; i++)
	{
		//4B file name length | filename (variable) | 8B pos | 4B length
		obbfile.seekg( 4 + namelength[i], ios_base::cur);
		obbfile.write( (char*)&filepos[i], 8 );
		obbfile.write( (char*)&filesize[i], 4);
	}

	obbfile.close();

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
		cout << "Input file is not an XCom OBB." << endl;
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
		obbfile.seekg( 1, ios_base::cur); // to cut ../ in the beginning of the filepath
		namelength -= 1;

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

int listFiles( char * filename, bool printsize = false )
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
		cout << "Input file is not an XCom OBB." << endl;
		return 1;
	}

	int filecount = 0;
	obbfile.read( (char*)&filecount, 4);

	//cout << "Number of files: " << filecount << endl;

	int namelength = 0;
	int listpos = 0;
	vector<char*> filelist;
	int filesize[filecount];
	int max_width = 0;
	for (int i = 0; i < filecount; i++)
	{
		obbfile.read( (char*)&namelength, 4);

		buffer = new char[namelength];
		obbfile.read( buffer, namelength);

		//4B file name length | filename (variable) | 8B pos | 4B length
		long position = 0;
		int length = 0;
		obbfile.read( (char*)&position, 8);
		obbfile.read( (char*)&length, 4);

		filelist.push_back(buffer);
		filesize[i] = length;

		if ( strlen(buffer) > max_width )
			max_width = strlen(buffer);
	}
	max_width += 3;
	cout << left; // << setw(max_width) << "  FILENAME  " << setw(8) << "FILESIZE" << endl;
	for (int i = 0; i < filecount; i++)
	{
		cout << setw(max_width) << filelist[i];
		if (printsize)
   			cout << setw(8) << filesize[i];
		cout << endl;
	}
}

void printHelp()
{
	cout << "Usage:" << endl;
	cout << "  Extract:  xcobb x obb_file [output_dir]" << endl;
	cout << "  Create:   xcobb c obb_file input_dir" << endl;
	cout << "  List:     xcobb l[s] obb_file" << endl;
	cout << "            (ls will also print the size)" << endl;
}

int main ( int argc, char *argv[])
{
	if( argc < 3 || (argv[1][0] == 'c' && argc < 4) )
	{
		printHelp();
		return 0;
	}

	
	fs::path working_dir(".");
	if( argc > 3 )
	{
		working_dir = argv[3];
	}


	if ( argv[1][0] == 'x' )
		return extract(argv[2], working_dir);
	else if ( argv[1][0] == 'c' )
		return create(argv[2], working_dir);
	else if ( argv[1][0] == 'l' )
		return listFiles(argv[2], (argv[1][1] == 's'));
	else
		printHelp();

	return 0;
}
