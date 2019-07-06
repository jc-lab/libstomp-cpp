/**
 * @file	frame_reader.hpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/ )
 * @date	2019/07/06
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#pragma once

#include <list>
#include <memory>

#include "frame.hpp"

namespace stomp {

	class FrameReader {
	private:
		enum State {
			READ_HEADERS,
			READ_CONTENT,
			READ_EOF
		};

		struct ReadContext {
			FrameReader* reader_;
			const char* buf_;
			int length_;
			int read_pos_;

			ReadContext(FrameReader *reader, const char* buf, int length) {
				reader_ = reader;
				buf_ = buf;
				length_ = length;
				read_pos_ = 0;
			}

			bool read_header_line();

			const char* current_ptr() const {
				return &buf_[read_pos_];
			}

			char read_char() {
				return buf_[read_pos_++];
			}

			int remaining() const {
				return length_ - read_pos_;
			}

			int read_position() const {
				return read_pos_;
			}
		};

		State state_;
		std::string line_buffer_;
		std::unique_ptr<Frame> reading_frame_;
		int reading_content_length_;

		bool parseAddHeader(Frame* frame, char* text);

	public:
		void reset();
		int decode(const char* buffer, int len, std::list< std::unique_ptr<Frame> >& out);
	};
}


