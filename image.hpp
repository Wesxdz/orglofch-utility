/*
* Copyright (c) 2015 Owen Glofcheski
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source
*    distribution.
*/

#ifndef _IMAGE_HPP_
#define _IMAGE_HPP_

#include <iostream>

#include <cassert>
#include <cstdint>
#include <fstream>
#include <string>
#include <png.h>
#include <vector>
#include <zlib.h>

#include "algebra.hpp"
#include "colour.hpp"
#include "util.hpp"

class Image
{
public:
	Image() : channels(0), data(NULL) {}
	Image(const Size &size, int channels)
		: size(size), channels(channels), data(new float[size.area() * channels]) {}
	Image(const Size &size, int channels, float *data)
		: size(size), channels(channels), data(data) {}

	~Image() {
		if (data) {
			delete data;
		}
	}

	/** Create a new image that is this image rotated about its center. */
	Image *rotate(float angle) const {
		// TODO(orglofch): This only works for 90 degree angles

		// Perform the inverse rotation to rotate the new image into
		// the old images coordinate system.
		Point2 center = size.center();
		Matrix4x4 transformation =
			Matrix4x4::translation(center.x, center.y, 0) *
			Matrix4x4::rotation('z', 90).invert() *
			Matrix4x4::translation(-center.x, -center.y, 0);

		float *new_data = new float[size.area() * channels];
		for (int x = 0; x < size.x; ++x) {
			for (int y = 0; y < size.y; ++y) {
				int index = (y * size.x + x) * channels;

				Point3 position = transformation * Point3(x, y, 0);
				// TODO(orglofch): Interpolate between the nearest pixels instead of rounding.
				position.y = floor(position.y) - 1;
				int old_index = (position.y * size.x + position.x) * channels;
				for (int c = 0; c < channels; ++c) {
					new_data[index + c] = data[old_index + c];
				}
			}
		}
		return new Image(size, channels, new_data);
	}

	/** Create a new image this is this image reflect across its center. */
	Image *reflect(const Vector3 &plane) const {
		float *new_data = new float[size.area() * channels];
		for (int i = 0; i < size.area(); ++i) {
			// TODO(orglofch):
		}
		return new Image(size, channels, new_data);
	}

	void set(int x, int y, int channel, float val) {
		data[(y * size.x + x) * channels + channel] = val;
	}

	void set(int x, int y, const Colour &colour) {
		for (int c = 0; c < channels; ++c) {
			data[(y * size.x + x) * channels + c] = colour[c];
		}
	}

	Colour colour(int x, int y) const {
		Colour colour;
		for (int i = 0; i < channels; ++i) {
			colour[i] = data[(y * size.x + x) * channels + i];
		}
		return colour;
	}

	Size size;
	int channels;
	float *data;
};

inline
bool LoadBMP(const std::string &filename, Image *image) {
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;

	FILE *file = fopen(filename.c_str(), "rb");
	if (!file) {
		LOG("Unable to open %s\n", filename.c_str());
		return false;
	}

	if (fread(header, 1, 54, file) != 54) {
		LOG("%s is not a BMP file\n", filename.c_str());
		return false;
	}

	if (header[0] != 'B' || header[1] != 'M') {
		LOG("%s is not a BMP file\n", filename.c_str());
		return false;
	}

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	image->size.x = *(int*)&(header[0x12]);
	image->size.y = *(int*)&(header[0x16]);

	if (imageSize == 0) {
		imageSize = image->size.area() * 4;
		image->channels = 4; // TODO(orglofch): Figure out location
	}
	if (dataPos == 0) {
		dataPos = 54;
	}
	image->data = new float[imageSize];

	fread(image->data, 1, imageSize, file);

	fclose(file);
	return true;
}

// TODO(orglofch): Make easier, merge into structure
inline
int dataIndex(unsigned int width,
		unsigned int height,
		unsigned int channels,
		unsigned int x,
		unsigned int y,
		unsigned int c) {
	return channels * ((height - y - 1) * width + x) + c;
}

