/*
 * Copyright (c) 2001,02,03,08,13 Bernd Walter Computer Technology
 * Copyright (c) 2008,13 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/ssl.cc $
 * $Date: 2020-05-05 15:19:12 +0200 (Tue, 05 May 2020) $
 * $Author: ticso $
 * $Rev: 42505 $
 */

#include "bwct.h"
#include <openssl/dh.h>
#include <openssl/ssl.h>

const size_t BUFSIZE = 256 * 1024;

static Mutex *locks;

static void
lock_callback(int mode, int type, const char *file, int line) {
	(void)file;
	(void)line;
	if ((mode & CRYPTO_LOCK) != 0)
		locks[type].lock();
	else
		locks[type].unlock();
}

static unsigned long
id_callback(void) {
	return ((unsigned long)pthread_self());
}

void
CSSL::init() {
	SSL_load_error_strings();
	SSL_library_init();

	RAND_load_file("/dev/random", 4096);

	abort_assert(RAND_status() != 0);

	int nlocks = CRYPTO_num_locks();
	locks = new Mutex[nlocks];
	CRYPTO_set_locking_callback(lock_callback);
	CRYPTO_set_id_callback(id_callback);
}

CSSL::Network::Network(Context *sc) {
	(this)->sc = sc;
	ssl = NULL;
	x509 = NULL;
}

CSSL::Network::Network(int nfd, Context *sc) : ::Network::Net(nfd) {
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
	if (sc == NULL) {
		return ::Network::Net::microread(vptr, n);
	}
	ERR_clear_error();
	return SSL_read(ssl, vptr, n);
}

ssize_t
CSSL::Network::microwrite(const void *vptr, size_t n) {
	if (sc == NULL) {
		return ::Network::Net::microwrite(vptr, n);
	}
	ERR_clear_error();
	return SSL_write(ssl, vptr, n);
}

ssize_t
CSSL::Network::readn(void *vptr, size_t n) {
	char *ptr = (char*)vptr;
	size_t nleft = n;
	while (nleft > 0) {
		if (rbufsize > 0) {
			// we have something in the readbuffer
			size_t copybytes = (rbufsize < nleft) ? rbufsize : nleft;
			bcopy(rbufpos, ptr, copybytes);
			ptr += copybytes;
			nleft -= copybytes;
			rbufsize -= copybytes;
			rbufpos += copybytes;
		}
		if (nleft > 0) {
			// request is still not satisfied
			ssize_t nread;
			if (nleft < sizeof(readbuf)) {
				// remaining request is smaller than buffersize, so try to fill the buffer in one go
				if ((nread = microread(readbuf, sizeof(readbuf))) < 0) {
					if (sc == NULL) {
						if (errno != EAGAIN && errno != EINTR) {
							return(nread);
						}
						waitread();
					} else {
						int e = SSL_get_error(ssl, nread);
						switch (e) {
						case SSL_ERROR_WANT_READ:
							waitread();
							break;
						case SSL_ERROR_WANT_WRITE:
							waitwrite();
							break;
						default:
							return(nread);
						}
					}
				} else {
					if (nread == 0) {
						goto exit;
					}
					rbufsize = nread;
					rbufpos = readbuf;
				}
			} else {
				// remaining request is bigger than buffer, so directly read into caller memory
				if ((nread = microread(ptr, nleft)) < 0) {
					if (sc == NULL) {
						if (errno != EAGAIN && errno != EINTR) {
							return(nread);
						}
						waitread();
					} else {
						int e = SSL_get_error(ssl, nread);
						switch (e) {
						case SSL_ERROR_WANT_READ:
							waitread();
							break;
						case SSL_ERROR_WANT_WRITE:
							waitwrite();
							break;
						default:
							return(nread);
						}
					}
				} else {
					if (nread == 0) {
						goto exit;
					}
					nleft -= nread;
					ptr += nread;
				}
			}
		}
	}
exit:
	return (n - nleft);
}

