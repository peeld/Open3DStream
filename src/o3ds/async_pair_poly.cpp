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

#include "async_pair_poly.h"

namespace O3DS
{
	AsyncPairPolyServer::AsyncPairPolyServer()
		: mSocket(NNG_SOCKET_INITIALIZER)
		, mAio(nullptr)
		, mMsgContext(nullptr)
		, mMsgFunc(nullptr)
		, mPipeContext(nullptr)
		, mPipeFunc(nullptr)
		, mStarted(false)
	{
	}

	AsyncPairPolyServer::~AsyncPairPolyServer()
	{
		stop();
	}

	void AsyncPairPolyServer::setMessageFunc(void* context, PolyInDataFunc f)
	{
		mMsgContext = context;
		mMsgFunc = f;
	}

	void AsyncPairPolyServer::setPipeEventFunc(void* context, PolyPipeEventFunc f)
	{
		mPipeContext = context;
		mPipeFunc = f;
	}

	void AsyncPairPolyServer::setError(const char* msg, int ret)
	{
		mError = msg;
		mError += ": ";
		mError += nng_strerror(ret);
	}

	bool AsyncPairPolyServer::start(const char* url)
	{
		int ret;

		ret = nng_pair1_open_poly(&mSocket);
		if (ret != 0) { setError("Opening poly pair1 socket", ret); return false; }

		ret = nng_pipe_notify(mSocket, NNG_PIPE_EV_ADD_POST, &AsyncPairPolyServer::onPipeEvent, this);
		if (ret != 0) { setError("Registering pipe-add notify", ret); return false; }

		ret = nng_pipe_notify(mSocket, NNG_PIPE_EV_REM_POST, &AsyncPairPolyServer::onPipeEvent, this);
		if (ret != 0) { setError("Registering pipe-remove notify", ret); return false; }

		ret = nng_aio_alloc(&mAio, &AsyncPairPolyServer::onRecv, this);
		if (ret != 0) { setError("Allocating receive aio", ret); return false; }

		ret = nng_listen(mSocket, url, nullptr, 0);
		if (ret != 0) { setError("Listening", ret); return false; }

		mStarted = true;
		nng_recv_aio(mSocket, mAio);

		return true;
	}

	void AsyncPairPolyServer::stop()
	{
		if (!mStarted)
			return;

		// Order matters: flip mStarted first so handleRecv() (which may run
		// concurrently on nng's aio thread as nng_close() tears the socket
		// down) doesn't try to re-arm a receive on a socket that's going
		// away.
		mStarted = false;

		nng_close(mSocket);
		if (mAio)
		{
			nng_aio_stop(mAio);
			nng_aio_free(mAio);
			mAio = nullptr;
		}
		mSocket = NNG_SOCKET_INITIALIZER;
	}

	bool AsyncPairPolyServer::sendTo(nng_pipe pipe, const char* data, size_t len)
	{
		int ret;
		nng_msg* msg;

		std::lock_guard<std::mutex> guard(mSendMutex);

		ret = nng_msg_alloc(&msg, 0);
		if (ret != 0) { setError("Allocating outbound message", ret); return false; }

		ret = nng_msg_append(msg, data, len);
		if (ret != 0) { nng_msg_free(msg); setError("Appending outbound message", ret); return false; }

		// Tags the message with which pipe to deliver it to - this is what
		// makes a poly socket usable as many independent point-to-point
		// connections rather than a broadcast. If the pipe has since closed,
		// nng_sendmsg below fails (NNG_ECLOSED) rather than silently
		// delivering to whoever reconnects with the same instance identity
		// later.
		nng_msg_set_pipe(msg, pipe);

		ret = nng_sendmsg(mSocket, msg, 0);
		if (ret != 0) { nng_msg_free(msg); setError("Sending to pipe", ret); return false; }

		return true;
	}

	void AsyncPairPolyServer::onRecv(void* self)
	{
		static_cast<AsyncPairPolyServer*>(self)->handleRecv();
	}

	void AsyncPairPolyServer::handleRecv()
	{
		int ret = nng_aio_result(mAio);
		if (ret != 0)
		{
			// NNG_ECLOSED is expected on stop() tearing the socket down
			// while a receive was outstanding - not worth reporting as an
			// error, just don't re-arm.
			if (ret != NNG_ECLOSED)
			{
				setError("Async receive", ret);
			}
			return;
		}

		nng_msg* msg = nng_aio_get_msg(mAio);

		// Re-arm before running the callback, so one slow callback doesn't
		// stall messages arriving from other peers on this same socket.
		if (mStarted)
		{
			nng_recv_aio(mSocket, mAio);
		}

		if (msg == nullptr)
		{
			return;
		}

		nng_pipe pipe = nng_msg_get_pipe(msg);
		void* data = nng_msg_body(msg);
		size_t len = nng_msg_len(msg);

		if (data && len > 0 && mMsgFunc)
		{
			mMsgFunc(mMsgContext, pipe, data, len);
		}

		nng_msg_free(msg);
	}

	void AsyncPairPolyServer::onPipeEvent(nng_pipe pipe, nng_pipe_ev ev, void* self)
	{
		auto* server = static_cast<AsyncPairPolyServer*>(self);
		if (server->mPipeFunc)
		{
			server->mPipeFunc(server->mPipeContext, pipe, ev);
		}
	}
}
