/**
 * @file	frame_reader.cpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/ )
 * @date	2019/07/06
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#include "frame_reader.hpp"

#include <vector>

#include <string.h>

#if !defined(_MSC_VER)
#define strtok_s strtok_r
#endif

namespace stomp {

	static void trim_end_type1(char* text) {
		int len = strlen(text);
		while ((len > 0) && ((text[len - 1] == '\r')))
			len--;
		text[len] = 0;
	}

	static int trim_end_type2(char* text) {
		int len = strlen(text);
		while ((len > 0) && ((text[len - 1] == '\r') || (text[len - 1] == ' ') || (text[len - 1] == '\n')))
			len--;
		text[len] = 0;
		return len;
	}

	static int my_strlen_s(const char* text, int max_length)
	{
		int cnt = 0;
		while (*(text++) && cnt < max_length)
			cnt++;
		return cnt;
	}

	void FrameReader::reset()
	{
		state_ = READ_HEADERS;
		line_buffer_.clear();
		reading_frame_.reset();
		line_buffer_.reserve(1024);
		reading_content_length_ = 0;
	}

	/*
	 * @return If fully read line then return true, otherwise false.
	 */
	bool FrameReader::ReadContext::read_header_line()
	{
		while (remaining()) {
			char c = read_char();
			if (c == '\n')
				return true;
			else
				reader_->line_buffer_.push_back(c);
		}
		return false;
	}

	bool FrameReader::parseAddHeader(Frame *frame, char* text)
	{
		char* value = NULL;
		char* key = strtok_s(text, ":", &value);
		if (!key || !value)
			return false;
		frame->header(key, Frame::header_decode(value));
		return true;
	}

	int FrameReader::decode(const char* buffer, int len, std::list< std::unique_ptr<Frame> >& out)
	{
		ReadContext read_context(this, buffer, len);

		while (read_context.remaining()) {
			switch (state_)
			{
			case READ_HEADERS:
				if (read_context.read_header_line())
				{
					do {
						if (reading_frame_.get()) {
							int line_length = line_buffer_.size();
							if (!line_buffer_.empty()) {
								line_length = trim_end_type2(&line_buffer_[0]);
							}
							if (line_length > 0) {
								parseAddHeader(reading_frame_.get(), &line_buffer_[0]);
							}
							else {
								if (reading_frame_->has_header(Frame::Headers::CONTENT_LENGTH)) {
									reading_content_length_ = reading_frame_->contentLength();
									if (reading_content_length_ == 0) {
										state_ = READ_EOF;
										break;
									}
								}
								state_ = READ_CONTENT;
							}
						}
						else {
							if (line_buffer_.empty()) {
								// Heartbeat
								printf("HEARTBEAT RECEVIVE\n");
								reset();
							}
							else {
								reading_frame_.reset(new Frame(line_buffer_));
							}
						}
					} while (false);
					line_buffer_.clear();
				}
				break;
			case READ_CONTENT:
				do {
					int buf_remaining = read_context.remaining();
					int readable_length = my_strlen_s(read_context.current_ptr(), buf_remaining);
					line_buffer_.append(read_context.current_ptr(), readable_length);
					read_context.read_pos_ += readable_length;
					if (readable_length < buf_remaining) {
						state_ = READ_EOF;
					}
				} while (false);
				break;
			case READ_EOF:
				if (read_context.read_char() == 0)
				{
					reading_frame_->body(line_buffer_);
					out.emplace_back(std::move(reading_frame_));
					reading_frame_.reset();
					reset();
				}
				break;
			}
		}
		return 0;
	}

} // namespace stomp