ssize_t
CSSL::Network::writen(const void *vptr, size_t n) {
	char *ptr = (char*)vptr;
	size_t nleft = n;
	while (nleft > 0) {
		ssize_t nwritten;
		if ((nwritten = microwrite(ptr, nleft)) < 0) {
			if (sc == NULL) {
				if (errno != EAGAIN && errno != EINTR)
					return(nwritten);
				waitwrite();
			} else {
				int e = SSL_get_error(ssl, nwritten);
				switch (e) {
				case SSL_ERROR_WANT_READ:
					waitread();
					break;
				case SSL_ERROR_WANT_WRITE:
					waitwrite();
					break;
				default:
					return(nwritten);
				}
			}
		} else {
			nleft -= nwritten;
			ptr += nwritten;
		}
	}
	return (n);
}

ssize_t
CSSL::Network::sendfile(File &infile) {
	if (sc == NULL) {
		return ::Network::Net::sendfile(infile);
	}
	a_ptr<char> buf;
	ssize_t bytessend;
	ssize_t bytesread;

	ssize_t bytesprocessed = 0;
	buf = new char[BUFSIZE];
	while ((bytesread = infile.read(buf.get(), BUFSIZE)) > 0) {
		bytessend = write(buf.get(), bytesread);
		if (bytessend != bytesread) {
			return -1;
		}
		bytesprocessed += bytessend;
	}
	return bytesprocessed;
}

// TODO: disable Nagle (TCP_NODELAY) on socket
void
CSSL::Network::saccept() {
	cassert(sc != NULL);
	ERR_clear_error();
	ssl = SSL_new(sc->sslContext);
	if (ssl == NULL) {
		throw Error(String("SSL: Error allocating handle: ") +
		    ERR_error_string(ERR_get_error(), NULL));
	}
	ERR_clear_error();
	SSL_set_fd(ssl, fd);
	int res;
	ERR_clear_error();
	while ((res = SSL_accept(ssl)) <= 0) {
		int e = SSL_get_error(ssl, res);
		if (res == 0) {
			throw Error(String("SSL: Error accepting on socket: ") +
			    ERR_error_string(e, NULL));
		}
		if (e == SSL_ERROR_WANT_READ) {
			waitread();
		} else if (e == SSL_ERROR_WANT_WRITE) {
			waitwrite();
		} else {
			throw Error(String("SSL: Error accepting on socket: ") +
			    ERR_error_string(ERR_get_error(), NULL));
		}
		ERR_clear_error();
	}
	syslog(LOG_INFO, "SSL: negotiated cipher: %s", SSL_get_cipher(ssl));
#if 0
	x509 = SSL_get_peer_certificate(ssl);
	syslog(LOG_INFO, "SSL: Certname %s", x509->name);
	a_ptr<char> buf;
	buf = new char[256];
	if (X509_NAME_get_text_by_NID(X509_get_subject_name(x509),
	    NID_commonName, buf.get(), 256) <= 0)
		throw Error(String("SSL: Error accepting on socket: ") +
		    ERR_error_string(ERR_get_error(), NULL));
#ifdef DEBUG_FIZON
	if (getpeername() != buf.get())
		syslog(LOG_INFO, "asumed peername\"%s\" != certname\"%s\"",
		    peername.c_str(), x509->name);
#endif
	peername = buf.get();
#endif
}

