/**
 * @file	frame.hpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/ )
 * @date	2019/07/05
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#pragma once

#include <string>
#include <list>
#include <unordered_map>
#include <vector>

namespace stomp {

	class Frame {
	private:
		const std::string empty_value_;

	protected:
		std::string command_;
		std::unordered_map<std::string, std::string> headers_;
		std::string body_;

		size_t prediction_size_;

	public:
		struct Commands {
			static const std::string CONNECT;
			static const std::string CONNECTED;
			static const std::string SUBSCRIBE;
			static const std::string MESSAGE;
		};

		struct Headers {
			static const std::string CONTENT_TYPE;
			static const std::string CONTENT_LENGTH;
			static const std::string ACCEPT_VERSION;
			static const std::string HEART_BEAT;
		};
		
		Frame();
		Frame(const std::string& command);
		Frame& command(const std::string& value);
		const std::string& command() const;
		Frame& header(const std::string& name, const std::string& value);
		const std::string& header(const std::string& key) const;
		bool has_header(const std::string& key) const;
		Frame& body(const std::string& value);
		std::string& refValue();
		const std::string& body() const;

		std::string destination() const;
		std::string contentType() const;
		std::string subscription() const;
		std::string messageId() const;
		int contentLength() const;

		void make_payload_append(std::vector<char>& output) const;
		std::vector<char> make_payload() const;

		static std::string header_encode(const std::string& raw);
		static std::string header_decode(const std::string& encoded);

		static int utf8_bytes(const char* string, int remainlen);
	};
}


