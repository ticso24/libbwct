/*
 * Copyright (c) 2002 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#ifndef _COMPRESS
#define _COMPRESS

#include <bwct/fdhelper.h>
#ifdef HAVE_LIBZ
#include <zlib.h>
#endif
#ifdef HAVE_LIBBZ2
#include <bzlib.h>
// some bz2 versions have different function names :(
#ifndef HAVE_BZ2_BZCOMPRESSINIT
#ifdef HAVE_BZCOMPRESSINIT
#define BZ2_bzCompressEnd bzCompressEnd
#define BZ2_bzDecompressEnd bzDecompressEnd
#define BZ2_bzCompress bzCompress
#define BZ2_bzDecompress bzDecompress
#define BZ2_bzCompressInit bzCompressInit
#define BZ2_bzDecompressInit bzDecompressInit
#endif
#endif
#endif

class Cmpfile : public File {
protected:
	bool comp; // true if we compress
	bool iscomp; // true if file is compressed
	virtual ssize_t microread(void *vptr, size_t n) = 0;
	virtual ssize_t microwrite(const void *vptr, size_t n) = 0;
public:
	Cmpfile();
	virtual void cmpinit(int comp, int iscomp) = 0;
	virtual ssize_t readv(SArray<struct iovec>& data);
	virtual ssize_t writev(SArray<struct iovec>& data);
	virtual int64_t lseek(int64_t offset, int whence = SEEK_SET);
	virtual int ioctl(unsigned long request, void *argp = NULL);
	virtual void waitread() = 0;
	virtual void waitwrite() = 0;
};

#ifdef HAVE_LIBZ
class Zfile : public Cmpfile {
protected:
	a_ptr<Matrix<char> > inbuf;
	a_ptr<Matrix<char> > outbuf;
	char *outptr;
	z_stream zs;
	virtual ssize_t microread(void *vptr, size_t n);
	virtual ssize_t microwrite(const void *vptr, size_t n);
	virtual void cmpinit(int comp, int iscomp);
public:
	Zfile();
	~Zfile();
	virtual void close();
	virtual void waitread();
	virtual void waitwrite();
};
#endif

#ifdef HAVE_LIBBZ2
class BZ2file : public Cmpfile {
protected:
	a_ptr<Matrix<char> > inbuf;
	a_ptr<Matrix<char> > outbuf;
	char *outptr;
	bz_stream zs;
	virtual ssize_t microread(void *vptr, size_t n);
	virtual ssize_t microwrite(const void *vptr, size_t n);
	virtual void cmpinit(int comp, int iscomp);
public:
	BZ2file();
	~BZ2file();
	virtual void close();
	virtual void waitread();
	virtual void waitwrite();
};
#endif

#endif /* !_COMPRESS */
