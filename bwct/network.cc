/*
 * Copyright (c) 2001,02,03 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#include <bwct/base.h>
#include <bwct/network.h>

Network::Net::Net(int nfd) {
	fd = nfd;
	cassert(opened());
	canon = 0;
	retrievepeername();
}

void
Network::Net::connect_UDS (const String& path) {
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
	val = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, val | O_NONBLOCK);
	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0)
		throw Error(String("connecting ") + path + " failed");
	peername = sgethostname();
}

#ifdef HAVE_GETNAMEINFO
void
Network::Net::connect_tcp(const String& name, const String& port, int family) {
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
		int fd = socket(family, SOCK_STREAM, PF_INET);
		val = SOCKSBUF;
		setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val));
		val = SOCKSBUF;
		setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &val, sizeof(val));
		val = fcntl(fd, F_GETFL, 0);
		fcntl(fd, F_SETFL, val | O_NONBLOCK);
		res = connect(fd, info->ai_addr, info->ai_addrlen);
		if (res == 0) break;
	} while ((info = info->ai_next) != NULL);
	if (res != 0) {
		throw Error(String("connecting [") + name + "]:" +
		    port + " failed");
		freeaddrinfo(infosave);
	}
	peeraddr = "";
	peername = info->ai_canonname;
	peerport = port;
	freeaddrinfo(infosave);
}

#else

void
Network::Net::connect_tcp(const String& name, const String& port, int family) {
	if (family == AF_UNSPEC)
		family = AF_INET;
	if (family != AF_INET)
		throw Error("AF unsupported");
	struct hostent *hent = gethostbyname(name.c_str());
	if (hent == NULL)
		throw Error("gethostbyname failed");
	if (hent->h_addrtype != AF_INET)
		throw Error("gethostbyname returned different AF");
	struct servent *sent = getservbyname(port.c_str(), "tcp");
	if (sent == NULL)
		throw Error("getservbyname failed");
	a_ptr<struct sockaddr_in> sa;
	sa = new struct sockaddr_in;
	int res = -1;
	int i = 0;
	do {
		sa->sin_family = family;
		sa->sin_port = sent->s_port;
		bcopy (&hent->h_addr_list[i], &(sa->sin_addr),
		    hent->h_length);
		int fd = socket(family, SOCK_STREAM, PF_INET);
		int val;
		val = SOCKSBUF;
		setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val));
		val = SOCKSBUF;
		setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &val, sizeof(val));
		val = fcntl(fd, F_GETFL, 0);
		fcntl(fd, F_SETFL, val | O_NONBLOCK);
		res = connect(fd, (sockaddr*)sa.get(),
		    sizeof(struct sockaddr_in));
		if (res == 0) break;
		i++;
	} while (hent->h_addr_list[i] != NULL);
	if (res != 0) {
		throw Error(String("connecting [") + name + "]:" +
		    port + " failed");
	}
	peername = hent->h_name;
	peerport = port;
	peeraddr = "";
}
#endif

#ifdef HAVE_GETNAMEINFO
int
Network::Net::retrievepeername() {
	struct addrinfo *addrn, *addr0;
	struct addrinfo hints;
	int res;
	Matrix<char> port(NI_MAXHOST);
	Matrix<char> ip(NI_MAXHOST);
	Matrix<char> name(NI_MAXHOST);
	int ret;
	socklen_t addrlen;

	check();
	cassert(opened());

	addr0 = NULL;
	ret = 0;
	strcpy(port.get(), "unresolved");
	strcpy(ip.get(), "unresolved");
	strcpy(name.get(), "unresolved");

	Matrix<char> addrdt(MAXSOCKADDR);
	struct sockaddr *addr = (struct sockaddr*)addrdt.get();
	addrlen = MAXSOCKADDR;
	res = ::getsockname(fd, addr, &addrlen);
	if (res < 0)
		goto failed;

	if (addr->sa_family == AF_UNIX) {
		peerport = "";
		peeraddr = "";
		peername = sgethostname();
		return 0;
	}

	/* get the reversemapping and portname for the client */
	getnameinfo(addr, addrlen, name.get(), NI_MAXHOST, NULL, 0, 0);
	getnameinfo(addr, addrlen, ip.get(), NI_MAXHOST, port.get(),
	    NI_MAXHOST, NI_NUMERICHOST | NI_NUMERICSERV);

	/* now check the results with the forwardmapping */
	bzero(&hints, sizeof(hints));
	hints.ai_family = addr->sa_family;
	res = getaddrinfo(name.get(), NULL, &hints, &addr0);
	if (res) { /* we failed to forward map */
		strncpy (name.get(), ip.get(), NI_MAXHOST);
		return -1;
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
			if (res == 0) goto ok;
		} else if (addrn->ai_family == AF_INET6) {
			res = memcmp(
			    &((struct sockaddr_in6*)addrn->ai_addr)->sin6_addr,
			    &((struct sockaddr_in6*)addr)->sin6_addr,
			    sizeof(struct in6_addr));
			if (res == 0) goto ok;
		}
	}
	/* reversemapping isn't backed by forward mapping */
