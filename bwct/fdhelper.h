/*
 * Copyright (c) 2001,02,03 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#ifndef _FDHELPER
#define _FDHELPER

class File;
class Stat;

#include <bwct/tool.h>
#include <bwct/thread.h>

class File : public Base {
protected:
	int fd;
	String filename;
	virtual ssize_t microread(void *vptr, size_t n);
	virtual ssize_t microwrite(const void *vptr, size_t n);
public:
	virtual int opened() const;
	File();
	File(const File& file);
	~File();
	virtual void close();
	virtual int flush();
	virtual ssize_t read(void *vptr, size_t n);
	virtual ssize_t write(const void *vptr, size_t n);
	virtual ssize_t readv(SArray<struct iovec>& data);
	virtual ssize_t writev(SArray<struct iovec>& data);
	virtual void open(const String& path, int flags = O_RDONLY);
	virtual int64_t lseek(int64_t offset, int whence = SEEK_SET);
	virtual const String& getpeername();
	virtual int ioctl(unsigned long request, void *argp = NULL);
	virtual void waitread();
	virtual void waitwrite();
	virtual ssize_t readn(void *vptr, size_t n);
	virtual ssize_t writen(const void *vptr, size_t n);
	virtual void ncox();
	virtual void cox();
	virtual String tinfo() const;
#ifdef HAVE_MMAP
	virtual void *mmap(size_t len, off_t offset);
	virtual int munmap(void *addr, size_t len);
#endif
};

class Memfile : public File {
protected:
	ssize_t pos;
public:
	Matrix<char> *data;
	Memfile(Matrix<char>& rhs);
	Memfile(const Memfile& rhs);
	void clear();
	virtual ssize_t read(void *vptr, size_t n);
	virtual ssize_t write(const void *vptr, size_t n);
	virtual ssize_t readv(SArray<struct iovec>& data);
	virtual ssize_t writev(SArray<struct iovec>& data);
	virtual int flush();
	virtual int64_t lseek(int64_t offset, int whence);
};

class FTask : public Thread {
protected:
	a_ptr<File> file;
public:
	FTask();
	~FTask();
	FTask(const FTask& ft);
	void setfile(File *nfile);
	virtual String tinfo() const;
};

class Stat : public Base {
private:
#if HAVE_OPEN64
	void init(const struct stat64 *sp);
	const Stat& operator=(const struct stat64& st);
#else
	void init(const struct stat *sp);
	const Stat& operator=(const struct stat& st);
#endif
public:
	class {
		uint64_t dev;
		uint64_t ino;
		uint64_t mode;
		uint64_t nlink;
		uint64_t uid;
		uint64_t gid;
		uint64_t rdev;
		uint64_t atime;
		uint64_t atimeusec;
		uint64_t mtime;
		uint64_t mtimeusec;
		uint64_t ctime;
		uint64_t ctimeusec;
		uint64_t size;
		uint64_t blocks;
		uint64_t blksize;
		uint64_t flags;
		uint64_t gen;
	} s;

	Stat();
	Stat(const String path);
	~Stat();
	int stat(const String& path);
	int lstat(const String& path);
	int fstat(int fd);
	int is_link();
	int is_reg();
	int is_dir();
};

class Dir : public Base {
private:
	DIR *dir;
#if HAVE_READDIR64
	struct dirent64 *entry;
#else
	struct dirent *entry;
#endif
public:
	String dirname;
	String name;
	Dir();
	Dir(const Dir& rhs);
	~Dir();
	void open(const String& ndir);
	int read();
	int opened();
};

#endif /* !_FDHELPER */
