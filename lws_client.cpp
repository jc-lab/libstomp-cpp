/**
 * @file	lws_client.cpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/ )
 * @date	2019/07/05
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#include "lws_client.hpp"

#include <chrono>

#if defined(HAS_LIBWEBSOCKETS) && HAS_LIBWEBSOCKETS

#include "frame.hpp"

#include "command/connect.hpp"

#if !defined(_MSC_VER) || !_MSC_VER
#define strtok_s strtok_r
#endif

namespace stomp {

	LibwebsocketsClient::LibwebsocketsClient(bool use_lws_timer)
		: Client(),
		use_lws_timer_(use_lws_timer),
		wsi_(NULL),
		state_(State::DISCONNECTED),
		id_tx_count_(0),
		id_sub_count_(0),
		heartbeat_cx_(10000),
		heartbeat_cy_(10000),
		heartbeat_sx_(0),
		heartbeat_sy_(0),
		heartbeat_send_interval_(0)
	{
	}

	LibwebsocketsClient::~LibwebsocketsClient()
	{
	}

	int LibwebsocketsClient::callbackProtocol(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len, bool* processed)
	{
		int rc = 0;

		switch (reason) {
		case LWS_CALLBACK_WSI_CREATE:
			wsi_ = wsi;
			break;

		case LWS_CALLBACK_WSI_DESTROY:
			wsi_ = NULL;
			break;

		case LWS_CALLBACK_CLIENT_ESTABLISHED:
			if (use_lws_timer_)
				lws_set_timer_usecs(wsi, 100000);
			sendConnectFrame();
			*processed = true;
			break;

		case LWS_CALLBACK_CLIENT_CLOSED:
			rc = onSocketClosed();
			*processed = true;
			break;

		case LWS_CALLBACK_CLIENT_WRITEABLE:
			onSocketWriteable(wsi);
			*processed = true;
			break;

		case LWS_CALLBACK_CLIENT_RECEIVE:
			onSocketReceive(wsi, (const char*)in, len);
			*processed = true;
			break;

		case LWS_CALLBACK_CLIENT_RECEIVE_PONG:
			lwsl_hexdump(in, len);
			break;

		case LWS_CALLBACK_TIMER:
			if (use_lws_timer_)
			{
				timerProc();
				lws_set_timer_usecs(wsi, 100000);
			}
			break;

		default:
			break;
		}

		return 0;
	}

	void LibwebsocketsClient::timerProc()
	{
		bool sendable;

		if (heartbeat_send_interval_ > 0 && state_ == State::CONNECTED) {
			std::chrono::steady_clock::time_point cur = std::chrono::steady_clock::now();
			std::chrono::steady_clock::duration diff = cur - heartbeat_prev_ticks_;

			if (diff >= std::chrono::milliseconds(heartbeat_send_interval_))
			{
				// Send heartbeat
				std::unique_ptr<MessageVectorBuffer> item(new MessageVectorBuffer());
				std::unique_ptr<MessageBuffer> temp;
				item->writePrepare().push_back('\n');
				item->writeDone();
				temp = std::move(item);
				pushSendData(temp);
				heartbeat_prev_ticks_ = cur;
			}
		}

		send_queue_lock_.lock();
		sendable = send_queue_data_.empty();
		send_queue_lock_.unlock();
		if (!sendable)
			lws_callback_on_writable(wsi_);
	}

	void LibwebsocketsClient::pushSendData(std::unique_ptr<MessageBuffer>& item)
	{
		std::unique_lock<std::mutex> lock(send_queue_lock_);
		send_queue_data_.emplace_back(std::move(item));
		lws_callback_on_writable(wsi_);
	}

	void LibwebsocketsClient::sendConnectFrame()
	{
		std::unique_ptr<MessageVectorBuffer> item(new MessageVectorBuffer());
		std::unique_ptr<MessageBuffer> temp;
		command::Connect connect(this);
		connect.frame()->make_payload_append(item->writePrepare());
		item->writeDone();
		temp = std::move(item);
		pushSendData(temp);
	}

	int LibwebsocketsClient::onSocketWriteable(struct lws* wsi)
	{
		std::unique_lock<std::mutex> lock(send_queue_lock_);
		if (!send_queue_data_.empty()) {
			std::unique_ptr<MessageBuffer> buf(std::move(send_queue_data_.front()));
			send_queue_data_.pop_front();
			lock.unlock();
			return lws_write(wsi_, (unsigned char*)buf->data_ptr(), buf->data_size(), LWS_WRITE_BINARY);
		}
		return 0;
	}

	int LibwebsocketsClient::onSocketReceive(struct lws* wsi, const char* data, int len)
	{
		std::list< std::unique_ptr<Frame> > frames;
		int rc = frame_reader_.decode(data, len, frames);
		for (std::list< std::unique_ptr<Frame> >::iterator iter = frames.begin(); iter != frames.end(); iter++) {
			const std::string& command = (*iter)->command();
			if (command == Frame::Commands::CONNECTED)
				rc = onFrameConnected(iter->get());
			else if (command == Frame::Commands::MESSAGE)
				rc = onMessage(iter->get());
		}
		return rc;
	}

	int LibwebsocketsClient::onSocketClosed()
	{
		return onClosed();
	}

	int LibwebsocketsClient::onFrameConnected(Frame* frame)
	{
		if(frame->has_header(Frame::Headers::HEART_BEAT)) {
			std::string heart_beat_raw = frame->header(Frame::Headers::HEART_BEAT);
			char *sx, *sy = NULL;
			if (!heart_beat_raw.empty()) {
				sx = strtok_s(&heart_beat_raw[0], ",", &sy);
				if (sx && sy) {
					heartbeat_sx_ = atoi(sx);
					heartbeat_sy_ = atoi(sy);
				}
			}
		}
		heartbeat_send_interval_ = (heartbeat_cx_ > heartbeat_sy_) ? heartbeat_cx_ : heartbeat_sy_;

		heartbeat_prev_ticks_ = std::chrono::steady_clock::now();
		state_ = State::CONNECTED;

		return onConnected(frame);
	}

	int LibwebsocketsClient::sendFrame(Frame* frame)
	{
		std::unique_ptr<MessageVectorBuffer> buffer(new MessageVectorBuffer());
		frame->make_payload_append(buffer->writePrepare());
		buffer->writeDone();
		send_queue_lock_.lock();
		send_queue_data_.push_back(std::move(buffer));
		send_queue_lock_.unlock();
		return 0;
	}

	int LibwebsocketsClient::sendCommand(command::Base* item)
	{
		return sendFrame(item->frame());
	}

	std::string LibwebsocketsClient::generateSubscribeId()
	{
		std::unique_lock<std::mutex> lock{ id_lock_ };
		char buf[128];
		snprintf(buf, sizeof(buf), "sub-%llx", ++id_sub_count_);
		return buf;
	}

	std::string LibwebsocketsClient::generateTransactionId()
	{
		std::unique_lock<std::mutex> lock{ id_lock_ };
		char buf[128];
		snprintf(buf, sizeof(buf), "tx-%llx", ++id_tx_count_);
		return buf;
	}

	Client::State LibwebsocketsClient::state() const {
		return state_;
	}

	int LibwebsocketsClient::get_send_buffer_pre_padding() {
		return LWS_SEND_BUFFER_PRE_PADDING;
	}
	
	int LibwebsocketsClient::get_send_buffer_post_padding() {
		return LWS_SEND_BUFFER_POST_PADDING;
	}

	int LibwebsocketsClient::get_timer_period_us() {
		return 100000;
	}
}

#endif /* HAS_LIBWEBSOCKETS */

