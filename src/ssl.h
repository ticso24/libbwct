/*
 * Copyright (c) 2001,02,03,08,13 Bernd Walter Computer Technology
 * Copyright (c) 2008,13 FIZON GmbH
 * All rights reserved.
 *
 * $URL: http://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/ssl.h $
 * $Date: 2016-10-14 02:33:59 +0200 (Fri, 14 Oct 2016) $
 * $Author: ticso $
 * $Rev: 30060 $
 */

#ifndef _SSL
#define _SSL

#include "network.h"
#include "aarray.h"

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
		AArray<SSL_CTX*> sslContexts;
		static int ssl_verify_cb(int ok, X509_STORE_CTX *ctx);
		static RSA *ssl_temp_rsa_cb(SSL *ssl,
		    int exp, int keylength);
		static int SSLserverNameCallback_helper(SSL *ssl, int *ad, void *arg);
		int SSLserverNameCallback(SSL *ssl, int *ad);
		Mutex mcerts;
		AArray<String> certs;
		AArray<String> certselection;
		SSL_CTX* initCTX(const String& keyfile, const String& certdir, const String& chainfile);

	public:
		Context(const String& keyfile, const String& certdir, const String& chainfile = "");
		void SetCerts(const AArray<String>& ncerts, const AArray<String>& ncertselection);
		~Context();
	};

	class Network : public ::Network::Net {
	private:
		::SSL *ssl;
		X509 *x509;
	protected:
		void sconnect();
		virtual ssize_t microread(void *vptr, size_t n);
		virtual ssize_t microwrite(const void *vptr, size_t n);
		virtual ssize_t readn(void *vptr, size_t n);
		virtual ssize_t writen(const void *vptr, size_t n);
	public:
		Context *sc;
		void saccept();
		Network(Context *sc = NULL);
		Network(int nfd, Context *sc = NULL);
		~Network();
		virtual void connect_UDS(const String& path);
		virtual void connect_tcp(const String& name, const String& port,
		    int family = AF_UNSPEC);
		virtual ssize_t sendfile(File &infile);
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
		ssize_t verify(
		    char *in, ssize_t insize, char* out, ssize_t outsize);
		ssize_t sign(
		    char *in, ssize_t insize, char* out, ssize_t outsize);
	};
};

#endif /* !_SSL */