// TODO: disable Nagle (TCP_NODELAY) on socket
void
CSSL::Network::sconnect() {
	cassert(sc != NULL);
	ERR_clear_error();
	ssl = SSL_new(sc->sslContext);
	if (ssl == NULL)
		throw Error(String("SSL: Error allocating handle: ") +
		    ERR_error_string(ERR_get_error(), NULL));
	ERR_clear_error();
	SSL_set_fd(ssl, fd);
	int res;
	ERR_clear_error();
	while ((res = SSL_connect(ssl)) <= 0) {
		int e = SSL_get_error(ssl, res);
		if (e == SSL_ERROR_WANT_READ) {
			waitread();
		} else if (e == SSL_ERROR_WANT_WRITE) {
			waitwrite();
		} else {
			throw Error(String("SSL: Error connecting on socket: ") +
			    ERR_error_string(ERR_get_error(), NULL));
		}
		ERR_clear_error();
	}
	syslog(LOG_INFO, "SSL: negotiated cipher: %s", SSL_get_cipher(ssl));
	x509 = SSL_get_peer_certificate(ssl);
	a_ptr<char> buf;
	buf = new char[256];
	if (X509_NAME_get_text_by_NID(X509_get_subject_name(x509),
	    NID_commonName, buf.get(), 256) <= 0)
		throw Error(String("SSL: Error connecting on socket: ") +
		    ERR_error_string(ERR_get_error(), NULL));
	syslog(LOG_INFO, "SSL: Certname %s", buf.get());
#ifdef DEBUG_FIZON
	if (peername != buf.get())
		syslog(LOG_INFO, "asumed peername\"%s\" != certname\"%s\"",
		    peername.c_str(), buf.get());
#endif
	peername = buf.get();
}

void
CSSL::Network::connect_UDS(const String& path) {
	::Network::Net::connect_UDS(path);
	if (sc != NULL)
		sconnect();
}

void
CSSL::Network::connect_tcp(const String& name, const String& port,
    int family) {
	::Network::Net::connect_tcp(name, port, family);
	if (sc != NULL)
		sconnect();
}

::Network::Net *
CSSL::Listen::newcon(int clientfd) {
	return new Network(clientfd);
}

