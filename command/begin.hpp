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

		class Begin : public Base {
		public:
			Begin(Client *client, bool auto_id = false)
				: Base(Frame::Commands::SUBSCRIBE)
			{
				if(auto_id)
					frame_.header("transaction", client->generateTransactionId());
			}

			const std::string& transaction() {
				return frame_.header("transaction");
			}
		};

	}

}

