/*
 * Copyright (c) 2002 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#include <bwct/base.h>
#include <bwct/compress.h>

Cmpfile::Cmpfile() {
	comp = 0;
	iscomp = 0;
}

int64_t
Cmpfile::lseek(int64_t offset, int whence) {
	throw Error("unavailable");
	/* NOT REACHED */
	return 0;
}

int
Cmpfile::ioctl(unsigned long request, void *argp) {
	throw Error("unavailable");
	/* NOT REACHED */
	return 0;
}

ssize_t
Cmpfile::readv(SArray<struct iovec>& data) {
	if (comp && iscomp)
		throw Error("compressread compressed file");
	if (!comp && !iscomp)
		throw Error("decompressread uncompressed file");
	ssize_t nread = 0;
	for (int i = 0; i <= data.max; i++) {
		ssize_t res = readn(data[i].iov_base, data[i].iov_len);
		nread += res;
		if (res != (ssize_t)data[i].iov_len)
			return nread;
		}
	return nread;
}

ssize_t
Cmpfile::writev(SArray<struct iovec>& data) {
	ssize_t nwritten = 0;
	for (int i = 0; i <= data.max; i++) {
		ssize_t res = writen(data[i].iov_base, data[i].iov_len);
		nwritten += res;
		if (res != (ssize_t)data[i].iov_len)
			return nwritten;
		}
	return nwritten;
}

#ifdef HAVE_LIBZ
Zfile::Zfile() {
}

Zfile::~Zfile() {
	if (opened())
		if (comp)
			deflateEnd(&zs);
		else
			inflateEnd(&zs);
}

void
Zfile::close() {
	if (opened())
		if (comp) {
			int err;
			err = deflate(&zs, Z_FINISH);
			if (err != Z_OK)
				throw Error("Compress error");
			char *ptr = outbuf->get();
			size_t nleft = outbuf->size() - zs.avail_out;
			while (nleft > 0) {
				ssize_t nwritten;
				if ((nwritten = File::microwrite(ptr, nleft)) < 0) {
					if (errno != EAGAIN && errno != EINTR)
						throw Error("zflush failed");
					nwritten = 0;
					File::waitwrite();
				}
				nleft -= nwritten;
				ptr += nwritten;
			}
			deflateEnd(&zs);
		} else
			inflateEnd(&zs);
	File::close();
}

void
Zfile::cmpinit(int comp, int iscomp) {
	inbuf = new Matrix<char>(65536);
	outbuf = new Matrix<char>(65536);
	outptr = outbuf->get();
	zs.zalloc = NULL;
	zs.zfree = NULL;
	zs.opaque = NULL;
	zs.next_in = 0;
	zs.avail_in = 0;
	outptr = outbuf->get();
	zs.next_out = (Bytef*)outbuf->get();
	zs.avail_out = outbuf->size();
	this->comp = comp;
	this->iscomp = iscomp;
	int err;
	if (comp) {
		err = deflateInit(&zs, Z_DEFAULT_COMPRESSION);
		if (err != Z_OK)
			throw Error("deflateInit error");
	} else {
		err = inflateInit(&zs);
		if (err != Z_OK)
			throw Error("inflateInit error");
	}
}

