/*
 * Copyright (c) 2001,02,03,04,08,18 Bernd Walter Computer Technology
 * Copyright (c) 2008 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/network.cc $
 * $Date: 2021-02-09 00:52:20 +0100 (Tue, 09 Feb 2021) $
 * $Author: ticso $
 * $Rev: 43854 $
 */

#include "bwct.h"

SArray<int> fdescs_to_close;

Network::Net::Net()
{
	timeout = INFTIM;
	delayed_peerretrieve = false;
}

Network::Net::Net(int nfd)
{
	fd = nfd;
	cassert(opened());
	timeout = INFTIM;
	canon = 0;
	delayed_peerretrieve = true;
}

Network::Net::~Net()
{
}

void
Network::Net::post_init_peer()
{
	if (delayed_peerretrieve) {
		Mutex::Guard mtx(delay_mtx);
		if (delayed_peerretrieve) {
			retrievepeername();
			delayed_peerretrieve = false;
		}
	}
}

String
Network::Net::getpeername()
{
	post_init_peer();
	return peername;
}

String
Network::Net::getpeeraddr()
{
	post_init_peer();
	return peeraddr;
}

void
Network::Net::connect_tcp4(const String& name, const String& port)
{
	connect_tcp(name, port, AF_INET);
}

void
Network::Net::connect_tcp6(const String& name, const String& port)
{
	connect_tcp(name, port, AF_INET6);
}

ssize_t
Network::Net::read(void *vptr, size_t n)
{
	return File::read(vptr, n);
}

ssize_t
Network::Net::write(const void *vptr, size_t n)
{
	return File::write(vptr, n);
}

ssize_t
Network::Net::write(const char *data)
{
	return File::write(data);
}

ssize_t
Network::Net::write(const String& data)
{
	return File::write(data);
}

void
Network::Net::settimeout(int nval)
{
	timeout = nval;
}

void
Network::Net::nonblocking(bool flag)
{
	int val;
	val = fcntl(fd, F_GETFL, 0);
	if (flag) {
		fcntl(fd, F_SETFL, val | O_NONBLOCK);
	} else {
		fcntl(fd, F_SETFL, val & ~O_NONBLOCK);
	}
}

void
Network::Net::nodelay(int flag)
{
	int val;

	cassert(fd >= 0);
	val = flag;
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
}

void
Network::Net::connect_UDS (const String& path)
{
	// XXX limit pathname!
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	int val;
	val = SOCKSBUF;
	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val));
	val = SOCKSBUF;
	setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &val, sizeof(val));
	struct sockaddr_un addr;
	bzero(&addr, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, path.c_str());
	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
		::close (fd);
		fd = -1;
		throw Error(String("connecting ") + path + " failed");
	}
	nonblocking(1);
	peername = sgethostname();
}

