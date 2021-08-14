/*
 * Copyright (c) 2001,02,03,08 Bernd Walter Computer Technology
 * Copyright (c) 2008 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/fdhelper.h $
 * $Date: 2019-10-01 15:09:37 +0200 (Tue, 01 Oct 2019) $
 * $Author: ticso $
 * $Rev: 40730 $
 */

#ifndef _FDHELPER
#define _FDHELPER

class File;
class Stat;

#include "tool.h"
#include "thread.h"
#include "bwct.h"

class Network;

class File : public Base {
	friend class Network;
	friend class Stat;
protected:
	char readbuf[8*1024];
	char* rbufpos;
	size_t rbufsize;
	
protected:
	String filename;
	virtual ssize_t readn(void *vptr, size_t n);
	virtual ssize_t writen(const void *vptr, size_t n);
	virtual void mywaitread();
	virtual void mywaitwrite();
public:
	int fd; // should be protected, but GCC 3.4.6 don't accept Friend status on Network::Net
	virtual int opened() const;
	File();
	File(const File& file);
	File(const String& path, int flags = O_RDONLY);
	File(int nfd);
	~File();
	virtual void close();
	virtual int flush();
	virtual ssize_t microread(void *vptr, size_t n);
	virtual ssize_t microwrite(const void *vptr, size_t n);
	virtual ssize_t read(void *vptr, size_t n);
	virtual ssize_t write(const void *vptr, size_t n);
	virtual ssize_t write(const char *data);
	virtual ssize_t write(const String& data);
	virtual String readline(size_t maxlength = 2048, bool noeoferr = false);
	virtual void open(const String& path, int flags = O_RDONLY, int mode = 0000644);
	virtual int64_t lseek(int64_t offset, int whence = SEEK_SET);
	virtual String getpeername();
	virtual String getpeeraddr();
	virtual int ioctl(unsigned long request, void *argp = NULL);
	virtual void ncox();
	virtual void cox();
	virtual String tinfo() const;
	virtual void *mmap(size_t len, off_t offset, int prot);
	virtual int munmap(void *addr, size_t len);
	virtual ssize_t sendfile(File &infile);
	static String realpath(String path);
	static String abspath(String path);
	void waitread();
	void waitwrite();
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
	struct {
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
	Stat(File &rhs);
	~Stat();
	int stat(const String& path);
	int lstat(const String& path);
	int fstat(int fd);
	bool is_link();
	bool is_reg();
	bool is_dir();
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
	uint8_t type;
	Dir();
	Dir(const Dir& rhs);
	~Dir();
	void open(const String& ndir);
	int read();
	int opened();
};

#endif /* !_FDHELPER */
