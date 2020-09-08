#ifndef O3DS_CLIENT_H
#define O3DS_CLIENT_H

#include <nng/nng.h>
#include <nng/protocol/pubsub0/sub.h>
#include <nng/supplemental/util/platform.h>
#include <string>

namespace O3DS
{
	// The client pulls data down from a listen server
	class AsyncSubscriber
	{
	public:
		AsyncSubscriber();
		virtual ~AsyncSubscriber() { nng_close(mSocket); }
		bool connect(const char*url);

		static void Callback(void *ref) { ((AsyncSubscriber*)ref)->Callback_(); }
		void Callback_();
		static void PipeEvent(nng_pipe pipe, nng_pipe_ev pipe_ev, void* ref)
		{
			((AsyncSubscriber*)ref)->PipeEvent_(pipe, pipe_ev);
		}
		void PipeEvent_(nng_pipe pipe, nng_pipe_ev pipe_ev);

		virtual void in_pipe() = 0;

		virtual void in_data(const char *msg, size_t len) = 0; 

		nng_socket mSocket;
		nng_dialer mDialer;
		nng_aio *aio;

		std::string mError;

	};


}


#endif