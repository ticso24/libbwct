/*
 * Copyright (c) 2001,02,08 Bernd Walter Computer Technology
 * Copyright (c) 2008 FIZON GmbH
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#include "bwct.h"

ssize_t
File::read(void *vptr, size_t n) {
	cassert(opened());
	flush();
	ssize_t nread = readn(vptr, n);
	//syslog(LOG_INFO, "%s read = %lld", tinfo().c_str(), LL(nread));
	return nread;
}

ssize_t
File::write(const char *data) {
	String str(data);
	return write(str);
}

ssize_t
File::write(const String& data) {
	ssize_t ret;

	ret = write(data.c_str(), data.length());
	return ret;
}

String
File::readline(size_t maxlength, bool noeoferr) {
	String ret;
	char buf[2];
	ssize_t res;
	size_t pos;

	buf[1] = '\0';
	for (pos = 0; pos < maxlength; pos++) {
		res = read(buf, 1);
		if (noeoferr && res == 0 && pos > 0)
			return ret;
		if (res != 1)
			throw Error(String("read error"));
		if (buf[0] == '\0') {
			throw Error(String("input data contains zero byte"));
		}
		if (buf[0] == '\n') {
			return ret;
		}
		if (buf[0] != '\r') {
			ret += buf;
		}
	}
	throw Error(String("line too long"));
	/* not reached */
	return ret;
}

ssize_t
File::write(const void *vptr, size_t n) {
	cassert(opened());
	ssize_t nwriten = writen(vptr, n);
	//syslog(LOG_INFO, "%s write = %lld", tinfo().c_str(), LL(nwriten));
	return nwriten;
}

ssize_t
File::readn(void *vptr, size_t n) {
	char *ptr = (char*)vptr;
	size_t nleft = n;
	while (nleft > 0) {
		if (rbufsize > 0) {
			// we have something in the readbuffer
			size_t copybytes = (rbufsize < nleft) ? rbufsize : nleft;
			bcopy(rbufpos, ptr, copybytes);
			ptr += copybytes;
			nleft -= copybytes;
			rbufsize -= copybytes;
			rbufpos += copybytes;
		}
		if (nleft > 0) {
			// request is still not satisfied
			ssize_t nread;
			if (nleft < sizeof(readbuf)) {
				// remaining request is smaller than buffersize, so try to fill the buffer in one go
				if ((nread = microread(readbuf, sizeof(readbuf))) < 0) {
					if (errno != EAGAIN && errno != EINTR) {
						return(nread);
					}
					waitread();
				} else {
					if (nread == 0) {
						goto exit;
					}
					rbufsize = nread;
					rbufpos = readbuf;
				}
			} else {
				// remaining request is bigger than buffer, so directly read into caller memory
				if ((nread = microread(ptr, nleft)) < 0) {
					if (errno != EAGAIN && errno != EINTR) {
						return(nread);
					}
					waitread();
				} else {
					if (nread == 0) {
						goto exit;
					}
					nleft -= nread;
					ptr += nread;
				}
			}
		}
	}
exit:
	return (n - nleft);
}

ssize_t
File::writen(const void *vptr, size_t n) {
	char *ptr = (char*)vptr;
	size_t nleft = n;
	while (nleft > 0) {
		ssize_t nwritten;
		if ((nwritten = microwrite(ptr, nleft)) < 0) {
			if (errno != EAGAIN && errno != EINTR)
				return(nwritten);
			waitwrite();
		} else {
			nleft -= nwritten;
			ptr += nwritten;
		}
	}
	return (n);
}

void
File::ncox() {
	// XXX throw
	cassert(fd >= 0);
	int val = fcntl(fd, F_GETFD, 0);
	cassert(val >= 0);
	int res = fcntl(fd, F_SETFD, val & ~FD_CLOEXEC);
	cassert(res >= 0);
}

void
File::cox() {
	// XXX throw
	cassert(fd >= 0);
	int val = fcntl(fd, F_GETFD, 0);
	cassert(val >= 0);
	int res = fcntl(fd, F_SETFD, val | FD_CLOEXEC);
	cassert(res >= 0);
}

String
File::tinfo() const {
	String ret;
	ret << "(" << typeid(*this).name() << "@" << this <<
	    ", fd=" << fd << ", file=" << filename << ")";
	return ret;
}

ssize_t
File::microread(void *vptr, size_t n) {
	return ::read(fd, vptr, n);
}

ssize_t
File::microwrite(const void *vptr, size_t n) {
	return ::write(fd, vptr, n);
}

int
File::opened() const {
	check();
	return (fd >= 0);
}

File::File() {
	rbufpos = NULL;
	rbufsize = 0;
	fd = -1;
}

File::File(const File& file) : Base() {
	rbufpos = NULL;
	rbufsize = 0;
	fd = dup(file.fd);
}

File::File(const String& path, int flags) {
	rbufpos = NULL;
	rbufsize = 0;
	open(path, flags);
}

File::File(int nfd) {
	rbufpos = NULL;
	rbufsize = 0;
	fd = dup(nfd);
}

File::~File() {
	if (opened()) {
		close();
	}
}

void
File::close() {
	cassert(opened());
	::close (fd);
	fd = -1;
}

int
File::flush () {
	return 0;
}

void
File::open(const String& path, int flags, int mode) {
#if HAVE_OPEN64
	if (flags & O_CREAT) {
		fd = ::open64(path.c_str(), flags | O_LARGEFILE, mode);
	} else {
		fd = ::open64(path.c_str(), flags | O_LARGEFILE);
	}
#else
	if (flags & O_CREAT) {
		fd = ::open(path.c_str(), flags, mode);
	} else {
		fd = ::open(path.c_str(), flags);
	}
#endif
	if (fd >= 0)
		filename = path;
	else {
		throw Error(path + ": " + strerror(errno));
	}
}

