/*
 * Copyright (c) 2001,02 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#include <bwct/config.h>
#include <bwct/tool.h>
#include <bwct/fdhelper.h>

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

ssize_t
File::readv(SArray<struct iovec>& data) {
	cassert(opened());
	ssize_t nread = 0;
	flush();
	int iovnum;
	int iovcnt = data.max + 1;
	cassert(iovcnt > 0);
	Matrix<struct iovec> iov(iovcnt);
	for (int i = 0; i < iovcnt; i++)
		iov[i] = data[i];
	for (iovnum = 0; iovcnt > 0;) {
		ssize_t sz = ::readv(fd, &iov[iovnum], MIN(iovcnt, IOV_MAX));
		if (sz == 0)
			return nread;
		if (sz < 0) {
			if (errno != EAGAIN)
				return -1;
			waitread();
		}
		else {
			nread += sz;
			while (sz > 0) {
				if ((size_t)iov[iovnum].iov_len >= (size_t)sz) {
					sz -= iov[iovnum].iov_len;
					iovnum++;
					iovcnt--;
				} else {
					iov[iovnum].iov_base =
					    (char*)(iov[iovnum].iov_base) + sz;
					iov[iovnum].iov_len -= sz;
					sz = 0;
				}
			}
		}
	}
	syslog(LOG_INFO, "%s readv = %lld", tinfo().c_str(), LL(nread));
	return nread;
}

ssize_t
File::writev(SArray<struct iovec>& data) {
	cassert(opened());
	ssize_t nwriten = 0;
	int iovnum;
	int iovcnt = data.max + 1;
	cassert(iovcnt > 0);
	Matrix<struct iovec> iov(iovcnt);
	for (int i = 0; i < iovcnt; i++)
		iov[i] = data[i];
	for (iovnum = 0; iovcnt > 0;) {
		ssize_t sz = ::writev(fd, &iov[iovnum], MIN(iovcnt, IOV_MAX));
		if (sz == 0)
			return nwriten;
		if (sz < 0) {
			if (errno != EAGAIN)
				return -1;
			waitwrite();
		}
		else {
			nwriten += sz;
			while (sz > 0) {
				if ((size_t)iov[iovnum].iov_len >= (size_t)sz) {
					sz -= iov[iovnum].iov_len;
					iovnum++;
					iovcnt--;
				} else {
					iov[iovnum].iov_base =
					    (char*)(iov[iovnum].iov_base) + sz;
					iov[iovnum].iov_len -= sz;
					sz = 0;
				}
			}
		}
	}
	syslog(LOG_INFO, "%s writev = %lld", tinfo().c_str(), LL(nwriten));
	return nwriten;
}

ssize_t
File::read(void *vptr, size_t n) {
	cassert(opened());
	flush();
	ssize_t nread = readn(vptr, n);
	syslog(LOG_INFO, "%s read = %lld", tinfo().c_str(), LL(nread));
	return nread;
}

ssize_t
File::write(const void *vptr, size_t n) {
	cassert(opened());
	ssize_t nwriten = writen(vptr, n);
	syslog(LOG_INFO, "%s write = %lld", tinfo().c_str(), LL(nwriten));
	return nwriten;
}

ssize_t
File::readn(void *vptr, size_t n) {
	char *ptr = (char*)vptr;
	size_t nleft = n;
	while (nleft > 0) {
		ssize_t nread;
		if ((nread = microread(ptr, nleft)) < 0) {
			if (errno != EAGAIN)
				return(nread);
			nread = 0;
			waitread();
		} else {
			if (nread == 0)
				break;
		}
		nleft -= nread;
		ptr += nread;
	}
	return (n - nleft);
}

ssize_t
File::writen(const void *vptr, size_t n) {
	char *ptr = (char*)vptr;
	size_t nleft = n;
	while (nleft > 0) {
		ssize_t nwritten;
		if ((nwritten = microwrite(ptr, nleft)) < 0) {
			if (errno != EAGAIN)
				return(nwritten);
			nwritten = 0;
			waitwrite();
		}
		nleft -= nwritten;
		ptr += nwritten;
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
Memfile::read(void *vptr, size_t n) {
	ssize_t len = (n < data->size() - pos) ? n : data->size() - pos;
	memcpy(vptr, &(*data)[pos], len);
	pos += len;
	return len;
}

ssize_t
Memfile::write(const void *vptr, size_t n) {
	ssize_t len = (n < data->size() - pos) ? n : data->size() - pos;
	memcpy(&(*data)[pos], vptr, len);
	pos += len;
	return len;
}

ssize_t
Memfile::readv(SArray<struct iovec>& data) {
	/* TODO */
	cassert(0);
	return 0;
}

ssize_t
Memfile::writev(SArray<struct iovec>& data) {
	/* TODO */
	cassert(0);
	return 0;
}

String
FTask::tinfo() const {
	String ret;
	ret << "(" << typeid(*this).name() << "@" << this << ", file=" <<
	    (file.isinit() ? file->tinfo() : "none") + ")";
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
	fd = -1;
}

File::File(const File& file) {
	fd = dup(file.fd);
}

File::~File() {
	if (opened())
		::close(fd);
}

void
File::close() {
	cassert(opened());
	if (opened()) {
		::close (fd);
		fd = -1;
	}
}

int
File::flush () {
	return 0;
}

void
File::open(const String& path, int flags) {
#if HAVE_OPEN64
	fd = ::open64(path.c_str(), flags | O_LARGEFILE);
#else
	fd = ::open(path.c_str(), flags);
#endif
	if (fd >= 0)
		filename = path;
	else
		filename = "";
}

int64_t
File::lseek(int64_t offset, int whence) {
	return ::lseek(fd, offset, whence);
}

const String&
File::getpeername() {
	return filename;
}

int
File::ioctl(unsigned long request, void *argp) {
	return ::ioctl(fd, request, (char*)argp);
}

void
File::waitread() {
	return;
}

void
File::waitwrite() {
	return;
}

#ifdef HAVE_MMAP
void *
File::mmap(size_t len, off_t offset) {
	// TODO: autoexpand
	return ::mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
}
#endif

#ifdef HAVE_MMAP
int
File::munmap(void *addr, size_t len) {
	return munmap(addr, len);
}
#endif

Memfile::Memfile(Matrix<char>& rhs) {
	data = &rhs;
	pos = 0;
}

void
Memfile::clear() {
	pos = 0;
	bzero(data->get(), data->size());
}

int
Memfile::flush () {
	return 0;
}

int64_t
Memfile::lseek(int64_t offset, int whence) {
	/* TODO */
	cassert(0);
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

Stat::~Stat() {
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

int
Stat::is_link() {
	return S_ISLNK(s.mode);
}

int
Stat::is_reg() {
	return S_ISREG(s.mode);
}

int
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
	if (*dirname[dirname.length()] != '/')  
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