// generated by "openssl dhparam -C 2236"
static DH *get_dh2236()
{
	static unsigned char dh2236_p[] = {
		0x0A,0x5A,0x86,0x36,0x07,0x02,0x16,0xAA,0x97,0xB4,0xBB,0xF1,
		0x66,0xE3,0x7E,0xF7,0xD7,0xFD,0xB0,0xDE,0x9B,0x7F,0xB7,0xDE,
		0x98,0xA0,0xDC,0x30,0xAE,0x87,0x19,0x90,0xE1,0xFC,0x8D,0x10,
		0xF7,0xDD,0x4D,0x16,0xDD,0x23,0x2C,0x24,0xE5,0xDB,0x3D,0x06,
		0x6F,0x92,0xA9,0x1F,0x55,0x38,0x58,0x96,0x77,0x5F,0xB7,0xE0,
		0x25,0xC8,0x2A,0x9F,0xCF,0xC1,0x13,0x74,0x94,0x1F,0x34,0xF7,
		0x2D,0x4C,0x8B,0xA3,0x88,0x6A,0x2C,0xD2,0x4D,0x9C,0x36,0x75,
		0x56,0x7F,0x6C,0x97,0xC3,0xA3,0xA8,0xD7,0x46,0x20,0xE7,0x86,
		0x91,0xB9,0x4A,0xC7,0xE5,0x1C,0x72,0x8A,0x8B,0x4D,0x30,0xBE,
		0x22,0x18,0xCF,0x00,0x32,0x17,0x2E,0xFC,0x44,0x8E,0x50,0x3B,
		0x44,0x87,0xB2,0xCE,0xDA,0x7D,0x6F,0x86,0xE2,0x2E,0xC6,0x94,
		0xDD,0x2A,0xCB,0x30,0xA8,0x84,0xA4,0x4D,0xFE,0x64,0x59,0x95,
		0xE8,0x2E,0x61,0x27,0x8C,0x63,0x14,0xA3,0xF5,0xDF,0x0B,0xA6,
		0xE0,0xBB,0x48,0x6A,0x95,0x32,0xD6,0x04,0x0A,0x2E,0x2F,0x69,
		0x16,0xF6,0x63,0xBA,0xDA,0x82,0x20,0xC2,0xEC,0x76,0xD7,0xC5,
		0x36,0xB7,0x46,0xAA,0xAF,0x7F,0xF3,0xC6,0xAA,0x35,0xB0,0x9C,
		0x87,0x73,0x6F,0x36,0x29,0x33,0x69,0x78,0x69,0x39,0x81,0x4E,
		0x49,0x7E,0x14,0x5B,0xA1,0x7A,0xA4,0xE9,0x85,0xBF,0x9E,0x38,
		0x58,0x7A,0x4C,0xE1,0x5F,0xE8,0x32,0x45,0x8B,0x91,0xBA,0xF9,
		0x3F,0x5B,0xDB,0x40,0xF4,0xB8,0x93,0xF3,0xC9,0x55,0x9D,0xBD,
		0xF8,0x77,0xD0,0x6D,0x5D,0x10,0x11,0xC6,0xAB,0xE8,0x04,0x37,
		0x23,0x21,0x19,0xDE,0x5D,0x78,0x2F,0x0A,0xAA,0xA6,0x19,0x74,
		0xF0,0x6C,0xAF,0x57,0x9D,0x18,0x52,0xAD,0xF2,0xD6,0x6A,0x6B,
		0x80,0x3C,0x76,0x5B,
	};
	static unsigned char dh2236_g[] = {
		0x02,
	};
	DH *dh;

	dh = DH_new();
	if (dh == NULL) {
		return NULL ;
	}

#if OPENSSL_VERSION_NUMBER < 0x10100000L
	dh->p=BN_bin2bn(dh2236_p, sizeof(dh2236_p), NULL);
	dh->g=BN_bin2bn(dh2236_g, sizeof(dh2236_g), NULL);
	if ((dh->p == NULL) || (dh->g == NULL)) {
		return NULL;
	}
#else
	BIGNUM *p = BN_bin2bn(dh2236_p, sizeof(dh2236_p), NULL);
	BIGNUM *g = BN_bin2bn(dh2236_g, sizeof(dh2236_g), NULL);
	if (p == NULL || g == NULL)
	{
		BN_free(p);
		BN_free(g);
		DH_free(dh);
		return NULL;
	}

	// p, g are freed later by DH_free()
	if (0 == DH_set0_pqg(dh, p, NULL, g))
	{
		BN_free(p);
		BN_free(g);
		DH_free(dh);
		return NULL;
	}
#endif

	return(dh);
}

CSSL::Context::Context(const String& keyfile, const String& certdir, const String& chainfile) {
	sslContext = initCTX(keyfile, certdir, chainfile);
}