void
Network::Net::connect_tcp(const Array<String>& names, const String& port, int ms_timeout, int family)
{
	bool connected = false;

	for (int i = 0; i <= names.max && !connected; i++) {
		struct addrinfo *info;
		struct addrinfo *infosave;
		struct addrinfo hints;
		const char *cname;
		const char *cport;
		int res;
		int val;

		cname = (names[i].length() == 0) ? NULL : names[i].c_str();
		cport = (port.length() == 0) ? NULL : port.c_str();
		bzero(&hints, sizeof(hints));
		hints.ai_flags = AI_CANONNAME;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_family = family;
		res = getaddrinfo(cname, cport, &hints, &info);
		infosave = info;
		if (res != 0) {
			syslog(LOG_NOTICE, "getaddrinfo failed for %s: %s", names[i].c_str(), gai_strerror(res));
		} else {
			do {
				fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
				if (!opened()) {
					freeaddrinfo(infosave);
					throw Error("failed to get socket");
				}
				val = SOCKSBUF;
				setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val));
				val = SOCKSBUF;
				setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &val, sizeof(val));
				nonblocking(1);
				res = connect(fd, info->ai_addr, info->ai_addrlen);
				if (res == 0) {
					// we have a connection
					connected = true;
					break;
				} else if (errno == EINPROGRESS) {
					// we have to wait for connection
					struct pollfd pfd;
					int res;

					bzero(&pfd, sizeof(pfd));
					pfd.fd = fd;
					pfd.events = POLLOUT; // POLLOUT for connected, POLLOUT and POLLIN for failed
					do {
						// XXX in case of interrupted system calls we wait full
						// time again, but it's not likely to happen
						res = poll(&pfd, 1, ms_timeout);
					} while (res == -1 && errno == EINTR);
					if (res == 1) {
						// test if we have a connection or an error
						int errval;
						socklen_t size = sizeof(errval);
						int res = getsockopt(fd, SOL_SOCKET, SO_ERROR, &errval, &size);
						if (res == 0 && errval == 0) {
							connected = true;
						} else {
							close();
						}
					} else {
						// we have a timeout or other connection problem
						// close socket and try next resource
						close();
					}
				} else {
					// likely a local connection refused
					close();
				}
			} while (!connected && (info = info->ai_next) != NULL);
			if (connected) {
				peeraddr = "";
				if (info->ai_canonname != NULL) {
					peername = info->ai_canonname;
				} else {
					peername = names[i];
				}
				peerport = port;
			}
			freeaddrinfo(infosave);
		}
	}
	if (!connected) {
		throw Error(String("connecting [") + S.join(names, ", ") + "]:" +
		    port + " failed: " + get_strerror(errno));
	}
}

void
Network::Net::connect_tcp(const String& name, const String& port, int family)
{
	struct addrinfo *info;
	struct addrinfo *infosave;
	struct addrinfo hints;
	const char *cname;
	const char *cport;
	int res;
	int val;

	cname = (name.length() == 0) ? NULL : name.c_str();
	cport = (port.length() == 0) ? NULL : port.c_str();
	bzero(&hints, sizeof(hints));
	hints.ai_flags = AI_CANONNAME;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = family;
	res = getaddrinfo(cname, cport, &hints, &info);
	infosave = info;
	if (res) {
		syslog(LOG_NOTICE, "getaddrinfo failed for %s: %s",
		name.c_str(), gai_strerror(res));
		throw Error(String("getaddrinfo [") + name + "]:" +
		    port + " failed");
	}
	do {
		fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
		if (!opened()) {
			freeaddrinfo(infosave);
			throw Error("failed to get socket");
		}
		val = SOCKSBUF;
		setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val));
		val = SOCKSBUF;
		setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &val, sizeof(val));
		res = connect(fd, info->ai_addr, info->ai_addrlen);
		if (res == 0) {
			nonblocking(1);
			break;
		} else {
			close();
		}
	} while ((info = info->ai_next) != NULL);
	if (res != 0) {
		freeaddrinfo(infosave);
		throw Error(String("connecting [") + name + "]:" +
		    port + " failed: " + get_strerror(errno));
	}
	peeraddr = "";
	if (info->ai_canonname != NULL) {
		peername = info->ai_canonname;
	} else {
		peername = name;
	}
	peerport = port;
	freeaddrinfo(infosave);
}

