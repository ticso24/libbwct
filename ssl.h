/*
 * Copyright (c) 2001,02 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#ifndef _SSL
#define _SSL

#include "network.h"

class CSSL {
public:
	static void init();
	class Network;
	class Context : public Base {
		friend class Network;
	private:
		SSL_CTX *sslContext;
		static int ssl_verify_cb(int ok, X509_STORE_CTX *ctx);
		static RSA *ssl_temp_rsa_cb(SSL *ssl,
		    int exp, int keylength);
	public:
		Context(const String& keyfile, const String& certdir);
		~Context();
	};
	class Network : public ::Network {
	private:
		::SSL *ssl;
		X509 *x509;
	protected:
		Context *sc;
		void saccept();
		void sconnect();
		virtual ssize_t microread(void *vptr, size_t n);
		virtual ssize_t microwrite(const void *vptr, size_t n);
	public:
		Network(Context *sc = NULL);
		Network(int nfd, Context *sc = NULL);
		~Network();
		virtual ssize_t readv(SArray<struct iovec>& data);
		virtual ssize_t writev(SArray<struct iovec>& data);
		virtual void connect_UDS(const String& path);
		virtual void connect_tcp(const String& name, const String& port,
		    int family = AF_UNSPEC);
	};

	class Listen : public ::Listen {
	private:
		virtual ::Network *newcon(int clientfd);
	};
};

#endif /* !_SSL */
