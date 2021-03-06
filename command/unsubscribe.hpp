/**
 * @file	unsubscribe.hpp
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

		class Unsubscribe : public Base {
		public:
			Unsubscribe(Client *client)
				: Base(Frame::Commands::UNSUBSCRIBE)
			{
			}

			Unsubscribe& id(const std::string& value) {
				frame_.header("id", value);
				return *this;
			}

			const std::string& id() {
				return frame_.header("id");
			}
		};

	}

}

