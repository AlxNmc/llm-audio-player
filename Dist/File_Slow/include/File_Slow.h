   
#ifndef FILE_SLOW_H
#define FILE_SLOW_H

#include <windows.h> // win32

// Make the assumption of c-char strings, not UNICODE
// 32 bit files, not supporting 64 bits


//typedef HANDLE FileHandle;

class File_Slow
{
public:
	typedef HANDLE Handle;


	enum Error
	{
		SUCCESS = 0xB0000000,
		OPEN_FAIL,
		CLOSE_FAIL,
		WRITE_FAIL,
		READ_FAIL,
		SEEK_FAIL,
		TELL_FAIL,
		FLUSH_FAIL
	};

	enum Seek
	{
		BEGIN = 0xC0000000,
		CURRENT,
		END
	};


	enum Mode
	{
		READ = 0xA0000000,
		WRITE,
		READ_WRITE
	};

public:

   static Error magic(unsigned int &num);
   static Error open( Handle &fh, const char * const fileName, Mode mode );
   static Error close( Handle fh );
   static Error write( Handle fh, const void * const buffer, const size_t inSize );
   static Error read( Handle fh, void * const _buffer, const size_t _size );
   static Error seek( Handle fh, Seek seek, int offset );
   static Error tell( Handle fh, int &offset );
   static Error flush( Handle fh );


private:

   static DWORD privGetFileDesiredAccess( Mode mode );
   static DWORD privGetSeek( Seek seek );
};

#endif