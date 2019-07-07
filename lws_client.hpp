/**
 * @file	lws_client.hpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/ )
 * @date	2019/07/05
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#pragma once

#if defined(HAS_LIBWEBSOCKETS) && HAS_LIBWEBSOCKETS

#include "client.hpp"
#include <libwebsockets.h>

#include <vector>
#include <deque>
#include <memory>
#include <mutex>

#include "frame_reader.hpp"

namespace stomp {

	class LibwebsocketsClient : public Client {
	public:
		static int get_send_buffer_pre_padding();
		static int get_send_buffer_post_padding();
		static int get_timer_period_us();


		class MessageVectorBuffer : public MessageBuffer {
		private:
			int data_size_;
			std::vector<char> buffer_;

		public:
			MessageVectorBuffer()
				: data_size_(0) {}
			MessageVectorBuffer(int size)
				: buffer_(get_send_buffer_pre_padding() + get_send_buffer_post_padding() + size, 0), data_size_(size) {}

			char* data_ptr() override {
				if (buffer_.size() <= get_send_buffer_pre_padding())
					return NULL;
				return &buffer_[0] + get_send_buffer_pre_padding();
			}
			int data_size() override {
				return data_size_;
			}
			void set_data_size(int data_size) {
				data_size_ = data_size;
			}

			std::vector<char>& writePrepare() {
				data_size_ = 0;
				buffer_.clear();
				buffer_.resize(get_send_buffer_pre_padding());
				return buffer_;
			}

			void writeDone() {
				int n = get_send_buffer_post_padding();
				data_size_ = buffer_.size() - get_send_buffer_pre_padding();
				while (n--)
					buffer_.push_back(0);
			}
		};

	private:
		bool use_lws_timer_;

		struct lws* wsi_;
		State state_;

		FrameReader frame_reader_;

		std::mutex send_queue_lock_;
		std::deque<std::unique_ptr<MessageBuffer> > send_queue_data_;

		LibwebsocketsClient(const LibwebsocketsClient& o) { assert(false); }

		void pushSendData(std::unique_ptr<MessageBuffer> & item);
		void sendConnectFrame();

		int onSocketWriteable(struct lws* wsi);
		int onSocketReceive(struct lws* wsi, const char *data, int len);
		int onSocketClosed();

		int onFrameConnected(Frame* frame);

	public:
		LibwebsocketsClient(bool use_lws_timer = true);
		virtual ~LibwebsocketsClient();

		int callbackProtocol(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len, bool *processed);

		void timerProc();

		State state() const override;
	};

}

#endif /* HAS_LIBWEBSOCKETS */