SSL_CTX* CSSL::Context::initCTX(const String& keyfile, const String& certdir, const String& chainfile) {
	SSL_CTX *newsslContext;

	// SSLv23_method handles SSLv2, SSLv3, TLSv1, TLSv1.1 and TLSv1.2, of
	// which we later disable some unwanted
	// other methods only support single protocols
	ERR_clear_error();
	newsslContext = SSL_CTX_new(SSLv23_method());
	if(newsslContext == NULL) {
		throw Error(String("SSL: Error allocating context: ") +
		    ERR_error_string(ERR_get_error(), NULL));
	}

	try {
		{
			long options = 0;
			// options |= SSL_OP_ALL;
			options |= SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION;
			options |= SSL_OP_NO_TICKET;
			options |= SSL_OP_NO_SSLv2;
			options |= SSL_OP_NO_SSLv3;
#ifdef SSL_OP_NO_COMPRESSION
			options |= SSL_OP_NO_COMPRESSION;
#endif
			SSL_CTX_set_options(newsslContext, options);
		}
		if (chainfile == "") {
			ERR_clear_error();
			if(!SSL_CTX_use_certificate_file(newsslContext, keyfile.c_str(),
			    SSL_FILETYPE_PEM))
				throw Error(String("SSL: error "
				    "reading certificate from file ") + keyfile +
				    " " + ERR_error_string(ERR_get_error(), NULL));
		}
		ERR_clear_error();
		if(!SSL_CTX_use_PrivateKey_file(newsslContext, keyfile.c_str(),
		    SSL_FILETYPE_PEM))
			throw Error(String("SSL: error "
			    "reading private key from file ") + keyfile +
			    " " + ERR_error_string(ERR_get_error(), NULL));
		if (chainfile != "") {
			ERR_clear_error();
			if(!SSL_CTX_use_certificate_chain_file(newsslContext, chainfile.c_str()))
				throw Error(String("SSL: error "
				    "reading private key from file ") + keyfile +
				    " " + ERR_error_string(ERR_get_error(), NULL));
		}
		ERR_clear_error();
		if(!SSL_CTX_check_private_key(newsslContext))
			throw Error(String() + "SSL: Private key "
			    "does not match public key in cert " + ERR_error_string(ERR_get_error(), NULL));

		{
			// setup diffie-hellman parameters
			DH *dh = get_dh2236();
			if (SSL_CTX_set_tmp_dh(newsslContext, dh) != 1) {
				throw Error(S + "SSL: SSL_CTX_set_tmp_dh failed");
			}
			DH_free(dh);
		}

		{
			// setup elliptic curve diffie-hellman
			EC_KEY *ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
			if (ecdh == NULL) {
				throw Error(S + "EC_KEY_new_by_curve_name failed");
			}
			if (SSL_CTX_set_tmp_ecdh(newsslContext, ecdh) != 1) {
				throw Error(S + "SSL_CTX_set_tmp_ecdh failed");
			}
			EC_KEY_free (ecdh);
		}

		SSL_CTX_set_tlsext_servername_callback(newsslContext, SSLserverNameCallback_helper);
		SSL_CTX_set_tlsext_servername_arg(newsslContext, this);

		SSL_CTX_set_tmp_rsa_callback(newsslContext, ssl_temp_rsa_cb);
//		SSL_CTX_set_verify(newsslContext,
//		    SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
//		    ssl_verify_cb);
//		SSL_CTX_set_verify(newsslContext, SSL_VERIFY_PEER, ssl_verify_cb);
		SSL_CTX_set_verify(newsslContext, SSL_VERIFY_NONE, NULL);
		SSL_CTX_load_verify_locations(newsslContext,
		    NULL, certdir.c_str());
//		String ID_Context("FRT SSL Context");
//		SSL_CTX_set_session_id_context(newsslContext, (const uint8_t*)ID_Context.c_str(), ID_Context.length());
		SSL_CTX_set_session_cache_mode(newsslContext, SSL_SESS_CACHE_OFF);

		// setup ciphers
		// openssl ciphers "ciphers=AECDH:ECDH:ALL:\!aNULL:\!eNULL:\!LOW:\!EXP:\!RC2:\!RC4:\!DES:\!EDH"
		//
		// EDH disables DHE- ciphers.
		// We need to exclude them completely as long as OpenSSL has the 512-bit downgrade unfixed
		//
		SSL_CTX_set_cipher_list(newsslContext,
		    "AECDH:ECDH:ALL:!aNULL:!eNULL:!LOW:!EXP:!RC2:!RC4:!DES:!EDH");

	} catch (...) {
		SSL_CTX_free(newsslContext);
		newsslContext = NULL;
		throw;
	}
	return newsslContext;
}

CSSL::Context::~Context() {
	if (sslContext != NULL) {
		SSL_CTX_free(sslContext);
	}
}

