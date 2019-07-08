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

		class Disconnect : public Base {
		public:
			Disconnect(Client *client)
				: Base(Frame::Commands::SUBSCRIBE)
			{
			}

			Disconnect& receipt(const std::string& value) {
				frame_.header("receipt", value);
				return *this;
			}

			const std::string& receipt() const {
				return frame_.header("receipt");
			}

		};

	}

}

