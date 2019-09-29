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
#include <initializer_list>

namespace bitmap {
	class bitmap_image {
	public:
		typedef struct {
			unsigned short type;
			unsigned int   size;
			unsigned short reserved1;
			unsigned short reserved2;
			unsigned int   off_bits;

			unsigned int struct_size() const {
				return sizeof(type) +
					sizeof(size) +
					sizeof(reserved1) +
					sizeof(reserved2) +
					sizeof(off_bits);
			}

			void clear() {
				std::memset(this, 0x00, sizeof(bitmap_file_header));
			}
		} bitmap_file_header, * pbitmap_file_header;

		typedef struct {
			unsigned int   size;
			unsigned int   width;
			unsigned int   height;
			unsigned short planes;
			unsigned short bit_count;
			unsigned int   compression;
			unsigned int   size_image;
			unsigned int   x_pels_per_meter;
			unsigned int   y_pels_per_meter;
			unsigned int   clr_used;
			unsigned int   clr_important;

			unsigned int struct_size() const {
				return sizeof(size) +
					sizeof(width) +
					sizeof(height) +
					sizeof(planes) +
					sizeof(bit_count) +
					sizeof(compression) +
					sizeof(size_image) +
					sizeof(x_pels_per_meter) +
					sizeof(y_pels_per_meter) +
					sizeof(clr_used) +
					sizeof(clr_important);
			}

			void clear() {
				std::memset(this, 0x00, sizeof(bitmap_information_header));
			}
		} bitmap_information_header, * pbitmap_information_header;

		typedef struct {
			unsigned char peRed;
			unsigned char peGreen;
			unsigned char peBlue;
			unsigned char peFlags;
		} palette_color, * ppalette_color;
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

	public:
		std::string  file_name_;
		unsigned int width_;
		unsigned int height_;
		unsigned int stride_;
		channel_mode channel_mode_;
		unsigned int bitcount_;
		std::vector<unsigned char> data_;

	public:
		bitmap_image(const std::string& filename)
			: file_name_(filename),
			width_(0),
			height_(0),
			channel_mode_(bgr_mode) {
			load_bitmap();
		}

		bitmap_image(const unsigned int width, const unsigned int height)
			: file_name_(""),
			width_(width),
			height_(height),
			channel_mode_(bgr_mode),
			bitcount_(8) {
			create_bitmap();
		}

		bitmap_image(const unsigned int width, const unsigned int height, const unsigned int bitcount)
			: file_name_(""),
			width_(width),
			height_(height),
			channel_mode_(bgr_mode),
			bitcount_(bitcount) {
			create_bitmap();
		}

		bitmap_image(const unsigned int width, const unsigned int height, const unsigned int bitcount, std::initializer_list<unsigned char*> img)
			: file_name_(""),
			width_(width),
			height_(height),
			channel_mode_(bgr_mode),
			bitcount_(bitcount) {
			auto begin = img.begin();
			create_bitmap();
			if (img.size() == 3 && bitcount == 24) {
				if (begin[0] == NULL || begin[1] == NULL || begin[2] == NULL) {
					throw new std::exception("bitmap_image::bitmap_image ERROR: bitmap data address nums error");
				}
				for (int h = 0; h < height; h++) {
					int begs = h * width;
					int begs24 = h * width * 3;
					for (int w = 0, w24 = 0; w < width; w++, w24 += 3) {
						int wh = begs + w;
						int wh24 = begs24 + w24;
						data_[w24 + 2] = begin[0][wh];
						data_[w24 + 1] = begin[1][wh];
						data_[w24 + 0] = begin[2][wh];
					}
				}
			} else if (img.size() == 1 && bitcount == 24) {
				if (begin[0] == NULL) {
					throw new std::exception("bitmap_image::bitmap_image ERROR: bitmap data address nums error");
				}
				memcpy(const_cast<unsigned char*>(&data_[0]), begin[0], width * 3 * height);

			} else if (img.size() == 1 && bitcount == 8) {
				if (begin[0] == NULL) {
					throw new std::exception("bitmap_image::bitmap_image ERROR: bitmap data address nums error");
				}
				memcpy(const_cast<unsigned char*>(&data_[0]), begin[0], width* height);
			} else {
				throw new std::exception("bitmap_image::bitmap_image ERROR: bitmap data address nums error");
			}
		}
	public:
		void save_image(const std::string& file_name) {
			std::ofstream stream(file_name.c_str(), std::ios::binary);

			if (!stream) {
				std::cerr << "bitmap_image::save_image(): Error - Could not open file " << file_name << " for writing!" << std::endl;
				return;
			}

			bitmap_information_header bih;

			bih.width = width_;
			bih.height = height_;
			bih.bit_count = static_cast<unsigned short>(bitcount_);
			bih.clr_important = 0;
			bih.clr_used = 0;
			bih.compression = 0;
			bih.planes = 1;
			bih.size = bih.struct_size();
			bih.x_pels_per_meter = 0;
			bih.y_pels_per_meter = 0;
			bih.size_image = ((((bih.width * bih.bit_count) + 31) & ~31) >> 3)* bih.height;

			bitmap_file_header bfh;

			bfh.type = 19778;
			bfh.size = bfh.struct_size() + bih.struct_size() + bih.size_image;
			bfh.reserved1 = 0;
			bfh.reserved2 = 0;
			bfh.off_bits = bih.struct_size() + bfh.struct_size();

			write_bfh(stream, bfh);
			write_bih(stream, bih);

			if (bih.bit_count == 8) {
				palette_color color[256] = { 0 };
				for (int index = 0; index < 256; index++) {
					color[index].peRed = index;
					color[index].peGreen = index;
					color[index].peBlue = index;
				}
				stream.write((const char*)color, sizeof(palette_color) * 256);
			}

			int row_increment_ = ((((bih.width * bih.bit_count) + 31) & ~31) >> 3);
			for (unsigned int i = 0; i < height_; ++i) {
				const unsigned char* data_ptr = &data_[(row_increment_ * (height_ - i - 1))];
				stream.write(reinterpret_cast<const char*>(data_ptr), sizeof(unsigned char)* row_increment_);
			}
			stream.close();
		}

