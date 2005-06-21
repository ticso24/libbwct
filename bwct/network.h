/*
 * Copyright (c) 2001,02,03 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#ifndef _NETWORK
#define _NETWORK

#include <bwct/fdhelper.h>

class Network {
private:
	const static int SOCKSBUF = 65536;
public:
	class Net : public File {
	protected:
		String peername;
		String peeraddr;
		String peerport;
		int timeout;
		virtual int retrievepeername();
	private:
		int canon;
	public:
		Net() {
			timeout = INFTIM;
		}
		Net(int nfd);
		~Net() {
		}
		ssize_t read(void *vptr, size_t n) {
			return File::read(vptr, n);
		}
		ssize_t write(const void *vptr, size_t n) {
			return File::write(vptr, n);
		}
		void settimeout(int nval) {
			timeout = nval;
		}
		virtual void connect_UDS(const String& path);
		virtual void connect_tcp(const String& name, const String& port,
		    int family = AF_UNSPEC);
		virtual void connect_tcp4(const String& name, const String& port) {
			connect_tcp(name, port, AF_INET);
		}
#ifdef HAVE_GETNAMEINFO
		virtual void connect_tcp6(const String& name, const String& port) {
			connect_tcp(name, port, AF_INET6);
		}
#endif
		const String& getpeername() {
			return peername;
		}
		virtual void waitread();
		virtual void waitwrite();
		virtual String tinfo() const;
		void nodelay(int flag);
	};

	class Listen : public Base {
	private:
		int maxfd;
		fd_set lfds;
		void addfd(int nfd);
		void ncox(int fd);
		void cox(int fd);
		virtual Net *newcon(int clientfd);
		virtual FTask *newtask() = 0;
	public:
		int loop();
		Listen();
		~Listen();
		int add_UDS(const String& path, int flags);
		int add_tcpv4(const String& name, const String& port) {
			return add_tcp(name, port, AF_INET);
		}
	#ifdef HAVE_GETNAMEINFO
		int add_tcpv6(const String& name, const String& port) {
			return add_tcp(name, port, AF_INET6);
		}
	#endif
		int add_tcp(const String& name, const String& port,
		    int family = AF_UNSPEC);
	};
};

#endif /* !_NETWORK */