int
Network::Net::retrievepeername()
{
	struct addrinfo *addrn;
	struct addrinfo *addr0;
	struct addrinfo hints;
	int res;
	a_ptr<char> port;
	port = new char[NI_MAXHOST];
	a_ptr<char> ip;
	ip = new char[NI_MAXHOST];
	a_ptr<char> name;
	name = new char[NI_MAXHOST];
	int ret;
	socklen_t addrlen;

	check();
	cassert(opened());

	addr0 = NULL;
	ret = 0;
	strcpy(port.get(), "unresolved");
	strcpy(ip.get(), "unresolved");
	strcpy(name.get(), "unresolved");

	a_ptr<char> addrdt;
	addrdt = new char[SOCK_MAXADDRLEN];
	struct sockaddr *addr = (struct sockaddr*)addrdt.get();
	addrlen = SOCK_MAXADDRLEN;
	res = ::getsockname(fd, addr, &addrlen);
	if (res < 0) {
		goto failed;
	}

	if (addr->sa_family == AF_UNIX) {
		peerport = "";
		peeraddr = "";
		peername = sgethostname();
		return 0;
	}

	res = ::getpeername(fd, addr, &addrlen);
	if (res < 0) {
		goto failed;
	}

	/* get the reversemapping and portname for the client */
	res = getnameinfo(addr, addrlen, name.get(), NI_MAXHOST, NULL, 0, 0);
	if (res != 0) {
		name.get()[0] = '\n';
	}
	res = getnameinfo(addr, addrlen, ip.get(), NI_MAXHOST, port.get(),
	    NI_MAXHOST, NI_NUMERICHOST | NI_NUMERICSERV);
	if (res != 0) {
		ip.get()[0] = '\n';
		port.get()[0] = '\n';
	}

	/* now check the results with the forwardmapping */
	bzero(&hints, sizeof(hints));
	hints.ai_family = addr->sa_family;
	res = getaddrinfo(name.get(), NULL, &hints, &addr0);
	if (res) { /* we failed to forward map */
		strncpy (name.get(), ip.get(), NI_MAXHOST);
		goto failed;
	}
	/*
	 * now check if the original IP is in the set returned by the
	 * forward check
	 */
	for (addrn = addr0; addrn; addrn = addrn->ai_next) {
		if (addrn->ai_family == AF_INET) {
			res = memcmp(
			    &((struct sockaddr_in*)addrn->ai_addr)->sin_addr,
			    &((struct sockaddr_in*)&addr)->sin_addr,
			    sizeof(struct in_addr));
			if (res == 0) {
				goto ok;
			}
		} else if (addrn->ai_family == AF_INET6) {
			res = memcmp(
			    &((struct sockaddr_in6*)addrn->ai_addr)->sin6_addr,
			    &((struct sockaddr_in6*)addr)->sin6_addr,
			    sizeof(struct in6_addr));
			if (res == 0) {
				goto ok;
			}
		}
	}
	/* reversemapping isn't backed by forward mapping */
failed:
	ret = -1;
ok:
	if (addr0 != NULL) {
		freeaddrinfo(addr0);
	}
	peername = name.get();
	peeraddr = ip.get();
	peerport = port.get();
	return ret;
}

ssize_t
Network::Net::sendfile(File &infile)
{
	off_t offset = 0;
	off_t sbytes;
	int res;
	int safeerrno;

	nonblocking(0);
	do {
		res = ::sendfile(infile.fd, fd, offset, 0, NULL, &sbytes, 0);
		offset += sbytes;
		if (res < 0 && errno == EAGAIN) {
			waitwrite();
		}
	} while (res < 0 && (errno == EAGAIN || errno == EINTR));
	safeerrno = errno;
	nonblocking(1);
	if (res < 0) {
		throw Error(String("sendfile: ") + get_strerror(safeerrno));
	}
	return (ssize_t)offset;
}

void
Network::Net::mywaitread()
{
	struct pollfd pfd;
	int res;

	bzero(&pfd, sizeof(pfd));
	pfd.fd = fd;
	pfd.events = POLLIN;
	do {
		res = poll(&pfd, 1, timeout);
	} while (res == -1);
	if (res == 0) {
		throw Error("read timeout", false);
	}
	if (pfd.revents & POLLHUP) {
		throw Error("socket disconnected");
	}
	if (pfd.revents & POLLERR) {
		throw Error("poll error");
	}
	if (pfd.revents & POLLNVAL) {
		throw Error("poll invalid descriptor");
	}
	return;
}

