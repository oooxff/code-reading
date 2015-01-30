/**
*Copyright (c) 2000-2001, Jim Crafton
*All rights reserved.
*Redistribution and use in source and binary forms, with or without
*modification, are permitted provided that the following conditions
*are met:
*	Redistributions of source code must retain the above copyright
*	notice, this list of conditions and the following disclaimer.
*
*	Redistributions in binary form must reproduce the above copyright
*	notice, this list of conditions and the following disclaimer in 
*	the documentation and/or other materials provided with the distribution.
*
*THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
*AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS
*OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
*EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
*PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
*PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
*LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
*NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
*SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*NB: This software will not save the world.
*/

/* Generated by Together */

#ifndef FILESTREAM_H
#define FILESTREAM_H



namespace VCF  {

class FileStreamPeer;

/**
*special file stream that uses native OS calls
*for file IO as opposed to the more generic C++ filestream class.
*This class uses a FileStreamPeer to do the actual work
*
*@author Jim Crafton
*@version 1.0
*/
class FRAMEWORK_API FileStream : public InputStream, public OutputStream, public Object {
public:
	FileStream( const String& filename, const FileStreamAccessType& accessType );	

	FileStream( const String& filename );

	/**
	*This constructor is used in stream chaining.
	*The flow goes from the FileStream to the outStream.
	*So if a write is called on the FileStream, it simply
	*passes it to the outStream's write() method
	*/
	FileStream( const String& filename, OutputStream* outStream );

	/**
	*This constructor is used in stream chaining.
	*The flow goes from the FileStream to the inStream.
	*So if a read is called on the FileStream, it simply
	*passes it to the inStream's read() method
	*/
	FileStream( const String& filename, InputStream* inStream );

	virtual ~FileStream();


	virtual void seek(const unsigned long& offset, const SeekType& offsetFrom);

	/**
	*returns the size of the stream. The size represents the 
	*number of bytes that have been <b>written</b>, or <b>read</b> to/from the stream
	*/
    virtual unsigned long getSize();

	/**
	*returns a pointer to the buffer that holds the stream memory
	*/
	virtual char* getBuffer();

	//input
	virtual void read( char* bytesToRead, unsigned long sizeOfBytes );

	//output
	virtual void write( char* bytesToWrite, unsigned long sizeOfBytes );

	void init();

	/**
	*closes the file stream
	*/
	void close();

	/**
	*opens a file stream
	*/
	void open( const String& filename, const FileStreamAccessType& accessType );

	void open( const String& filename );
private:
	FileStreamPeer* m_fsPeer;
	String m_filename;
	FileStreamAccessType m_access;
	OutputStream* m_outputStream;
	InputStream* m_inputStream;

};


};
#endif //FILESTREAM_HFILESTREAM_H