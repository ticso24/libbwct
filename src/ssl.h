/*
 * Copyright (c) 2001,02,03,08,13 Bernd Walter Computer Technology
 * Copyright (c) 2008,13 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/ssl.h $
 * $Date: 2025-03-19 17:35:09 +0100 (Wed, 19 Mar 2025) $
 * $Author: as $
 * $Rev: 49020 $
 */

#ifndef _SSL
#define _SSL

#ifdef HAVE_OPENSSL

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
#ifdef WITH_SSL_KEYLOG
		static File keylog;
		static Mutex keylog_mtx;
#endif
		SSL_CTX *sslContext;
		AArray<SSL_CTX*> sslContexts;
		static int ssl_verify_cb(int ok, X509_STORE_CTX *ctx);
#if OPENSSL_VERSION_NUMBER < 0x30000000L
		static RSA *ssl_temp_rsa_cb(SSL *ssl, int exp, int keylength);
#endif
#ifdef  WITH_SSL_KEYLOG
		static void SSL_CTX_keylog_cb(const SSL *ssl, const char *line);
#endif
		static int SSLserverNameCallback_helper(SSL *ssl, int *ad, void *arg);
		static int alpn_select_proto_cb_helper(SSL *ssl, const unsigned char **out, unsigned char *outlen, const unsigned char *in, unsigned int inlen, void *arg);
		int alpn_select_proto_cb(SSL *ssl, const unsigned char **out, unsigned char *outlen, const unsigned char *in, unsigned int inlen);
		int SSLserverNameCallback(SSL *ssl, int *ad);
		Mutex mcerts;
		AArray<String> certs;
		AArray<String> certselection;
		Array<String> ALPN_options;
		SSL_CTX* initCTX(const String& keyfile, const String& certdir, const String& chainfile, Array<String> ALPN);

	public:
		Context(const String& keyfile, const String& certdir, const String& chainfile = "", Array<String> ALPN = Array<String>());
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
		String cipher;
		String tls_ver;
		String ALPN_selected;
		Context *sc;
		void saccept();
		Network(Context *sc = NULL);
		Network(int nfd, Context *sc = NULL);
		~Network();
		virtual void connect_UDS(const String& path);
		virtual void connect_tcp(const String& name, const String& port,
		    int family = AF_UNSPEC);
		virtual ssize_t sendfile(File &infile);
		virtual String tinfo() const;
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

#endif /* HAVE_OPENSSL */

#endif /* !_SSL */