void
Network::Net::mywaitwrite()
{
	struct pollfd pfd;
	int res;

	bzero(&pfd, sizeof(pfd));
	pfd.fd = fd;
	pfd.events = POLLOUT;
	do {
		res = poll(&pfd, 1, timeout);
	} while (res == -1);
	if (res == 0) {
		throw Error("write timeout");
	}
	if (pfd.revents & POLLHUP) {
		throw Error("socket disconnected");
	}
	if (pfd.revents & POLLERR) {
		throw Error("poll error");
	}
	if (pfd.revents & POLLNVAL) {
		throw Error("poll invalid descriptor");
	}
	return;
}

String
Network::Net::tinfo() const
{
	String ret;
	ret << "(" << typeid(*this).name() << "@" << this <<
	    ", fd=" << fd << ", peer=" << peeraddr << ")";
	return ret;
}

Network::Listen::Listen() {
}

Network::Listen::~Listen()
{
	for (int i = 0; i < lfds.max; i++) {
		for (int j = 0; j <= fdescs_to_close.max; j++) {
			if (fdescs_to_close[j] == lfds[i]) {
				fdescs_to_close.del(j);
				j--;
			}
		}
		close(lfds[i]);
	}
}

int
Network::Listen::add_tcpv4(const String& name, const String& port, int queuelen)
{
	return add_tcp(name, port, queuelen, AF_INET);
}

int
Network::Listen::add_tcpv6(const String& name, const String& port, int queuelen)
{
	return add_tcp(name, port, queuelen, AF_INET6);
}

void
Network::Listen::loop()
{
	// TODO: use kevent
	cassert (lfds.max >= 0);
	for (;;) {
		struct pollfd pfd[lfds.max + 1];
		int res;
		for (int i = 0; i <= lfds.max; i++) {
			bzero(&pfd[i], sizeof(struct pollfd));
			pfd[i].fd = lfds[i];
			pfd[i].events = POLLIN;
		}
		do {
			res = poll(pfd, lfds.max + 1, INFTIM);
		} while (res < 0);
		for (int i = 0; i <= lfds.max; i++) {
			if (pfd[i].revents) {
				int clientfd = accept(lfds[i], NULL, NULL);
				if (clientfd < 0) {
					if (errno == EINTR || errno == EAGAIN) {
						continue;
					}
				} else {
					pid_t child = fork();
					if (child == 0) {
						try {
							/// close listen FDs
							for (int j = 0; j <= fdescs_to_close.max; j++) {
								close(fdescs_to_close[j]);
							}
							/// process request
							FTask *newthread = newtask();
							newthread->setfile(newcon(clientfd));
							newthread->threadstart();
						} catch (std::exception& e) {
							syslog(LOG_INFO, "exception: %s", e.what());
							exit(1);
						} catch (...) {
							syslog(LOG_INFO, "unknown exception");
							exit(1);
						}
						exit(0);
					}
					close (clientfd);
					if (child != -1) {
						/// TODO bookkeeping
					}
				}
			}
		}
	}
}

void
Network::Listen::addfd(int nfd)
{
	lfds << nfd;
	fdescs_to_close << nfd;
}

int
Network::Listen::add_UDS(const String& path, int flags, int queuelen)
{
	struct sockaddr_un aun;
	int lfd;
	int val;

	// XXX limit pathname!
	// XXX failed exception?!
	cassert(path != "");
	unlink(path.c_str());
	bzero(&aun, sizeof(aun));
	aun.sun_family = AF_UNIX;
	strcpy(aun.sun_path, path.c_str());
	lfd = socket(AF_UNIX, SOCK_STREAM, 0);
	val = SOCKSBUF;
	setsockopt(lfd, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val));
	val = SOCKSBUF;
	setsockopt(lfd, SOL_SOCKET, SO_SNDBUF, &val, sizeof(val));
	bind(lfd, (struct sockaddr*)&aun, sizeof(aun));
	listen(lfd, queuelen);
	val = fcntl(lfd, F_GETFL, 0);
	fcntl(lfd, F_SETFL, val | O_NONBLOCK);
	chmod(path.c_str(), flags);
	addfd(lfd);
	return 0;
}