	private:
		void create_bitmap() {
			if (bitcount_ == 8) {
				data_.resize(width_ * height_);
			} else if (bitcount_ == 24) {
				data_.resize(width_ * height_ * 3);
			}
		}

		void load_bitmap() {
			std::ifstream stream(file_name_.c_str(), std::ios::binary);

			if (!stream) {
				std::cerr << "bitmap_image::load_bitmap() ERROR: bitmap_image - file " << file_name_ << " not found!" << std::endl;
				return;
			}

			width_ = 0;
			height_ = 0;

			bitmap_file_header bfh;
			bitmap_information_header bih;

			bfh.clear();
			bih.clear();

			read_bfh(stream, bfh);
			read_bih(stream, bih);

			if (bfh.type != 19778) {
				bfh.clear();
				bih.clear();

				stream.close();

				std::cerr << "bitmap_image::load_bitmap() ERROR: bitmap_image - Invalid type value " << bfh.type << " expected 19778." << std::endl;
				return;
			}

			if (bih.bit_count == 24) {
				bitcount_ = 24;
			} else if (bih.bit_count == 8) {
				bitcount_ = 8;
				stream.seekg(sizeof(palette_color) * 256, std::ios::cur);
			} else {
				bfh.clear();
				bih.clear();

				stream.close();

				std::cerr << "bitmap_image::load_bitmap() ERROR: bitmap_image - Invalid bit depth " << bih.bit_count << std::endl;

				return;
			}

			if (bih.size != bih.struct_size()) {
				bfh.clear();
				bih.clear();

				stream.close();

				std::cerr << "bitmap_image::load_bitmap() ERROR: bitmap_image - Invalid BIH size " << bih.size << " expected " << bih.struct_size() << std::endl;

				return;
			}

			width_ = bih.width;
			height_ = bih.height;
			stride_ = ((width_ * 3 + 3) & 0xFFFFFFFC);

			std::size_t bitmap_file_size = file_size(file_name_);
			std::size_t bitmap_logical_size = 0;
			if (bitcount_ == 8) {
				bitmap_logical_size = (height_ * width_)
					+ (sizeof(palette_color) * 256)
					+ bih.struct_size()
					+ bfh.struct_size();
			} else if (bitcount_ == 24) {
				unsigned int padding = (4 - ((3 * width_) % 4)) % 4;
				char padding_data[4] = { 0,0,0,0 };
				unsigned int bytes_per_pixel_ = bih.bit_count >> 3;
				bitmap_logical_size = (height_ * width_ * bytes_per_pixel_) +
					(height_ * padding) +
					bih.struct_size() +
					bfh.struct_size();
			}

			if (bitmap_file_size != bitmap_logical_size) {
				bfh.clear();
				bih.clear();

				stream.close();

				std::cerr << "bitmap_image::load_bitmap() ERROR: bitmap_image - Mismatch between logical and physical sizes of bitmap. " <<
					"Logical: " << bitmap_logical_size << " " <<
					"Physical: " << bitmap_file_size << std::endl;

				return;
			}

			if (bitcount_ == 8) {
				data_.resize(height_ * width_);
				for (unsigned int i = 0; i < height_; i++) {
					unsigned char* data_ptr = const_cast<unsigned char*>(&data_[(height_ - i - 1) * width_]);
					stream.read(reinterpret_cast<char*>(data_ptr), sizeof(char)* width_);
				}
			} else if (bitcount_ == 24) {
				data_.resize(height_ * 3 * width_);
				for (unsigned int i = 0; i < height_; i++) {
					unsigned char* data_ptr = const_cast<unsigned char*>(&data_[(height_ - i - 1) * 3 * width_]);
					stream.read(reinterpret_cast<char*>(data_ptr), sizeof(char) * 3 * width_);
				}
			}
			stream.close();
		}

