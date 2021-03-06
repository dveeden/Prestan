/* 
   HTTP Request Handling
   Copyright (C) 1999-2002, Joe Orton <joe@manyfish.co.uk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA

*/

/* THIS IS NOT A PUBLIC INTERFACE. You CANNOT include this header file
 * from an application.  */
 
#ifndef NE_PRIVATE_H
#define NE_PRIVATE_H

#ifdef NEON_SSL
#include <openssl/ssl.h>
#endif

#include "ne_request.h"
#include "ne_socket.h"

struct host_info {
    char *hostname;
    unsigned int port;
    ne_sock_addr *address; /* if non-NULL, result of resolving 'hostname'. */
    /* current network address obtained from 'address' being used. */
    const ne_inet_addr *current;
    char *hostport; /* URI hostport segment */
};

/* Store every registered callback in a generic container, and cast
 * the function pointer when calling it.  */
struct hook {
    void (*fn)(void);
    void *userdata;
    const char *id; /* non-NULL for accessors. */
    struct hook *next;
};

#define HAVE_HOOK(st,func) (st->hook->hooks->func != NULL)
#define HOOK_FUNC(st, func) (*st->hook->hooks->func)

/* Session support. */
struct ne_session_s {
    /* Connection information */
    ne_socket *socket;

    /* non-zero if connection has been established. */
    int connected;
    
    /* non-zero if connection has persisted beyond one request. */
    int persisted;

    int is_http11; /* >0 if connected server is known to be
		    * HTTP/1.1 compliant. */

    char *scheme;
    struct host_info server, proxy;

    /* Settings */
    unsigned int use_proxy:1; /* do we have a proxy server? */
    unsigned int no_persist:1; /* set to disable persistent connections */
    unsigned int use_ssl:1; /* whether a secure connection is required */
    unsigned int in_connect:1; /* doing a proxy CONNECT */

    int expect100_works; /* known state of 100-continue support */

    ne_progress progress_cb;
    void *progress_ud;

    ne_notify_status notify_cb;
    void *notify_ud;

    int rdtimeout; /* read timeout. */

    struct hook *create_req_hooks, *pre_send_hooks, *post_send_hooks;
    struct hook *destroy_req_hooks, *destroy_sess_hooks, *private;

    char *user_agent; /* full User-Agent string */

#ifdef NEON_SSL
    SSL_CTX *ssl_context;
    X509 *server_cert;
    SSL_SESSION *ssl_sess;
    /* client cert */
    EVP_PKEY *client_key;
    X509 *client_cert;
#endif

    /* Server cert verification callback: */
    ne_ssl_verify_fn ssl_verify_fn;
    void *ssl_verify_ud;
    /* Client cert provider callback: */
    ne_ssl_provide_fn ssl_provide_fn;
    void *ssl_provide_ud;
    /* Client cert key password private key provider callback: */
    ne_ssl_keypw_fn ssl_keypw_fn;
    void *ssl_keypw_ud;

    /* Error string */
    char error[BUFSIZ];
};

typedef int (*ne_push_fn)(void *userdata, const char *buf, size_t count);

/* Pulls the request body for the given request, passing blocks to the
 * given callback.
 */
int ne_pull_request_body(ne_request *req, ne_push_fn fn, void *ud);

/* Do the SSL negotiation. */
int ne_negotiate_ssl(ne_request *req);

#endif /* HTTP_PRIVATE_H */
