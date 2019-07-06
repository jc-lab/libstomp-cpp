/**
 * @file	frame.cpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/ )
 * @date	2019/07/05
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#include "frame.hpp"

#include <vector>

#include <string.h>

#if !defined(_MSC_VER)
#define strtok_s strtok_r
#endif

namespace stomp {

	const std::string
		Frame::Commands::CONNECT("CONNECT"),
		Frame::Commands::CONNECTED("CONNECTED"),
		Frame::Commands::SUBSCRIBE("SUBSCRIBE"),
		Frame::Commands::MESSAGE("MESSAGE")
		;

	const std::string
		Frame::Headers::CONTENT_TYPE("content-type"),
		Frame::Headers::CONTENT_LENGTH("content-length"),
		Frame::Headers::ACCEPT_VERSION("accept-version"),
		Frame::Headers::HEART_BEAT("heart-beat")
		;

	Frame::Frame()
		: empty_value_(), prediction_size_(0)
	{}

	Frame::Frame(const std::string& command)
		: empty_value_(), prediction_size_(0), command_(command)
	{}

	Frame& Frame::command(const std::string& value) {
		command_ = value;
		return *this;
	}
	const std::string& Frame::command() const {
		return command_;
	}

	static void string_to_lower(std::string &text) {
		for (std::string::iterator iter = text.begin(); iter != text.end(); iter++)
		{
			*iter = tolower(*iter);
		}
	}

	Frame& Frame::header(const std::string& name, const std::string& value) {
		std::string lower_name(name);
		string_to_lower(lower_name);
		headers_.emplace(lower_name, value);
		prediction_size_ += name.size() + value.size() + 2;
		return *this;
	}
	const std::string& Frame::header(const std::string& name) const {
		std::string lower_name(name);
		string_to_lower(lower_name);
		std::unordered_map<std::string, std::string>::const_iterator iter = headers_.find(lower_name);
		if (iter != headers_.end()) {
			return iter->second;
		}
		return empty_value_;
	}

	bool Frame::has_header(const std::string& name) const
	{
		std::unordered_map<std::string, std::string>::const_iterator iter = headers_.find(name);
		if (iter != headers_.end()) {
			return true;
		}
		return false;
	}

	Frame& Frame::body(const std::string& value) {
		body_ = value;
		return *this;
	}

	std::string& Frame::refValue() {
		return body_;
	}

	const std::string& Frame::body() const {
		return body_;
	}

	std::string Frame::destination() const
	{
		for (std::unordered_map<std::string, std::string>::const_iterator iter = headers_.begin(); iter != headers_.end(); iter++)
		{
			if (iter->first == "destination")
				return iter->second;
		}
		return "";
	}

	std::string Frame::contentType() const
	{
		for (std::unordered_map<std::string, std::string>::const_iterator iter = headers_.begin(); iter != headers_.end(); iter++)
		{
			if (iter->first == "content-type")
				return iter->second;
		}
		return "";
	}

	std::string Frame::subscription() const
	{
		for (std::unordered_map<std::string, std::string>::const_iterator iter = headers_.begin(); iter != headers_.end(); iter++)
		{
			if (iter->first == "subscription")
				return iter->second;
		}
		return "";
	}

	std::string Frame::messageId() const
	{
		for (std::unordered_map<std::string, std::string>::const_iterator iter = headers_.begin(); iter != headers_.end(); iter++)
		{
			if (iter->first == "message-id")
				return iter->second;
		}
		return "";
	}

	int Frame::contentLength() const
	{
		for (std::unordered_map<std::string, std::string>::const_iterator iter = headers_.begin(); iter != headers_.end(); iter++)
		{
			if (iter->first == "content-length")
			{
				return atoi(iter->second.c_str());
			}
		}
		return body_.size();
	}

	void Frame::make_payload_append(std::vector<char>& output) const {
		output.reserve(command_.size() + headers_.size() * 32 + body_.size() + 32);
		output.insert(output.end(), command_.begin(), command_.end());
		output.push_back('\n');
		for (std::unordered_map<std::string, std::string>::const_iterator iter = headers_.begin(); iter != headers_.end(); iter++)
		{
			std::string encoded_key(header_encode(iter->first));
			std::string encoded_value(header_encode(iter->second));
			output.insert(output.end(), encoded_key.begin(), encoded_key.end());
			output.push_back(':');
			output.insert(output.end(), encoded_value.begin(), encoded_value.end());
			output.push_back('\n');
		}
		output.push_back('\n');
		output.insert(output.end(), body_.begin(), body_.end());
		output.push_back(0);
	}

	std::vector<char> Frame::make_payload() const {
		std::vector<char> temp;
		make_payload_append(temp);
		return temp;
	}

	int Frame::utf8_bytes(const char* string, int remainlen) {
		const unsigned char* bytes = (const unsigned char*)string;
		if ((// ASCII
			 // use bytes[0] <= 0x7F to allow ASCII control characters
			bytes[0] == 0x09 ||
			bytes[0] == 0x0A ||
			bytes[0] == 0x0D ||
			(0x20 <= bytes[0] && bytes[0] <= 0x7E)
			)
			) {
			return 1;
		}

		if (remainlen < 2)
			return 0;

		if ((// non-overlong 2-byte
			(0xC2 <= bytes[0] && bytes[0] <= 0xDF) &&
			(0x80 <= bytes[1] && bytes[1] <= 0xBF)
			)
			) {
			return 2;
		}

		if (remainlen < 3)
			return 0;

		if ((// excluding overlongs
			bytes[0] == 0xE0 &&
			(0xA0 <= bytes[1] && bytes[1] <= 0xBF) &&
			(0x80 <= bytes[2] && bytes[2] <= 0xBF)
			) ||
			(// straight 3-byte
			((0xE1 <= bytes[0] && bytes[0] <= 0xEC) ||
				bytes[0] == 0xEE ||
				bytes[0] == 0xEF) &&
				(0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
				(0x80 <= bytes[2] && bytes[2] <= 0xBF)
				) ||
				(// excluding surrogates
					bytes[0] == 0xED &&
					(0x80 <= bytes[1] && bytes[1] <= 0x9F) &&
					(0x80 <= bytes[2] && bytes[2] <= 0xBF)
					)
			) {
			return 3;
		}

		if (remainlen < 4)
			return 0;

		if ((// planes 1-3
			bytes[0] == 0xF0 &&
			(0x90 <= bytes[1] && bytes[1] <= 0xBF) &&
			(0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
			(0x80 <= bytes[3] && bytes[3] <= 0xBF)
			) ||
			(// planes 4-15
			(0xF1 <= bytes[0] && bytes[0] <= 0xF3) &&
				(0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
				(0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
				(0x80 <= bytes[3] && bytes[3] <= 0xBF)
				) ||
				(// plane 16
					bytes[0] == 0xF4 &&
					(0x80 <= bytes[1] && bytes[1] <= 0x8F) &&
					(0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
					(0x80 <= bytes[3] && bytes[3] <= 0xBF)
					)
			) {
			return 4;
		}

		return 0; // non-validated
	}

	std::string Frame::header_encode(const std::string& raw)
	{
		std::string output;
		int n = raw.size();
		int i = 0;
		output.reserve(raw.size() + 16);

		for (i = 0; i < n; i++)
		{
			const char* ptr = &raw[i];
			int m = utf8_bytes(ptr, n - i);
			if (m <= 1)
			{
				char c = *ptr;
				switch (c)
				{
				case '\r':
					output.append("\\r");
					break;
				case '\n':
					output.append("\\n");
					break;
				case ':':
					output.append("\\c");
					break;
				case '\\':
					output.append("\\\\");
					break;
				default:
					output.append(&c, 1);
				}
			}
			else {
				output.append(ptr, m);
			}
		}
		return output;
	}

	std::string Frame::header_decode(const std::string& encoded)
	{
		std::string output;
		int n = encoded.size();
		int i = 0;
		output.reserve(encoded.size());

		for (i = 0; i < n; i++)
		{
			bool spec_char = false;
			const char* ptr = &encoded[i];
			int m = utf8_bytes(ptr, n - i);
			if (m <= 1)
			{
				char c = *ptr;
				int remaining = n - i - 1;
				if (c == '\\')
				{
					if (remaining > 0)
					{
						char c = ptr[1];
						switch (c)
						{
						case 'r':
							output.append("\r");
							spec_char = true;
							break;
						case 'n':
							output.append("\n");
							spec_char = true;
							break;
						case 'c':
							output.append(":");
							spec_char = true;
							break;
						case '\\':
							output.append("\\");
							spec_char = true;
							break;
						}
					}
				}
			}
			if (spec_char)
				i++;
			else
				output.append(ptr, m);
		}
		return output;
	}

} // namespace stomp


