#include "stdafx.h"
#include "vfs.h"
#include "exception.h"
#include "reporter.h"
#include "misc.h"
#include <zlib.h>

static CVFS vfs;

void CVFS::LoadArchive(string name) {
	FILE *f = fopen(name.c_str(), "rb");

	if(!f)
		throw CException("Unable to load archive.");

	CFileReader *fr = new CFileReader(f); // NB: this never gets deleted! It really needs fixing at some point
	CArchive arch(fr);

	vfs.archives[name] = arch;
		//.insert( map<string, CArchive>::value_type(name, arch) );
}

CReader *CVFS::LoadFile(string name) {
	return vfs.LoadFileS(name);
}

CReader *CVFS::LoadFileS(string name) {
	string::size_type f = name.find(".barc", 0);
	if(f == string::npos) {
		CFileReader *reader;
		FILE *file = fopen(name.c_str(), "rb");
		
		if(!file) {
			throw CException(string("Unable to open file ") +
				name + string(" for reading."));
		}

		reader = new CFileReader(file); // check for compressed file here in a generic way

		return reader;
	}
	else {
		string::size_type r = name.rfind("/", f), end = f + 5;
		string archiveName, fileName;
		map<string, CArchive>::iterator archiveIter;

		if(r == string::npos)
			r = name.rfind("\\", f);

		if(r == string::npos) {
			archiveName = name.substr(0, end);
			fileName = name.substr(end + 1, name.length() - end);
		}
		else {
			archiveName = name.substr(0, end);
			fileName = name.substr(end + 1, name.length() - end);
		}

		archiveIter = archives.find(archiveName);

		if(archiveIter == archives.end()) {
			throw CException(
				bsprintf("Unknown/unregistered archive given in LoadFile: archive='%s'.", archiveName.c_str())
				);
		}

		return archiveIter->second.LoadFile(fileName);
	}
}
////////////////////////////////////////////////////////////////////////////
// CArchive functions:
CReader *CArchive::LoadFile(string name) {
	map<string, archive_file>::iterator fileIter;

	fileIter = fileEntries.find(name);

	if(fileIter == fileEntries.end()) {
		string::size_type st;
		while( (st = name.find_first_of("\\")) != string::npos) {
			name[st] = '/';
		}
	
		fileIter = fileEntries.find(name);

		if(fileIter == fileEntries.end()) {
			throw CException(string("Bad file in CArchive::LoadFile() - ") + name);
		}
	}

	CReader *reader = new CDummyReader(archiveReader);
	reader = new CArchiveReader(reader, fileIter->second.offset, 
		fileIter->second.com_length == 0 ? fileIter->second.unc_length : 
		fileIter->second.com_length);

	if(fileIter->second.com_length != 0) {
		reader = new CGZReader(reader, fileIter->second.unc_length);
	}

	return reader;
}

void CArchive::LoadFileEntries()
{
	archiveReader->Seek(0);
	archiveReader->Read(&header, sizeof(archive_header));

	if(strncmp(header.barc, "BARC", 4) != 0)
	{
		string err = "Magic numbers for archive ";
		err += " do not match 'BARC'.";
		throw err;
	}

	if(header.version != 1)
	{
		string err = "Archive version unsupported or bad archive file: ";
		err += " version: ";
		err += (uint32)header.version;
		err += " expected: 1";

		throw err;
	}

	fileEntries.clear();

	for(unsigned int i = 0; i < header.num_files; i++)
	{
		archive_file arc_file;

		archiveReader->Read(&arc_file, sizeof(archive_file));

		fileEntries.insert( map<string, archive_file>::value_type(string(arc_file.name), arc_file));
	}
}

CArchive::~CArchive() { }
////////////////////////////////////////////////////////////////////////////
// File reader functions:
int CFileReader::Read(void *buf, uint32 length) {
	fread(buf, length, 1, file);
	return length;
}

int CFileReader::Seek(uint32 offset) {
	return fseek(file, offset, SEEK_SET);
}

uint32 CFileReader::GetLength() {
	int length, offset;
	offset = ftell(file);
	fseek(file, 0, SEEK_END);
	length = ftell(file);
	fseek(file, offset, SEEK_SET);
	return length;
}
////////////////////////////////////////////////////////////////////////////
// Archive reader functions:
CArchiveReader::CArchiveReader(CReader *archive, int archOffset, int length) {
	archiveOffset = archOffset;
	fileLength = length;
	internalOffset = 0;
	parent = archive;
}

int CArchiveReader::Read(void *buf, uint32 length) {
	int read;
	if(internalOffset >= fileLength) {
		throw CEOFException("CArchiveReader went past EOF in Read().");
	}
	parent->Seek(archiveOffset + internalOffset);
	read = parent->Read(buf, length);
	internalOffset += read;
	return read;
}

int CArchiveReader::Seek(uint32 offset)  {
	if(offset > GetLength()) {
		throw CEOFException("CArchiveReader went past EOF in Seek().");
	}
	internalOffset = offset;
	return offset;
}

uint32 CArchiveReader::GetLength()  {
	return fileLength;
}
////////////////////////////////////////////////////////////////////////////
// GZip reader functions:
CGZReader::CGZReader(CReader *f, int uncLength) { 
	parent = f; 
	internalOffset = 0; 
	uncompressedBuf = NULL;
	uncompressedLength = uncLength;
}

CGZReader::~CGZReader()  { 
	if(uncompressedBuf) delete [] uncompressedBuf; 
	delete parent;
}

int CGZReader::Read(void *buf, uint32 length) {
	if(internalOffset + length > (uint32)uncompressedLength) {
		throw CEOFException("CGZReader went past EOF in Read().");
	}

	if(uncompressedBuf == NULL) {
		Uncompress();
	}

	memcpy(buf, (void *)(uncompressedBuf + internalOffset), length);
	internalOffset += length;

	return length;
}

int CGZReader::Seek(uint32 offset) {
	if(offset > GetLength()) {
		throw CEOFException("CGZReader went past EOF in Seek().");
	}
	internalOffset = offset;
	return offset;
}

uint32 CGZReader::GetLength() {
	return uncompressedLength;
}

void CGZReader::Uncompress() {
	uint32 uncLen = uncompressedLength, compressedLength = parent->GetLength();
	int ret;
	byte *compressedBuf;

	compressedBuf = new byte[compressedLength];
	uncompressedBuf = new byte[uncompressedLength];

	parent->Seek(0);
	parent->Read(compressedBuf, compressedLength);

	ret = uncompress(uncompressedBuf, (unsigned long *)&uncLen, compressedBuf, 
			compressedLength);

	if(ret != Z_OK)
	{
		CReporter::Report(CReporter::R_ERROR, "Zlib: uncompress return=%d, should have been %d.", 
			ret, Z_OK);
		throw CException("Error uncompressing in CGZReader");
	}

	delete [] compressedBuf;
}
////////////////////////////////////////////////////////////////////////////
// Text reader functions:
int CTextReader::ReadLine(char *buf, int maxlen)
{
	char c;
	int count = 0;

	do {
		if(count + 1 >= maxlen) {
			buf[count] = '\0';
			return count;
		}

		try {
			parent->Read(&c, 1);
		} catch(CEOFException &e) {
			return -1; /// hmmmmmm...
		}
		
		if(c == '\r') continue;
		if(c == '\n') { 
			buf[count] = '\0';
			return count;
		}
		
		buf[count] = c;

		count++;
	} while(c != '\0');
	return count;
}
////////////////////////////////////////////////////////////////////////////