ssize_t
Zfile::microread(void *vptr, size_t n) {
	if (comp && iscomp)
		throw Error("compressread compressed file");
	if (!comp && !iscomp)
		throw Error("decompressread uncompressed file");
	for (;;) {
		if ((char*)zs.next_out != outbuf->get()) {
			size_t len = MIN(n, (size_t)((char*)zs.next_out - outptr));
			bcopy(outptr, vptr, len);
			outptr += len;
			return len;
		}
		while (zs.avail_in == 0) {
			ssize_t res = File::microread(inbuf->get(), inbuf->size());
			if (res < 0) {
				if (errno != EAGAIN && errno != EINTR)
					return res;
				res = 0;
				File::waitread();
			} else
				if (res == 0) {
					if (comp) {
						int err;
						err = deflate(&zs, Z_FINISH);
						if (err != Z_OK)
							throw Error("Compress error");
						if (zs.avail_out == 0) {
							outptr = outbuf->get();
							zs.next_out = (Bytef*)outbuf->get();
							zs.avail_out = outbuf->size();
						}
						if ((char*)zs.next_out != outbuf->get()) {
							size_t len = MIN(n, (size_t)((char*)zs.next_out - outptr));
							bcopy(outptr, vptr, len);
							outptr += len;
							return len;
						}
					}
					return res;
				}
			zs.avail_in = res;
		}
		if (zs.avail_out == 0) {
			outptr = outbuf->get();
			zs.next_out = (Bytef*)outbuf->get();
			zs.avail_out = outbuf->size();
		}
		int err;
		if (comp) {
			err = deflate(&zs, Z_NO_FLUSH);
			if (err != Z_OK)
				throw Error("deflate error");
		} else {
			err = inflate(&zs, Z_NO_FLUSH);
			if (err != Z_OK)
				throw Error("inflate error");
		}
	}
	/* NOT REACHED */
	return -1;
}

ssize_t
Zfile::microwrite(const void *vptr, size_t n) {
	if (comp && !iscomp)
		throw Error("compresswrite uncompressed file");
	if (!comp && iscomp)
		throw Error("decompresswrite compressed file");
	size_t len = MIN(n, inbuf->size());
	bcopy(vptr, inbuf->get(), len);
	zs.next_in = (Bytef*)vptr;
	zs.avail_in = len;
	do {
		outptr = outbuf->get();
		zs.next_out = (Bytef*)outbuf->get();
		zs.avail_out = outbuf->size();
		int err;
		if (comp) {
			err = deflate(&zs, Z_NO_FLUSH);
			if (err != Z_OK)
				throw Error("deflate error");
		} else {
			err = inflate(&zs, Z_NO_FLUSH);
			if (err != Z_OK)
				throw Error("inflate error");
		}
		char *ptr = outbuf->get();
		size_t nleft = outbuf->size() - zs.avail_out;
		while (nleft > 0) {
			ssize_t nwritten;
			if ((nwritten = File::microwrite(ptr, nleft)) < 0) {
				if (errno != EAGAIN && errno != EINTR)
					return(nwritten);
				nwritten = 0;
				File::waitwrite();
			}
			nleft -= nwritten;
			ptr += nwritten;
		}
	} while (zs.avail_out == 0);
	return n;
}

void
Zfile::waitread() {
	if ((char*)zs.next_out != outbuf->get())
		return;
	File::waitread();
}

void
Zfile::waitwrite() {
	File::waitwrite();
}

#endif

#ifdef HAVE_LIBBZ2

BZ2file::BZ2file() {
}

BZ2file::~BZ2file() {
	if (opened())
		if (comp)
			BZ2_bzCompressEnd(&zs);
		else
			BZ2_bzDecompressEnd(&zs);
}

void
BZ2file::close() {
	if (opened())
		if (comp) {
			int err;
			err = BZ2_bzCompress(&zs, BZ_FINISH);
			if (err != BZ_OK)
				throw Error("Compress error");
			char *ptr = outbuf->get();
			size_t nleft = outbuf->size() - zs.avail_out;
			while (nleft > 0) {
				ssize_t nwritten;
				if ((nwritten = File::microwrite(ptr, nleft)) < 0) {
					if (errno != EAGAIN && errno != EINTR)
						throw Error("bzflush failed");
					nwritten = 0;
					File::waitwrite();
				}
				nleft -= nwritten;
				ptr += nwritten;
			}
			BZ2_bzCompressEnd(&zs);
		} else
			BZ2_bzDecompressEnd(&zs);
	File::close();
}

