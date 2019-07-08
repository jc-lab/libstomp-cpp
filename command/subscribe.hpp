/**
 * @file	subscribe.hpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/ )
 * @date	2019/07/08
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#pragma once

#include <string>

#include "base.hpp"
#include "../frame.hpp"
#include "../client.hpp"

namespace stomp {

	namespace command {

		class Subscribe : public Base {
		public:
			Subscribe(Client *client, bool auto_id = true)
				: Base(Frame::Commands::SUBSCRIBE)
			{
				if(auto_id)
					frame_.header("id", client->generateSubscribeId());
			}

			Subscribe& id(const std::string& value) {
				frame_.header("id", value);
				return *this;
			}

			Subscribe& destination(const std::string& value) {
				frame_.header("destination", value);
				return *this;
			}

			Subscribe& ack(const std::string& value) {
				frame_.header("ack", value);
				return *this;
			}

			const std::string& id() {
				return frame_.header("id");
			}

			const std::string& destination() {
				return frame_.header("destination");
			}

			const std::string& ack() {
				return frame_.header("ack");
			}
		};

	}

}

