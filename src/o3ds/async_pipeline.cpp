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

#include "async_pipeline.h"

namespace O3DS
{
	bool AsyncPipelinePush::start(const char *url)
	{
		int ret;

		ret = nng_push0_open(&mSocket);
		if (ret != 0) { setError("Pipeline push open", ret); return false; }

		// Queue up to this many messages when the connection is momentarily not ready
		// for one. The default is an unbuffered socket, where the non-blocking write()
		// drops a message (NNG_EAGAIN) whenever the writer is busy with the previous
		// one - about 1-3% of a 120 msg/s stream. Best effort: if it is refused the
		// socket still works, unbuffered.
		nng_socket_set_int(mSocket, NNG_OPT_SENDBUF, 64);

		ret = nng_aio_alloc(&aio, AsyncPipeline::Callback, this);
		if (ret != 0) { setError("Pipeline push aio alloc", ret); return false; }

		// Create the dialer separately (rather than nng_dial) so a TLS-PSK
		// config can be attached before it starts.
		ret = nng_dialer_create(&mDialer, mSocket, url);
		if (ret != 0) { setError("Pipeline push dialer create", ret); return false; }

		ret = applyTlsPsk(mDialer);
		if (ret != 0) { setError("Pipeline push TLS-PSK config", ret); return false; }

		// Blocking start, same as nng_dial(..., 0): returns the result of the
		// first connection attempt (NNG_ECRYPTO on a failed TLS handshake).
		ret = nng_dialer_start(mDialer, 0);
		if (ret != 0)
		{
			setError("Pipeline push dial", ret);
			// A failed start leaves a socket, dialer and aio behind, and the caller
			// retries start() on a timer - release them so retries do not leak.
			nng_dialer_close(mDialer);
			mDialer = NNG_DIALER_INITIALIZER;
			nng_aio_free(aio);
			aio = nullptr;
			nng_close(mSocket);
			mSocket = NNG_SOCKET_INITIALIZER;
			return false;
		}

		nng_recv_aio(mSocket, aio);

		return true;
	}

	// Pipeline 
	bool AsyncPipelinePull::start(const char *url)
	{
		int ret;

		ret = nng_pull0_open(&mSocket);
		if (ret != 0) { return false; }

		ret = nng_aio_alloc(&aio, AsyncPipeline::Callback, this);
		if (ret != 0) { return false; }

		ret = nng_listen(mSocket, url, NULL, 0);
		if (ret != 0) return false;

		nng_recv_aio(mSocket, aio);

		return true;
	}

	void AsyncPipeline::Callback_()
	{
		int ret;

		ret = nng_aio_result(aio);
		if (ret != 0) return;

		char *buf = NULL;
		size_t sz;
		ret = nng_recv(mSocket, &buf, &sz, NNG_FLAG_ALLOC);
		if (ret != 0) return;

		if (mInDataFunc) mInDataFunc(mContext, (void*)buf, sz);

		nng_free(buf, sz);

		nng_recv_aio(mSocket, aio);
	}
}

