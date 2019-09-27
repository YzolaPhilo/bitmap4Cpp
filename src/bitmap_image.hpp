#pragma once

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <string>
#include <vector>

class bitmap_image {
private:
	std::string  _file_name_;
	unsigned int _width_;
	unsigned int _height_;
	unsigned int _stride_;
	channel_mode _channel_mode_;
	bitcount_mode _bitcount;
	std::vector<unsigned char> data_;

public:
	bitmap_image(const std::string& filename)
		: _file_name_(filename),
		_width_(0),
		_height_(0),
		_channel_mode_(bgr_mode) {
		//load_bitmap();
	}

	bitmap_image(const unsigned int width, const unsigned int height)
		: _file_name_(""),
		_width_(width),
		_height_(height),
		_channel_mode_(bgr_mode),
		_bitcount(bit8){
		//create_bitmap();
	}

	bitmap_image(const unsigned int width, const unsigned int height, bitcount_mode bitcount)
		: _file_name_(""),
		_width_(width),
		_height_(height),
		_channel_mode_(bgr_mode) {
		//create_bitmap();
	}

	bitmap_image(const unsigned int width, const unsigned int heigh, bitcount_mode bitcount, ...)
		: _file_name_(""),
		_width_(width),
		_height_(height),
		_channel_mode_(bgr_mode) {
		//create_bitmap();
	}
public:
	enum channel_mode {
		rgb_mode = 0,
		bgr_mode = 1
	};

	enum color_plane {
		blue_plane = 0,
		green_plane = 1,
		red_plane = 2
	};

	static enum bitcount_mode {
		bit8 = 0,
		bit24 = 1
	};

	struct rgb_t {
		unsigned char   red;
		unsigned char green;
		unsigned char  blue;
	};


};