		inline std::size_t file_size(const std::string& file_name) const {
			std::ifstream file(file_name.c_str(), std::ios::in | std::ios::binary);
			if (!file) return 0;
			file.seekg(0, std::ios::end);
			return static_cast<std::size_t>(file.tellg());
		}

		template <typename T>
		inline void read_from_stream(std::ifstream& stream, T& t) {
			stream.read(reinterpret_cast<char*>(&t), sizeof(T));
		}

		template <typename T>
		inline void write_to_stream(std::ofstream& stream, const T& t) const {
			stream.write(reinterpret_cast<const char*>(&t), sizeof(T));
		}

		inline bool big_endian() const {
			unsigned int v = 0x01;
			return (1 != reinterpret_cast<char*>(&v)[0]);
		}

		inline unsigned short flip(const unsigned short& v) const {
			return ((v >> 8) | (v << 8));
		}

		inline void read_bfh(std::ifstream& stream, bitmap_file_header& bfh) {
			read_from_stream(stream, bfh.type);
			read_from_stream(stream, bfh.size);
			read_from_stream(stream, bfh.reserved1);
			read_from_stream(stream, bfh.reserved2);
			read_from_stream(stream, bfh.off_bits);

			if (big_endian()) {
				bfh.type = flip(bfh.type);
				bfh.size = flip(bfh.size);
				bfh.reserved1 = flip(bfh.reserved1);
				bfh.reserved2 = flip(bfh.reserved2);
				bfh.off_bits = flip(bfh.off_bits);
			}
		}

		inline void write_bfh(std::ofstream& stream, const bitmap_file_header& bfh) const {
			if (big_endian()) {
				write_to_stream(stream, flip(bfh.type));
				write_to_stream(stream, flip(bfh.size));
				write_to_stream(stream, flip(bfh.reserved1));
				write_to_stream(stream, flip(bfh.reserved2));
				write_to_stream(stream, flip(bfh.off_bits));
			} else {
				write_to_stream(stream, bfh.type);
				write_to_stream(stream, bfh.size);
				write_to_stream(stream, bfh.reserved1);
				write_to_stream(stream, bfh.reserved2);
				write_to_stream(stream, bfh.off_bits);
			}
		}

		inline void read_bih(std::ifstream& stream, bitmap_information_header& bih) {
			read_from_stream(stream, bih.size);
			read_from_stream(stream, bih.width);
			read_from_stream(stream, bih.height);
			read_from_stream(stream, bih.planes);
			read_from_stream(stream, bih.bit_count);
			read_from_stream(stream, bih.compression);
			read_from_stream(stream, bih.size_image);
			read_from_stream(stream, bih.x_pels_per_meter);
			read_from_stream(stream, bih.y_pels_per_meter);
			read_from_stream(stream, bih.clr_used);
			read_from_stream(stream, bih.clr_important);

			if (big_endian()) {
				bih.size = flip(bih.size);
				bih.width = flip(bih.width);
				bih.height = flip(bih.height);
				bih.planes = flip(bih.planes);
				bih.bit_count = flip(bih.bit_count);
				bih.compression = flip(bih.compression);
				bih.size_image = flip(bih.size_image);
				bih.x_pels_per_meter = flip(bih.x_pels_per_meter);
				bih.y_pels_per_meter = flip(bih.y_pels_per_meter);
				bih.clr_used = flip(bih.clr_used);
				bih.clr_important = flip(bih.clr_important);
			}
		}

		inline void write_bih(std::ofstream& stream, const bitmap_information_header& bih) const {
			if (big_endian()) {
				write_to_stream(stream, flip(bih.size));
				write_to_stream(stream, flip(bih.width));
				write_to_stream(stream, flip(bih.height));
				write_to_stream(stream, flip(bih.planes));
				write_to_stream(stream, flip(bih.bit_count));
				write_to_stream(stream, flip(bih.compression));
				write_to_stream(stream, flip(bih.size_image));
				write_to_stream(stream, flip(bih.x_pels_per_meter));
				write_to_stream(stream, flip(bih.y_pels_per_meter));
				write_to_stream(stream, flip(bih.clr_used));
				write_to_stream(stream, flip(bih.clr_important));
			} else {
				write_to_stream(stream, bih.size);
				write_to_stream(stream, bih.width);
				write_to_stream(stream, bih.height);
				write_to_stream(stream, bih.planes);
				write_to_stream(stream, bih.bit_count);
				write_to_stream(stream, bih.compression);
				write_to_stream(stream, bih.size_image);
				write_to_stream(stream, bih.x_pels_per_meter);
				write_to_stream(stream, bih.y_pels_per_meter);
				write_to_stream(stream, bih.clr_used);
				write_to_stream(stream, bih.clr_important);
			}
		}
	};
}