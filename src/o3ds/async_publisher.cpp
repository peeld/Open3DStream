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

#include "o3ds/async_publisher.h"

namespace O3DS
{
	bool AsyncPublisher::open()
	{
		int ret;

		ret = nng_pub0_open(&mSocket);
		NNG_ERROR("Creating publish socket");

		return true;
	}

	bool AsyncPublisher::start(const char *url)
	{
		if (!open())
		{
			return false;
		}

		if (!startListener(url, nullptr))
		{
			return false;
		}

		mState = Connector::STARTED;

		return true;
	}

	bool AsyncPublisher::startListener(const char* url, nng_tls_config* tlsConfig)
	{
		int ret;

		ret = nng_listener_create(&mListenerHandle, mSocket, url);
		NNG_ERROR("Could not create publish listener");

		if (tlsConfig != nullptr)
		{
			ret = nng_listener_set_ptr(mListenerHandle, NNG_OPT_TLS_CONFIG, tlsConfig);
			NNG_ERROR("Could not attach TLS config to publish listener");
		}

		ret = nng_listener_start(mListenerHandle, 0);
		NNG_ERROR("Could not start publish listener");

		mHasListener = true;
		mState = Connector::STARTED;

		return true;
	}

	void AsyncPublisher::closeListener()
	{
		if (mHasListener)
		{
			nng_listener_close(mListenerHandle);
			mListenerHandle = NNG_LISTENER_INITIALIZER;
			mHasListener = false;
		}
	}

	bool AsyncPublisher::notifyPipeEvent(nng_pipe_ev event, nng_pipe_cb callback, void* context)
	{
		int ret;

		ret = nng_pipe_notify(mSocket, event, callback, context);
		NNG_ERROR("Could not register pipe notify callback");

		return true;
	}
}