failed:	ret = -1;
ok:
	if (addr0 != NULL)
		freeaddrinfo(addr0);
	peername = name;
	peeraddr = ip;
	peerport = port;
	return ret;
}

#else

int
Network::Net::retrievepeername() {
	// TODO: implement
	//       It's not essential as we use SSL-Certs CN.
	peername = "";
	peeraddr = "";
	peerport = "";
	return 0;
}
#endif

void
Network::Net::waitread() {
	// TODO: use kevent & select
	struct pollfd pfd;

	pfd.fd = fd;
	pfd.revents = 0;
	pfd.events = POLLIN;
	poll(&pfd, 1, 1000);
	return;
}

void
Network::Net::waitwrite() {
	// TODO: use kevent & select
	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLOUT;
	pfd.revents = 0;
	poll(&pfd, 1, 1000);
	return;
}

String
Network::Net::tinfo() const {
	String ret;
	ret << "(" << typeid(*this).name() << "@" << this <<
	    ", fd=" << fd << ", peer=" << peername << ")";
	return ret;
}

int
Network::Listen::loop() {
	// TODO: use poll & kevent
	cassert (maxfd >= 0);
	for (;;) {
		fd_set fds;
		int res;
		do {
			memcpy(&fds, &lfds, sizeof(fd_set));
			res = select(maxfd + 1, &fds, NULL, NULL, NULL);
		} while (res < 0);
		for (int i = 0; i <= maxfd; i++) {
			if (FD_ISSET(i, &fds)) {
				beepme();
				int clientfd = accept(i, NULL, NULL);
				if (clientfd < 0 ) {
					if (errno == EINTR)
						continue;
				} else {
					beepme();
					try {
						beepme();
						// makestarter a virtual Listen function
						FTask *newthread = newtask();
						cassert(newthread != NULL);
						beepme();
						newthread->setfile(newcon(clientfd));
						newthread->start();
						beepme();
					} catch (std::exception& e) {
						beepme();
						syslog(LOG_INFO, "exception: %s", e.what());
					} catch (...) {
						beepme();
						syslog(LOG_INFO, "unknown exception");
					}
				}
			}
		}

	}
}

Network::Listen::Listen() {
	maxfd = -1;
	FD_ZERO(&lfds);
}

Network::Listen::~Listen() {
	for(int i = 0; i < maxfd; i++)
		if (FD_ISSET(i, &lfds))
			close(i);
}

void
Network::Listen::addfd(int nfd) {
	// XXX: check for overflow
	FD_SET(nfd, &lfds);
	if (nfd > maxfd)
		maxfd = nfd;
}

