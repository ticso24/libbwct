/*
 * Copyright (c) 2001,02 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#include <bwct/base.h>
#include <bwct/ssl.h>

void
CSSL::init() {
	SSL_load_error_strings();
	SSL_library_init();
}

CSSL::Network::Network(Context *sc) {
	(this)->sc = sc;
	ssl = NULL;
	x509 = NULL;
}

CSSL::Network::Network(int nfd, Context *sc) : ::Network(nfd) {
	(this)->sc = sc;
	ssl = NULL;
	x509 = NULL;
	if (sc != NULL)
		saccept();
}

CSSL::Network::~Network() {
	if (ssl != NULL)
		SSL_free(ssl);
	if (x509 != NULL)
		X509_free(x509);
}

ssize_t
CSSL::Network::microread(void *vptr, size_t n) {
	if (sc == NULL)
		return ::Network::microread(vptr, n);
	return SSL_read(ssl, vptr, n);
}

ssize_t
CSSL::Network::microwrite(const void *vptr, size_t n) {
	if (sc == NULL)
		return ::Network::microwrite(vptr, n);
	return SSL_write(ssl, vptr, n);
}

ssize_t
CSSL::Network::readv(SArray<struct iovec>& data) {
	if (sc == NULL)
		return ::Network::readv(data);
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
CSSL::Network::writev(SArray<struct iovec>& data) {
	if (sc == NULL)
		return ::Network::writev(data);
	ssize_t nwritten = 0;
	for (int i = 0; i <= data.max; i++) {
		ssize_t res = writen(data[i].iov_base, data[i].iov_len);
		nwritten += res;
		if (res != (ssize_t)data[i].iov_len)
			return nwritten;
	}
	return nwritten;
}

void
CSSL::Network::saccept() {
	beepme();
	cassert(sc != NULL);
	ssl = SSL_new(sc->sslContext);
	if (ssl == NULL)
		throw Error(String("SSL: Error allocating handle: ") +
		    ERR_error_string(ERR_get_error(), NULL));
	SSL_set_fd(ssl, fd);
	int res;
	while ((res = SSL_accept(ssl)) <= 0)
		if (SSL_get_error(ssl, res) == SSL_ERROR_WANT_READ)
			waitread();
		else if (SSL_get_error(ssl, res) == SSL_ERROR_WANT_WRITE)
			waitwrite();
		else
			throw Error(String("SSL: Error accepting on socket: ") +
			    ERR_error_string(ERR_get_error(), NULL));
	syslog(LOG_INFO, "SSL: negotiated cipher: %s", SSL_get_cipher(ssl));
	x509 = SSL_get_peer_certificate(ssl);
	syslog(LOG_INFO, "SSL: Certname %s", x509->name);
	Matrix<char> buf(256);
	if (X509_NAME_get_text_by_NID(X509_get_subject_name(x509),
	    NID_commonName, buf.get(), buf.size()) <= 0)
		throw Error(String("SSL: Error accepting on socket: ") +
		    ERR_error_string(ERR_get_error(), NULL));
#ifdef DEBUG
	if (peername != buf)
		syslog(LOG_INFO, "asumed peername\"%s\" != certname\"%s\"",
		    peername.c_str(), x509->name);
#endif
	peername = buf;
}

void
CSSL::Network::sconnect() {
	beepme();
	cassert(sc != NULL);
	ssl = SSL_new(sc->sslContext);
	if (ssl == NULL)
		throw Error(String("SSL: Error allocating handle: ") +
		    ERR_error_string(ERR_get_error(), NULL));
	SSL_set_fd(ssl, fd);
	int res;
	while ((res = SSL_connect(ssl)) <= 0)
		if (SSL_get_error(ssl, res) == SSL_ERROR_WANT_READ)
			waitread();
		else if (SSL_get_error(ssl, res) == SSL_ERROR_WANT_WRITE)
			waitwrite();
		else
			throw Error(String("SSL: Error accepting on socket: ") +
			    ERR_error_string(ERR_get_error(), NULL));
	syslog(LOG_INFO, "SSL: negotiated cipher: %s", SSL_get_cipher(ssl));
	x509 = SSL_get_peer_certificate(ssl);
	syslog(LOG_INFO, "SSL: Certname %s", x509->name);
	Matrix<char> buf(256);
	if (X509_NAME_get_text_by_NID(X509_get_subject_name(x509),
	    NID_commonName, buf.get(), buf.size()) <= 0)
		throw Error(String("SSL: Error accepting on socket: ") +
		    ERR_error_string(ERR_get_error(), NULL));
#ifdef DEBUG
	if (peername != buf)
		syslog(LOG_INFO, "asumed peername\"%s\" != certname\"%s\"",
		    peername.c_str(), x509->name);
#endif
	peername = buf;
}

void
CSSL::Network::connect_UDS(const String& path) {
	::Network::connect_UDS(path);
	if (sc != NULL)
		sconnect();
}

void
CSSL::Network::connect_tcp(const String& name, const String& port,
    int family) {
	::Network::connect_tcp(name, port, family);
	if (sc != NULL)
		sconnect();
}

::Network *
CSSL::Listen::newcon(int clientfd) {
	return new Network(clientfd);
}

CSSL::Context::Context(const String& keyfile, const String& certdir) {
	sslContext = SSL_CTX_new(SSLv23_method());
	if(sslContext == NULL)
		throw Error(String("SSL: Error allocating context: ") +
		    ERR_error_string(ERR_get_error(), NULL));
	try {
		SSL_CTX_set_options(sslContext, SSL_OP_ALL);
		if(!SSL_CTX_use_certificate_file(sslContext, keyfile.c_str(),
		    SSL_FILETYPE_PEM))
			throw Error(String("SSL: error "
			    "reading certificate from file ") + keyfile +
			    " " + ERR_error_string(ERR_get_error(), NULL));
		if(!SSL_CTX_use_PrivateKey_file(sslContext, keyfile.c_str(),
		    SSL_FILETYPE_PEM))
			throw Error(String("SSL: error "
			    "reading private key from file ") + keyfile +
			    " " + ERR_error_string(ERR_get_error(), NULL));
		if(!SSL_CTX_check_private_key(sslContext))
			throw Error("SSL: Private key"
			    "does not match public key in cert");
		SSL_CTX_set_tmp_rsa_callback(sslContext, ssl_temp_rsa_cb);
		SSL_CTX_set_verify(sslContext,
		    SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
		    ssl_verify_cb);
		SSL_CTX_load_verify_locations(sslContext,
		    NULL, certdir.c_str());
	} catch (...) {
		SSL_CTX_free(sslContext);
		throw;
	}
}

CSSL::Context::~Context() {
	SSL_CTX_free(sslContext);
}

int
CSSL::Context::ssl_verify_cb(int ok, X509_STORE_CTX *ctx) {
	Matrix<char> buffer(256);

	/* TODO: truncate to accepted CAs */
	X509_NAME_oneline(X509_get_issuer_name(ctx->current_cert),
	buffer.get(), buffer.size());
	if (ok)
		syslog(LOG_INFO, "SSL: Certificate OK: %s", buffer.get());
	else {
		switch (ctx->error){
		case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
			throw Error(String("SSL: Cert error: "
			    "CA unknown: ") + buffer);
		case X509_V_ERR_CERT_NOT_YET_VALID:
			throw Error(String("SSL: Cert error: "
			    "Cert not yet valid: ") + buffer);
		case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
			throw Error(String("SSL: Cert error: "
			    "illegal \'not before\' field: ") + buffer);
		case X509_V_ERR_CERT_HAS_EXPIRED:
			throw Error(String("SSL: Cert error: "
			    "Cert expired: ") + buffer);
		case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
			throw Error(String("SSL: Cert error: "
			    "invalid \'not after\' field: ") + buffer);
		case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
			throw Error(String("SSL: Cert error: "
			    "self signed cert in chain: ") + buffer);
		default:
			String Err;
			Err << "SSL: Cert error: unknown error " <<
			    ctx->error << " in " << buffer;
			throw Error(Err);
		}
	}
	return ok;
}

RSA *
CSSL::Context::ssl_temp_rsa_cb(SSL *ssl, int exp, int keylength) {
	// XXX?
	RSA *rsa = NULL;
	    
	if (rsa == NULL)
		rsa = RSA_generate_key(512, RSA_F4, NULL, NULL);
	return rsa;
}