void
BZ2file::cmpinit(int comp, int iscomp) {
	inbuf = new Matrix<char>(65536);
	outbuf = new Matrix<char>(65536);
	outptr = outbuf->get();
	zs.bzalloc = NULL;
	zs.bzfree = NULL;
	zs.opaque = NULL;
	zs.next_in = 0;
	zs.avail_in = 0;
	outptr = outbuf->get();
	zs.next_out = outbuf->get();
	zs.avail_out = outbuf->size();
	this->comp = comp;
	this->iscomp = iscomp;
	int err;
	if (comp) {
		err = BZ2_bzCompressInit(&zs, 6, 0, 0);
		if (err != BZ_OK)
			throw Error("BZ2_bzCompressInit error");
	} else {
		err = BZ2_bzDecompressInit(&zs, 0, 0);
		if (err != BZ_OK)
			throw Error("BZ2_bzDecompressInit error");
	}
}

ssize_t
BZ2file::microread(void *vptr, size_t n) {
	if (comp && iscomp)
		throw Error("compressread compressed file");
	if (!comp && !iscomp)
		throw Error("decompressread uncompressed file");
	for (;;) {
		if ((char*)zs.next_out != outbuf->get()) {
			size_t len = MIN(n, (size_t)((char*)zs.next_out - outptr));
			bcopy(outptr, vptr, len);
			outptr += len;
			return len;
		}
		while (zs.avail_in == 0) {
			ssize_t res;
			res = File::microread(inbuf->get(), inbuf->size());
			if (res < 0) {
				if (errno != EAGAIN && errno != EINTR)
					return res;
				res = 0;
				File::waitread();
			} else
				if (res == 0) {
					if (comp) {
						int err;
						err = BZ2_bzCompress(&zs, BZ_FINISH);
							if (err != BZ_OK)
								throw Error("Compress error");
						if (zs.avail_out == 0) {
							outptr = outbuf->get();
							zs.next_out = outbuf->get();
							zs.avail_out = outbuf->size();
						}
						if ((char*)zs.next_out != outbuf->get()) {
							size_t len = MIN(n, (size_t)((char*)zs.next_out - outptr));
							bcopy(outptr, vptr, len);
							outptr += len;
							return len;
						}
						return res;
					}
				}
			zs.avail_in = res;
		}
		if (zs.avail_out == 0) {
			outptr = outbuf->get();
			zs.next_out = outbuf->get();
			zs.avail_out = outbuf->size();
		}
		int err;
		if (comp) {
			err = BZ2_bzCompress(&zs, BZ_RUN);
			if (err != BZ_OK)
				throw Error("Compress error");
		} else {
			err = BZ2_bzDecompress(&zs);
			if (err != BZ_OK)
				throw Error("Decompress error");
		}
	}
	/* NOT REACHED */
	return -1;
}

ssize_t
BZ2file::microwrite(const void *vptr, size_t n) {
	if (comp && !iscomp)
		throw Error("compresswrite uncompressed file");
	if (!comp && iscomp)
		throw Error("decompresswrite compressed file");
	size_t len = MIN(n, inbuf->size());
	bcopy(vptr, inbuf->get(), len);
	zs.next_in = (char*)vptr;
	zs.avail_in = len;
	do {
		outptr = outbuf->get();
		zs.next_out = outbuf->get();
		zs.avail_out = outbuf->size();
		int err;
		if (comp) {
			err = BZ2_bzCompress(&zs, BZ_RUN);
			if (err != BZ_OK)
				throw Error("BZ2_bzCompress error");
		} else {
			err = BZ2_bzDecompress(&zs);
			if (err != BZ_OK)
				throw Error("BZ2_bzDecompress error");
		}
		char *ptr = outbuf->get();
		size_t nleft = outbuf->size() - zs.avail_out;
		while (nleft > 0) {
			ssize_t nwritten;
			if ((nwritten = File::microwrite(ptr, nleft)) < 0) {
				if (errno != EAGAIN && errno != EINTR)
					return(nwritten);
				nwritten = 0;
				File::waitwrite();
			}
			nleft -= nwritten;
			ptr += nwritten;
		}
	} while (zs.avail_out == 0);
	return n;
}

void
BZ2file::waitread() {
	if ((char*)zs.next_out != outbuf->get())
		return;
	File::waitread();
}

void
BZ2file::waitwrite() {
	File::waitwrite();
}

#endif

