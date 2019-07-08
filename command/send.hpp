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

#include <stdio.h>

namespace stomp {

	namespace command {

		class Send : public Base {
		public:
			Send(Client *client)
				: Base(Frame::Commands::SEND)
			{
			}

			Send& destination(const std::string& value) {
				frame_.header("destination", value);
				return *this;
			}

			Send& transaction(const std::string& value) {
				frame_.header("transaction", value);
				return *this;
			}

			Send& content_type(const std::string& value) {
				frame_.header(Frame::Headers::CONTENT_TYPE, value);
				return *this;
			}

			Send& body(const std::string& value) {
				char buf[64];
				snprintf(buf, sizeof(buf), "%d", value.length());
				frame_.header(Frame::Headers::CONTENT_LENGTH, buf);
				frame_.body(value);
				return *this;
			}

			const std::string& destination() {
				return frame_.header("destination");
			}

			const std::string& transaction() {
				return frame_.header("transaction");
			}

			const std::string& content_type() {
				return frame_.header(Frame::Headers::CONTENT_TYPE);
			}

			const int content_length() {
				return frame_.contentLength();
			}

			const std::string& body() {
				return frame_.body();
			}
		};

	}

}