int64_t
File::lseek(int64_t offset, int whence) {
	return ::lseek(fd, offset, whence);
}

String
File::getpeername() {
	return filename;
}

String
File::getpeeraddr() {
	String tmp;
	return tmp;
}

int
File::ioctl(unsigned long request, void *argp) {
	return ::ioctl(fd, request, (char*)argp);
}

void
File::waitread() {
	if (rbufsize > 0) {
		return;
	}
	mywaitread();
}

void
File::waitwrite() {
	mywaitwrite();
}

void
File::mywaitread() {
	return;
}

void
File::mywaitwrite() {
	return;
}

void *
File::mmap(size_t len, off_t offset) {
	// TODO: autoexpand
	return ::mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
}

int
File::munmap(void *addr, size_t len) {
	return ::munmap(addr, len);
}

String
File::realpath(String path) {
	String ret;
	char resolve_path[PATH_MAX];
	char *res;

	res = ::realpath(path.c_str(), resolve_path);
	if (res == NULL) {
		throw Error(path + ": " + strerror(errno));

	}
	ret = res;
	return ret;

}

String
File::abspath(String path) {
	return realpath(path); // TODO realpath resolves softlinks as well
}

ssize_t
File::sendfile(File &infile) {
	throw Error("sendfile not implemented in class File");
	return 0;
}

FTask::FTask() {
}

FTask::~FTask() {
}

FTask::FTask(const FTask& ft) :
	Thread(ft)
{
}

String
FTask::tinfo() const {
	String ret;
	ret << "(" << typeid(*this).name() << "@" << this << ", file=" <<
	    (file.isinit() ? file->tinfo() : "none") + ")";
	return ret;
}

void
FTask::setfile(File *nfile) {
	cassert(nfile != NULL);
	file = nfile;
}

Stat::Stat() {
	bzero(&s, sizeof(s));
}

Stat::Stat(const String path) {
	bzero(&s, sizeof(s));
	lstat(path);
}

Stat::Stat(File &rhs) {
	fstat(rhs.fd);
}

Stat::~Stat() {
}

void
#if HAVE_OPEN64
Stat::init(const struct stat64 *sp) {
#else
Stat::init(const struct stat *sp) {
#endif
	s.dev = sp->st_dev;
	s.ino = sp->st_ino;
	s.mode = sp->st_mode;
	s.nlink = sp->st_nlink;
	s.uid = sp->st_uid;
	s.gid = sp->st_gid;
	s.rdev = sp->st_rdev;
#ifdef BSD
	s.atime = sp->st_atimespec.tv_sec;
	s.atimeusec = sp->st_atimespec.tv_nsec * 1000;
	s.ctime = sp->st_ctimespec.tv_sec;
	s.ctimeusec = sp->st_ctimespec.tv_nsec * 1000;
	s.mtime = sp->st_mtimespec.tv_sec;
	s.mtimeusec = sp->st_mtimespec.tv_nsec * 1000;
	s.flags = sp->st_flags;
	s.gen = sp->st_gen;
#else
	s.atime = sp->st_atime;
	s.ctime = sp->st_ctime;
	s.mtime = sp->st_mtime;
	s.atimeusec = 0;
	s.ctimeusec = 0;
	s.mtimeusec = 0;
	s.flags = 0;
	s.gen = 0;
#endif
	s.size = sp->st_size;
	s.blocks = sp->st_blocks;
	s.blksize = sp->st_blksize;
}

int
Stat::stat(const String& path) {
	int ret;
#if HAVE_OPEN64
	struct stat64 st;
	ret = ::stat64(path.c_str(), &st);
#else
	struct stat st;
	ret = ::stat(path.c_str(), &st);
#endif
	init(&st);
	return ret;
}

int
Stat::lstat(const String& path) {
	int ret;
#if HAVE_OPEN64
	struct stat64 st;
	ret =  ::lstat64(path.c_str(), &st);
#else
	struct stat st;
	ret =  ::lstat(path.c_str(), &st);
#endif
	init(&st);
	return ret;
}

int
Stat::fstat(int fd) {
	int ret;
#if HAVE_OPEN64
	struct stat64 st;
	ret =  ::fstat64(fd, &st);
#else
	struct stat st;
	ret =  ::fstat(fd, &st);
#endif
	init(&st);
	return ret;
}

bool
Stat::is_link() {
	return S_ISLNK(s.mode);
}

bool
Stat::is_reg() {
	return S_ISREG(s.mode);
}

bool
Stat::is_dir() {
	return S_ISDIR(s.mode);
}

const Stat&
#if HAVE_OPEN64
Stat::operator=(const struct stat64& st) {
#else
Stat::operator=(const struct stat& st) {
#endif
	init(&st);
	return (*this);
}

Dir::Dir() {
	dir = NULL;
	entry = NULL;
}

Dir::~Dir() {
	if (dir != NULL)
		closedir(dir);
}

void
Dir::open(const String& ndir) {
	if (dir != NULL)
		closedir(dir);
	dirname = ndir;
	if (dirname[dirname.length()] != '/')
			dirname += "/";
	dir = opendir(dirname.c_str());
	entry = NULL;
}

int
Dir::read() {
	cassert(dir != NULL);
#if HAVE_READDIR64
        entry = readdir64(dir);
#else
	entry = readdir(dir);
#endif
	if (entry != NULL)
		name = entry->d_name;
	return (entry != NULL);
}

int
Dir::opened() {
	return (dir != NULL);
}
