/*
 * Copyright (c) 2001,02,03 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#ifndef _SSL
#define _SSL

#include <bwct/network.h>

class CSSL {
public:
	static void init();
	class Network;
	class PKCS7;

	class Context : public Base {
		friend class Network;
		friend class PKCS7;
	private:
		SSL_CTX *sslContext;
		static int ssl_verify_cb(int ok, X509_STORE_CTX *ctx);
		static RSA *ssl_temp_rsa_cb(SSL *ssl,
		    int exp, int keylength);
	public:
		Context(const String& keyfile, const String& certdir);
		~Context();
	};

	class Network : public ::Network::Net {
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

	class Listen : public ::Network::Listen {
	private:
		virtual ::Network::Net *newcon(int clientfd);
	};

	class PKCS7 : public Base {
	private:
		X509 *x509;
		STACK_OF(X509) *certs;
		X509_STORE *store;
		SSL *ssl;
		Context *sc;
		void cleanup();
	public:
		PKCS7(Context *sc);
		~PKCS7();
		ssize_t check(
		    char *in, ssize_t insize, char* out, ssize_t outsize);
		ssize_t sign(
		    char *in, ssize_t insize, char* out, ssize_t outsize);
	};
};

#endif /* !_SSL */
