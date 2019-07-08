/**
 * @file	client.hpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/ )
 * @date	2019/07/05
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#pragma once

#include <assert.h>

#include "frame.hpp"
#include "command/base.hpp"

namespace stomp {

	class Client {
	public:
		enum State {
			DISCONNECTED = 0,
			CONNECTING = 1,
			CONNECTED = 2,
		};

		class MessageBuffer {
		public:
			virtual char* data_ptr() = 0;
			virtual int data_size() = 0;
		};

	public:
		Client() {}
		virtual ~Client() {}

		virtual State state() const = 0;

		virtual int onConnected(Frame* frame) { return 0; }
		virtual int onMessage(Frame* frame) { return 0; }
		virtual int onClosed() { return 0; }

		virtual int sendFrame(Frame *frame) = 0;

		virtual int sendCommand(command::Base* item) = 0;

		virtual std::string generateSubscribeId() = 0;
		virtual std::string generateTransactionId() = 0;
	};

}

