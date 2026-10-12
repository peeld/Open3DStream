/*
Open 3D Stream

Copyright 2022 Alastair Macleod

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef O3DS_ASYNC_PUBLISHER_H
#define O3DS_ASYNC_PUBLISHER_H

#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include "nng_connector.h"
#include <vector>

// Forward-declared rather than pulling in <nng/supplemental/tls/tls.h> here,
// so that callers who don't need TLS (the common case) aren't dragged into
// depending on it. Only startListener()'s caller needs the real definition,
// to build the nng_tls_config it passes in.
struct nng_tls_config;

namespace O3DS
{
	//! Wrapper for nng pub0 - Asynchronous publish connection
	/*! \class Starts a server and listens for connections, publishes data to clients. */
	class AsyncPublisher : public AsyncNngConnector
	{
	public:
		//! Opens the publish socket and starts a plain tcp:// listener on it in
		//! one step. Equivalent to open() followed by startListener(url, nullptr).
		virtual bool start(const char* url);

		//! Opens the publish socket without listening. Use this when the
		//! listener needs options (such as a TLS config) attached before it
		//! starts accepting - see startListener().
		bool open();

		//! Creates and starts a listener for this (already-open) socket,
		//! optionally attaching tlsConfig (server mode) beforehand. tlsConfig
		//! may be nullptr for a plain listener. Setting NNG_OPT_TLS_CONFIG
		//! places its own hold on tlsConfig, so the caller keeps ownership and
		//! may free its own reference once this call returns, whether it
		//! succeeded or not.
		bool startListener(const char* url, nng_tls_config* tlsConfig);

		//! Closes the current listener, if any. Per nng_listener_close's own
		//! documentation this also drops every pipe it accepted - not just
		//! future connections - so this is also how an already-connected
		//! subscriber gets disconnected. The socket itself stays open, ready
		//! for a later startListener() call. No-op if no listener is active.
		void closeListener();

		//! Registers a pipe lifecycle callback on this socket - see
		//! nng_pipe_notify - e.g. to track a live subscriber count for idle
		//! detection. Per nng_pipe_notify's own documentation, at most one
		//! callback can be registered per event; a second call for the same
		//! event replaces the first. Safe to call as soon as open() has
		//! succeeded, before any listener exists.
		bool notifyPipeEvent(nng_pipe_ev event, nng_pipe_cb callback, void* context);

	private:
		nng_listener mListenerHandle = NNG_LISTENER_INITIALIZER;
		bool mHasListener = false;
	};
} // namespace O3DS

#endif