int
Network::Listen::add_tcp(const String& name, const String& port, int queuelen, int family)
{
	struct addrinfo *info;
	struct addrinfo *infosave;
	struct addrinfo hints;
	int res;
	int lfd;
	int val;
	const char *cname;
	const char *cport;

	// XXX failed exception?!
	bzero(&hints, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = family;
	cname = (name.length() == 0) ? NULL : name.c_str();
	cport = (port.length() == 0) ? NULL : port.c_str();
	res = getaddrinfo(cname, cport, &hints, &info);
	if (res) {
		syslog(LOG_EMERG, "getaddrinfo failed: %s", gai_strerror(res));
		cassert(0);
	}
	infosave = info;
	do {
		lfd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
		cassert(lfd >= 0);
		cox(lfd);
		val = 1;
		setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &val,
		    sizeof(val));
		val = SOCKSBUF;
		setsockopt(lfd, SOL_SOCKET, SO_RCVBUF, &val,
		    sizeof(val));
		val = SOCKSBUF;
		setsockopt(lfd, SOL_SOCKET, SO_SNDBUF, &val,
		sizeof(val));
		bind(lfd, info->ai_addr, info->ai_addrlen);
		listen(lfd, queuelen);
		val = fcntl(lfd, F_GETFL, 0);
		fcntl(lfd, F_SETFL, val | O_NONBLOCK);
		addfd(lfd);
	} while ((info = info->ai_next) != NULL);
	freeaddrinfo(infosave);
	return 0;
}

void
Network::Listen::ncox(int fd)
{
	cassert(fd >= 0);
	int val = fcntl(fd, F_GETFD, 0);
	cassert(val >= 0);
	int res = fcntl(fd, F_SETFD, val & ~FD_CLOEXEC);
	cassert(res >= 0);
}

void
Network::Listen::cox(int fd)
{
	cassert(fd >= 0);
	int val = fcntl(fd, F_GETFD, 0);
	cassert(val >= 0);
	int res = fcntl(fd, F_SETFD, val | FD_CLOEXEC);
	cassert(res >= 0);
}

Network::Net *
Network::Listen::newcon(int clientfd)
{
	return new Net(clientfd);
}

/*
 * function to get round trip time by association ID
 * for documentation only
 */
uint32_t
sctp_getpeerrtt(int sc, sctp_assoc_t id)
{
	uint32_t ret;
	struct sctp_paddrinfo info;
	socklen_t len = sizeof(info);
	sctp_opt_info(sc, id, SCTP_GET_PEER_ADDR_INFO, &info, &len);
	// XXX error handling
	ret = info.spinfo_srtt;
	return ret;
}

/*
 * function to get round trip time by address
 * for documentation only
 */
uint32_t
sctp_address_to_rtt(int sc, struct sockaddr *sa, socklen_t salen)
{
	uint32_t ret = 0xffffffff;
	struct sctp_paddrinfo info;
	bcopy(sa, &info.spinfo_address, salen);
	socklen_t len = sizeof(info);
	if (sctp_opt_info(sc, 0, SCTP_GET_PEER_ADDR_INFO, &info, &len) == 0) {
		ret = info.spinfo_srtt;
	}
	return ret;
}

/*
 * function to get the association ID by address
 * for documentation only
 */
sctp_assoc_t
sctp_address_to_associd(int sc, struct sockaddr *sa, socklen_t salen)
{
	sctp_assoc_t ret;
	struct sctp_paddrparams sp;
	bzero(&sp, sizeof(sp));
	socklen_t len = sizeof(sp);
	bcopy(sa, &sp.spp_address, salen);
	sctp_opt_info(sc, 0, SCTP_PEER_ADDR_PARAMS, &sp, &len);
	// XXX error handling
	ret = sp.spp_assoc_id;
	return ret;
}
