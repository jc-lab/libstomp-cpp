/**
 * @file	base.hpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/ )
 * @date	2019/07/08
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#pragma once

#ifdef _DEBUG
#include <assert.h>
#endif

#include "../frame.hpp"

namespace stomp {

	namespace command {

		class Base {
		private:
#ifdef _DEBUG
			Base(const Base& o) { assert(false); }
#endif

		protected:
			Frame frame_;

		public:
			Base(const std::string& command)
				: frame_(command)
			{}
			virtual ~Base() {}
			Frame* frame() {
				return &frame_;
			}
		};

	}

}