void 
CSSL::Context::SetCerts(const AArray<String>& ncerts, const AArray<String>& ncertselection)
{
	Mutex::Guard mtx(mcerts);
	certs = ncerts;
	certselection = ncertselection;
	// cleanup loaded certfiles
	Array<String> certkeys = sslContexts.getkeys();
	for (int64_t i = 0; i <= certkeys.max; i++) {
		SSL_CTX_free(sslContexts[certkeys[i]]);
	}
	sslContexts.empty();
}

int
CSSL::Context::SSLserverNameCallback_helper(SSL *ssl, int *ad, void *arg)
{
	CSSL::Context* me = (CSSL::Context*)arg;
	return me->SSLserverNameCallback(ssl, ad);
}

int
CSSL::Context::SSLserverNameCallback(SSL *ssl, int *ad)
{
	if (ssl == NULL) {
		return SSL_TLSEXT_ERR_NOACK;
	}

	String servername = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
	syslog(LOG_INFO, "SSL: negotiated servername: %s", servername.c_str());

	Mutex::Guard mtx(mcerts);
	if (certselection.exists("default")) {
		// we have more than a single standard cert
		String certname;
		if (certselection.exists(servername)) {
			certname = certselection[servername];
		} else {
			certname = certselection["default"];
		}
		String certfile = certs[certname];
		if (!sslContexts.exists(certfile)) {
			// setup a new sslContext
			SSL_CTX* newsslContext = initCTX(certfile, certfile, certfile);
			if (newsslContext == NULL) {
				syslog(LOG_INFO, "SSL: failed to load certfile: %s", certfile.c_str());
				// just use the default cert
				return SSL_TLSEXT_ERR_OK;
			}
			sslContexts[certfile] = newsslContext;
		}
		SSL_CTX* ctx = sslContexts[certfile];
		cassert(ctx != NULL);
		if (ctx == NULL)
			return SSL_TLSEXT_ERR_NOACK;

		/* Useless return value */
		SSL_CTX* v = SSL_set_SSL_CTX(ssl, ctx);
		cassert(v == ctx);
		if (v != ctx) {
			return SSL_TLSEXT_ERR_NOACK;
		}
	}

	return SSL_TLSEXT_ERR_OK; // uses default cert
}

int
CSSL::Context::ssl_verify_cb(int ok, X509_STORE_CTX *ctx) {
	a_ptr<char> buffer;
	buffer = new char[256];

	X509 *err_cert;
	int err, depth;
	SSL *ssl;

	err_cert = X509_STORE_CTX_get_current_cert(ctx);
	err = X509_STORE_CTX_get_error(ctx);
	depth = X509_STORE_CTX_get_error_depth(ctx);

	/*
	* Retrieve the pointer to the SSL of the connection currently treated
	* and the application specific data stored into the SSL object.
	*/
	ssl = (SSL*) X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());

	X509_NAME_oneline(X509_get_subject_name(err_cert), buffer.get(), 256);

#if 0
	/*
	* Catch a too long certificate chain. The depth limit set using
	* SSL_CTX_set_verify_depth() is by purpose set to "limit+1" so
	* that whenever the "depth>verify_depth" condition is met, we
	* have violated the limit and want to log this error condition.
	* We must do it here, because the CHAIN_TOO_LONG error would not
	* be found explicitly; only errors introduced by cutting off the
	* additional certificates would be logged.
	*/
	if (depth > mydata->verify_depth) {
		ok = 0;
		err = X509_V_ERR_CERT_CHAIN_TOO_LONG;
		X509_STORE_CTX_set_error(ctx, err);
	}
#endif
	if (!ok) {
		syslog(LOG_DEBUG, "verify error:num=%d:%s:depth=%d:%s\n", err,
		X509_verify_cert_error_string(err), depth, buffer.get());
	}

	/*
	* At this point, err contains the last verification error. We can use
	* it for something special
	*/
	if (!ok && (err == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT)) {
		X509_NAME_oneline(X509_get_issuer_name(err_cert), buffer.get(), 256);
		syslog(LOG_DEBUG, "issuer= %s\n", buffer.get());
	}

	return ok;
}

