/* $Header$ $Log$
/* $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
#ifndef __VFS_H__
#define __VFS_H__

#include <map>

class CReader;
class CFile;
class CVFS;

class CArchive
{
private:
	struct archive_header
	{
		char	barc[4];	// Magic number 'BARC' for BuNg Archive
		uint32	version;	// Should be 1 for now
		uint32	num_files;	// Number of files in archive
	};

	struct archive_file
	{
		char	name[255];	// Null terminated file name and path
		uint32	offset;		// Offset of file data
		uint32	unc_length;	// Length of file data (uncompressed)
		uint32	com_length;	// Length of file data (compressed)
	};

	archive_header header;
	map<string, archive_file> fileEntries;
	CReader *archiveReader;

	void LoadFileEntries();

public:
	CArchive(CReader *r) { archiveReader = r; LoadFileEntries(); }
	CArchive() {} // ?
	~CArchive();
	CReader *LoadFile(string name);
};

class CVFS
{
private:
	map<string, CArchive> archives;
	CReader *LoadFileS(string name);
public:
	static void LoadArchive(string name);
	static CReader *LoadFile(string name);
};

/*
class CFile
{
	friend class CVFS;
	friend class CArchive;
private:
	CReader *reader;
	//CWriter *writer;
	CFile(CReader *r);//, CWriter *w);
public:
	~CFile();
	CReader *GetReader();
	int Read(void *buf, uint32 length);
	int Seek(uint32 offset);
	//CWriter *GetWriter(); // writing isn't yet implemented (low priority)
}; */

////////////////////////////////////////////////////////////////////////////
// READERS follow:

// The Interface to input
class CReader
{
protected:
	CReader *parent;
public:
	virtual ~CReader() {}
	virtual int Read(void *buf, uint32 length) = 0;
	virtual int Seek(uint32 offset) = 0;
	virtual uint32 GetLength() = 0;
};

// A dummy reader that does nothing: this serves as a reader that just doesn't
// delete its parent upon destruction.
class CDummyReader : public CReader
{
public:
	CDummyReader(CReader *r) { parent = r; }
	virtual ~CDummyReader() {}
	virtual int Read(void *buf, uint32 length) { return parent->Read(buf, length); }
	virtual int Seek(uint32 offset) { return parent->Seek(offset); }
	virtual uint32 GetLength() { return parent->GetLength(); }
};

// A reader that relies upon no other readers: just an interface to basic
// file input.
class CFileReader : public CReader
{
private:
	FILE *file;		
public:
	CFileReader(FILE *f) : file(f) {}
	virtual ~CFileReader() { fclose(file); }
	virtual int Read(void *buf, uint32 length);
	virtual int Seek(uint32 offset);
	virtual uint32 GetLength();
	FILE *getFile() { return file; }
};

// A reader for a file stored inside an archive.  This relies upon another
// reader.  For an uncompressed file, that would just be a CFileReader.
class CArchiveReader : public CReader
{
private:
	int archiveOffset, internalOffset, fileLength;
public:
	CArchiveReader(CReader *archive, int archOffset, int length);
	virtual ~CArchiveReader() { delete parent; }
	virtual int Read(void *buf, uint32 length);
	virtual int Seek(uint32 offset);
	virtual uint32 GetLength();
};

// A reader to read (gzip) compressed data
class CGZReader : public CReader
{
private:
	byte *uncompressedBuf;
	int uncompressedLength;
	int internalOffset;
public:
	CGZReader(CReader *f, int uncLength);
	virtual ~CGZReader();
	virtual int Read(void *buf, uint32 length);
	virtual int Seek(uint32 offset);
	virtual uint32 GetLength();
	virtual void Uncompress();
};

class CTextReader : public CReader
{
public:
	CTextReader(CReader *f) { parent = f; }

	virtual int Read(void *buf, uint32 length) { return parent->Read(buf, length); }
	virtual int Seek(uint32 offset) { return parent->Seek(offset); }
	virtual uint32 GetLength() { return parent->GetLength(); }

	// New text-related functions follow
	int ReadLine(char *buf, int maxlen);
};

#endif

