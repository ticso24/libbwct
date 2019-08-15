/*
 * Copyright (c) 2001,02,03,08 Bernd Walter Computer Technology
 * Copyright (c) 2008 FIZON GmbH
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#ifndef _NETWORK
#define _NETWORK

#include "fdhelper.h"

extern SArray<int> fdescs_to_close;

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
		virtual void mywaitread();
		virtual void mywaitwrite();
	private:
		int canon;
	public:
		Net();
		Net(int nfd);
		~Net();
		ssize_t read(void *vptr, size_t n);
		virtual ssize_t write(const void *vptr, size_t n);
		virtual ssize_t write(const char *data);
		virtual ssize_t write(const String& data);
		void settimeout(int nval);
		virtual void connect_UDS(const String& path);
		virtual void connect_tcp(const Array<String>& names, const String& port,
		    int ms_timeout = 500, int family = AF_UNSPEC);
		virtual void connect_tcp(const String& name, const String& port,
		    int family = AF_UNSPEC);
		virtual void connect_tcp4(const String& name, const String& port);
		virtual void connect_tcp6(const String& name, const String& port);
		virtual String getpeername();
		virtual String getpeeraddr();
		virtual String tinfo() const;
		void nodelay(int flag);
		void nonblocking(bool flag);
	};

	class Listen : public Base {
	protected:
		SArray<int> lfds;
		void addfd(int nfd);
		void ncox(int fd);
		void cox(int fd);
		virtual Net *newcon(int clientfd);
		virtual FTask *newtask() = 0;
	public:
		void loop();
		Listen();
		~Listen();
		int add_UDS(const String& path, int flags, int queuelen);
		int add_tcpv4(const String& name, const String& port, int queuelen);
		int add_tcpv6(const String& name, const String& port, int queuelen);
		int add_tcp(const String& name, const String& port,
		    int queuelen, int family = AF_UNSPEC);
	};
};

#endif /* !_NETWORK */