RSA *
CSSL::Context::ssl_temp_rsa_cb(SSL *ssl, int exp, int keylength) {
	// XXX?
	RSA *rsa = NULL;

	if (rsa == NULL) {
		BIGNUM *e;
		e = BN_new();
		BN_set_word(e, RSA_F4);

		RSA_generate_key_ex(rsa, 512, e, NULL);

		BN_free(e);
		e = NULL;
	}
	return rsa;
}

void
CSSL::PKCS7::cleanup() {
	if (ssl != NULL)
		SSL_free(ssl);
	cassert(certs == NULL);
	delete sc;
}

CSSL::PKCS7::PKCS7(Context *sc) {
	ssl = NULL;
	certs = NULL;
	cassert (sc != NULL);
	(this)->sc = sc;
	try {
		ERR_clear_error();
		ssl = SSL_new(sc->sslContext);
		if (ssl == NULL)
			throw Error(String("SSL: Error allocating handle: ") +
			    ERR_error_string(ERR_get_error(), NULL));
		x509 = SSL_get_certificate(ssl);
		if (x509 == NULL)
			throw Error("failed to get an X509 structure");
		store = SSL_CTX_get_cert_store(sc->sslContext);
		if (store == NULL)
			throw Error("failed to get an X509_STORE structure");
	} catch (...) {
		cleanup();
		throw;
	}
}

CSSL::PKCS7::~PKCS7() {
	cleanup();
}

ssize_t
CSSL::PKCS7::verify(char *in, ssize_t insize, char* out, ssize_t outsize) {
	cassert(x509 != NULL);
	BIO *bin = NULL;
	BIO *bout = NULL;
	try {
		bin = BIO_new(BIO_s_mem());
		if (bin == NULL)
			throw Error("failed to allocate BIO");
		bout = BIO_new(BIO_s_mem());
		if (bout == NULL)
			throw Error("failed to allocate BIO");
		BIO_write(bin, in, insize);
		::PKCS7 *pkcs7;
		(void)BIO_reset(bout);
		pkcs7 = SMIME_read_PKCS7(bin, NULL);
		if (pkcs7 == NULL)
			throw Error("failed to read signed message");
		int res;
		res = PKCS7_verify(pkcs7, certs, store, bin, bout, 0);
		if (res != 0)
			throw Error("failed to verify signed message");
		size_t ret = BIO_read(bout, out, outsize);
		BIO_free_all(bin);
		BIO_free_all(bout);
		return ret;
	} catch (...) {
		if (bin != NULL)
			BIO_free_all(bin);
		if (bout != NULL)
			BIO_free_all(bout);
		throw;
	}
}

ssize_t
CSSL::PKCS7::sign(char *in, ssize_t insize, char* out, ssize_t outsize) {
	cassert(x509 != NULL);
	BIO *bin = NULL;
	BIO *bout = NULL;
	try {
		bin = BIO_new(BIO_s_mem());
		if (bin == NULL)
			throw Error("failed to allocate BIO");
		bout = BIO_new(BIO_s_mem());
		if (bout == NULL)
			throw Error("failed to allocate BIO");
		BIO_write(bin, in, insize);
		::PKCS7 *pkcs7;
		pkcs7 = PKCS7_sign(x509, NULL, NULL, bin, 0);
		if (pkcs7 == NULL)
			throw Error("failed to sign message");
		(void)BIO_reset(bout);
		if (SMIME_write_PKCS7(bout, pkcs7, bin, 0) != 0)
			throw Error("failed to sign message");
		size_t ret = BIO_read(bout, out, outsize);
		BIO_free_all(bin);
		BIO_free_all(bout);
		return ret;
	} catch (...) {
		if (bin != NULL)
			BIO_free_all(bin);
		if (bout != NULL)
			BIO_free_all(bout);
		throw;
	}
}