int
Network::Listen::add_UDS(const String& path, int flags) {
	struct sockaddr_un aun;
	int lfd;
	int val;

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
	listen(lfd, 128);
	val = fcntl(lfd, F_GETFL, 0);
	fcntl(lfd, F_SETFL, val | O_NONBLOCK);
	chmod(path.c_str(), flags);
	addfd(lfd);
	return 0;
}

#ifdef HAVE_GETNAMEINFO
int
Network::Listen::add_tcp(const String& name, const String& port, int family) {
	struct addrinfo *info;
	struct addrinfo *infosave;
	struct addrinfo hints;
	int res;
	int lfd;
	int val;
	const char *cname;
	const char *cport;

	bzero(&hints, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = family;
	cname = (name.length() == 0) ? NULL : name.c_str();
	cport = (port.length() == 0) ? NULL : port.c_str();
	res = getaddrinfo(cname, cport, &hints, &info);
	if (res) {
		syslog(LOG_EMERG, "getaddrinfo failed: %s",
		gai_strerror(res));
		cassert(0);
	}
	infosave = info;
	do {
		lfd = socket(info->ai_family, info->ai_socktype,
		info->ai_protocol);
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
		listen(lfd, 128);
		val = fcntl(lfd, F_GETFL, 0);
		fcntl(lfd, F_SETFL, val | O_NONBLOCK);
		addfd(lfd);
	} while ((info = info->ai_next) != NULL);
	freeaddrinfo(infosave);
	return 0;
}

#else

int
Network::Listen::add_tcp(const String& name, const String& port, int family) {
	if (family == AF_UNSPEC)
		family = AF_INET;
	if (family != AF_INET)
		throw Error("AF unsupported");
	struct hostent *hent = gethostbyname(name.c_str());
	if (hent == NULL)
		throw Error("gethostbyname failed");
	if (hent->h_addrtype != AF_INET)
		throw Error("gethostbyname returned different AF");
	struct servent *sent = getservbyname(port.c_str(), "tcp");
	if (sent == NULL)
		throw Error("getservbyname failed");
	a_ptr<struct sockaddr> sa;
	sa = (struct sockaddr*) new char[SOCK_MAXADDRLEN];
	int i = 0;
	do {
		sa->sin_family = family;
		sa->sin_port = sent->s_port;
		bcopy (&hent->h_addr_list[i], &(sa->sin_addr),
		    hent->h_length);
		int lfd = socket(family, SOCK_STREAM, PF_INET);
		cassert(lfd >= 0);
		cox(lfd);
		int val;
		val = 1;
		setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &val,
		sizeof(val));
		val = SOCKSBUF;
		setsockopt(lfd, SOL_SOCKET, SO_RCVBUF, &val,
		sizeof(val));
		val = SOCKSBUF;
		setsockopt(lfd, SOL_SOCKET, SO_SNDBUF, &val,
		sizeof(val));
		bind(lfd, sa.get(), SOCK_MAXADDRLEN);
		listen(lfd, 128);
		val = fcntl(lfd, F_GETFL, 0);
		fcntl(lfd, F_SETFL, val | O_NONBLOCK);
		addfd(lfd);
		i++;
	} while (hent->h_addr_list[i] != NULL);
	return 0;
}
#endif

void
Network::Listen::ncox(int fd) {
	cassert(fd >= 0);
	int val = fcntl(fd, F_GETFD, 0);
	cassert(val >= 0);
	int res = fcntl(fd, F_SETFD, val & ~FD_CLOEXEC);
	cassert(res >= 0);
}

void
Network::Listen::cox(int fd) {
	cassert(fd >= 0);
	int val = fcntl(fd, F_GETFD, 0);
	cassert(val >= 0);
	int res = fcntl(fd, F_SETFD, val | FD_CLOEXEC);
	cassert(res >= 0);
}

Network *
Network::Listen::newcon(int clientfd) {
	return new Network(clientfd);
}

