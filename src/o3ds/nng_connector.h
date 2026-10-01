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

#ifndef O3DS_NNG_CONNECTOR
#define O3DS_NNG_CONNECTOR

#include <string>
#include <vector>
#include <cstdint>
#include <mutex>
#include <atomic>
#include "base_connector.h"
#include "nng/nng.h"

#define NNG_ERROR(msg) if(ret != 0) { setError(msg, ret); return false;  }

namespace O3DS
{
	class BlockingNngConnector : public Connector
	{
	public:

		virtual ~BlockingNngConnector();

		// Base class for blocking connectors
		virtual bool   write(const char* data, size_t len) override;  //!< Writes an nng message
		virtual size_t read(char* data, size_t len) override;         //!< Read bytes - len is the size of data
		virtual size_t read(char** data, size_t* len) override;       //!< Read bytes, resize data if needed

		// Set the error with nng return code
		void setError(const char* msg, int ret);
	protected:
		nng_socket mSocket;
		std::mutex  mutex;
	};


	

	//! Bass class for async nng connectors
	/*! \class AsyncConnector base_server.h o3ds/base_server.h
	If the messages needs to be recieved, pass a callback to setFunc - this is optional  */
	class AsyncNngConnector : public AsyncConnector
	{
	public:
		AsyncNngConnector()
			: AsyncConnector()
			, mDialer()
			, aio(nullptr)
			, mSocket(NNG_SOCKET_INITIALIZER)
		{
			nng_ctx_open(&ctx, mSocket);
		};

		virtual ~AsyncNngConnector()
		{
			stop();
			if (aio) nng_aio_free(aio);
		}

		// Set the error with nng return code
		void setError(const char* msg, int ret);

		virtual bool start(const char* url) = 0;              //!< Starts the server - servers will listen, clients will dial
		bool         write(const char* data, size_t len) override;  //!< Writes an nng message
		size_t       read(char* data, size_t len) override;            //!< Read bytes - len is the size of data
		size_t       read(char** data, size_t* len) override;       //!< Read bytes, resize data if needed
		bool         asyncReadMsg();                          //!< handles a nng_recv_aio call.  Calls nng_recv_aio again if message was okay and returns true
		void         stop();

		//! Messages write() dropped because nng reported back-pressure (NNG_EAGAIN).
		uint64_t     droppedCount() const { return mDropped.load(std::memory_order_relaxed); }

		//! Configure TLS-PSK for a dialing (client) connector.  Must be called
		//! before start(), and only has an effect for tls+tcp:// urls.  The key
		//! is raw bytes (not hex).  Clears any previous setting if identity is empty.
		void         setTlsPsk(const std::string& identity, const std::vector<uint8_t>& key);
	protected:
		//! Builds a client mode nng_tls_config from setTlsPsk() and attaches it to
		//! the (created, not yet started) dialer.  Returns 0 if no PSK was set.
		int          applyTlsPsk(nng_dialer dialer);

		std::string          mPskIdentity;
		std::vector<uint8_t> mPskKey;

		nng_dialer mDialer;
		nng_aio* aio;

		nng_socket mSocket;
		std::mutex  mutex;
		nng_ctx  ctx;
		std::atomic<uint64_t> mDropped{ 0 };
	};
}

#endif  // O3DS_NNG_CONNECTOR