inline
bool LoadPNG(const std::string &filename, Image *image) {
	png_byte buf[8];

	FILE* in = fopen(filename.c_str(), "rb");
	if (!in) {
		LOG("Unable to open %s\n", filename.c_str());
		return false;
	}

	for (int i = 0; i < 8; ++i) {
		if (!(buf[i] = fgetc(in))) {
			LOG("Unable to read %s\n", filename.c_str());
			return false;
		}
	}
	if (png_sig_cmp(buf, 0, 8)) {
		LOG("Bad PNG signature %s\n", filename.c_str());
		return false;
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr) {
		LOG("Failed to acquire png_ptr %s\n", filename.c_str());
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		LOG("Failed to acquire info_ptr %s\n", filename.c_str());
		png_destroy_read_struct(&png_ptr, 0, 0);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		LOG("Failed to setjmp %s\n", filename.c_str());
		png_destroy_read_struct(&png_ptr, 0, 0);
		return false;
	}

	png_init_io(png_ptr, in);

	png_set_sig_bytes(png_ptr, 8);

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, 0);

	int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	if (bit_depth % 8) {
		png_destroy_read_struct(&png_ptr, 0, 0);
		LOG("Invalid bit depth %d, %s\n", bit_depth, filename.c_str());
		return false;
	}

	int colour_type = png_get_color_type(png_ptr, info_ptr);

	if (colour_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
		colour_type = PNG_COLOR_TYPE_RGB;
	}

	if (colour_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	}

	if (colour_type != PNG_COLOR_TYPE_RGB
		&& colour_type != PNG_COLOR_TYPE_GRAY
		&& colour_type != PNG_COLOR_TYPE_RGBA) {
		png_destroy_read_struct(&png_ptr, 0, 0);
		LOG("Invalid colour_type %d, %s\n", colour_type, filename.c_str());
		return false;
	}

	image->size.x = png_get_image_width(png_ptr, info_ptr);
	image->size.y = png_get_image_height(png_ptr, info_ptr);
	switch (colour_type) {
	case PNG_COLOR_TYPE_RGB:
		image->channels = 3;
		break;
	case PNG_COLOR_TYPE_RGBA:
		image->channels = 4;
		break;
	default:
		image->channels = 1;
		break;
	}

	png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);

	image->data = new float[image->size.area() * image->channels];
	assert(image->data);

	for (int y = 0; y < image->size.y; y++) {
		for (int x = 0; x < image->size.x; x++) {
			for (int c = 0; c < image->channels; c++) {
				png_byte *row = row_pointers[y];
				int index = dataIndex(image->size.x, image->size.y, image->channels, x, y, c);

				long element = 0;
				for (int j = bit_depth / 8 - 1; j >= 0; j--) {
					element <<= 6;
					element += row[(x * image->channels + c) * bit_depth / 8 + j];
				}

				image->data[index] = element / static_cast<float>((1 << bit_depth) - 1);
			}
		}
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	return true;
}

inline
void WritePNG(const Image &image, const std::string &filename) {
	FILE *fout = fopen(filename.c_str(), "wb");
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info_ptr = png_create_info_struct(png_ptr);
	setjmp(png_jmpbuf(png_ptr));

	png_init_io(png_ptr, fout);

	png_set_filter(png_ptr, 0, PNG_FILTER_PAETH);

	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

	int color_type;
	switch (image.channels) {
	case 1:
		color_type = PNG_COLOR_TYPE_GRAY;
		break;
	case 2:
		color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
		break;
	case 3:
		color_type = PNG_COLOR_TYPE_RGB;
		break;
	case 4:
	default:
		color_type = PNG_COLOR_TYPE_RGBA;
		break;
	}

	png_set_IHDR(png_ptr, info_ptr, image.size.x, image.size.y, 8,
		color_type,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);

	png_byte *temp_line = new png_byte[image.size.x * image.channels];

	for (int y = 0; y < image.size.y; ++y) {
		for (int x = 0; x < image.size.x; ++x) {
			for (int c = 0; c < image.channels; ++c) {
				int index = dataIndex(image.size.x, image.size.y, image.channels, x, y, c);

				double value = std::min(1.0f, std::max(0.0f, image.data[index]));

				temp_line[image.channels * x + c] = static_cast<png_byte>(value * 255.0);
			}
		}
		png_write_row(png_ptr, temp_line);
	}

	delete temp_line;

	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fout);
}

#endif