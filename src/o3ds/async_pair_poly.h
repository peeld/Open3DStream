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

#ifndef O3DS_ASYNC_PAIR_POLY
#define O3DS_ASYNC_PAIR_POLY

#include "nng/nng.h"
#include "nng/protocol/pair1/pair.h"

#include <mutex>
#include <string>

namespace O3DS
{
	//! Callback fired for each message received on a poly pair1 socket,
	//! tagged with the pipe it arrived on so the caller can tell which peer
	//! sent it. Fires on nng's own aio callback thread, not whatever thread
	//! called start().
	typedef void (*PolyInDataFunc)(void* context, nng_pipe pipe, const void* data, size_t len);

	//! Callback fired when a pipe is added to or removed from the socket
	//! (see start() - only NNG_PIPE_EV_ADD_POST / NNG_PIPE_EV_REM_POST are
	//! registered). Fires on nng's own notify thread.
	typedef void (*PolyPipeEventFunc)(void* context, nng_pipe pipe, nng_pipe_ev event);

	/*! \class AsyncPairPolyServer async_pair_poly.h o3ds/async_pair_poly.h */
	//! Wrapper for nng's pair1 "polyamorous" mode: one listening socket that
	//! accepts many simultaneous peers, each its own independent
	//! bidirectional pipe, individually addressable by sendTo(). This is the
	//! transport the Cloud Relay Service's control-plane backplane listener
	//! needs (see tcp_relay_service.md's "Backplane connection"): every relay
	//! instance dials the same URL, and plain pair1 (AsyncPairServer, see
	//! async_pair.h) only ever accepts one peer at a time.
	//!
	//! Deliberately not built on Connector/AsyncConnector (base_connector.h):
	//! that interface's InDataFunc has no room for a pipe id, which is the
	//! one thing every caller of this class actually needs, and bending it
	//! to fit would have meant changing a signature every other connector
	//! type relies on. Kept as its own small class instead, following the
	//! same pattern as the other protocol-specific wrappers in this
	//! directory (pipeline, publisher, subscriber, pair, request).
	class AsyncPairPolyServer
	{
	public:
		AsyncPairPolyServer();
		~AsyncPairPolyServer();

		//! Opens a poly pair1 socket and starts listening on url. Registers
		//! for NNG_PIPE_EV_ADD_POST / NNG_PIPE_EV_REM_POST so the pipe-event
		//! callback fires on connect/disconnect, and starts the async
		//! receive loop so the message callback starts firing for inbound
		//! messages. Returns false and fills getError() on failure.
		bool start(const char* url);

		//! Stops the receive loop and closes the socket. Safe to call more
		//! than once (including implicitly, from the destructor).
		void stop();

		//! Optional. Set before start() to avoid a race with the first
		//! inbound message, though nng won't complete the handshake with any
		//! peer until after nng_listen() returns from within start() anyway.
		void setMessageFunc(void* context, PolyInDataFunc f);

		//! Optional, same timing note as setMessageFunc.
		void setPipeEventFunc(void* context, PolyPipeEventFunc f);

		//! Sends data to one specific peer, addressed by the pipe id the
		//! message/pipe-event callbacks handed back. Thread-safe - safe to
		//! call from any thread, concurrently with the receive callback and
		//! with other calls to sendTo().
		bool sendTo(nng_pipe pipe, const char* data, size_t len);

		const std::string& getError() const { return mError; }

	private:
		static void onRecv(void* self);
		static void onPipeEvent(nng_pipe pipe, nng_pipe_ev ev, void* self);
		void handleRecv();
		void setError(const char* msg, int ret);

		nng_socket mSocket;
		nng_aio*   mAio;
		std::mutex mSendMutex;

		void* mMsgContext;
		PolyInDataFunc mMsgFunc;

		void* mPipeContext;
		PolyPipeEventFunc mPipeFunc;

		std::string mError;
		bool mStarted;
	};
}

#endif // O3DS_ASYNC_PAIR_POLY
