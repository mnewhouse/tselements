/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "pattern.hpp"
#include "terrain_library.hpp"

#include "utility/stream_utilities.hpp"
#include "utility/rect.hpp"

#include <stdexcept>
#include <vector>
#include <array>
#include <random>
#include <algorithm>

extern "C"
{
#include <png.h>
}

namespace ts
{
  namespace png
  {
    struct InfoStruct
    {
      InfoStruct() = default;

      InfoStruct(const InfoStruct&) = delete;
      InfoStruct& operator=(const InfoStruct&) = delete;

      png_structp& png_ptr() { return png_ptr_; }
      const png_structp& png_ptr() const { return png_ptr_; }

      png_infop& info_ptr() { return info_ptr_; }
      const png_infop& info_ptr() const { return info_ptr_; };

    protected:
      explicit InfoStruct(png_structp png_ptr, png_infop info_ptr)
        : png_ptr_(png_ptr),
        info_ptr_(info_ptr)
      {
      }

    private:
      png_structp png_ptr_ = nullptr;
      png_infop info_ptr_ = nullptr;
    };

    struct ReadInfo
      : InfoStruct
    {
      ReadInfo()
      {
        png_ptr() = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        info_ptr() = png_create_info_struct(png_ptr());
      }

      ~ReadInfo()
      {
        if (info_ptr())
        {
          png_destroy_info_struct(png_ptr(), &info_ptr());
        }

        if (png_ptr())
        {
          png_destroy_read_struct(&png_ptr(), nullptr, nullptr);
        }
      }

      explicit operator bool() const
      {
        return png_ptr() && info_ptr();
      }
    };

    struct WriteInfo
      : InfoStruct
    {
      WriteInfo()
      {
        png_ptr() = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        info_ptr() = png_create_info_struct(png_ptr());
      }

      ~WriteInfo()
      {
        if (info_ptr())
        {
          png_destroy_info_struct(png_ptr(), &info_ptr());
        }

        if (png_ptr())
        {
          png_destroy_write_struct(&png_ptr(), nullptr);
        }
      }
    };

    struct ReaderStruct
    {
      const char* data_;
      const char* end_;
    };

    struct WriterStruct
    {
      std::basic_ofstream<char>* out_;
    };

    void read_using_reader_struct(png_structp png_ptr, png_bytep out_bytes, png_size_t byte_count)
    {
      auto* reader = static_cast<ReaderStruct*>(png_ptr->io_ptr);
      if (reader)
      {
        png_size_t bytes_left = reader->end_ - reader->data_;
        if (bytes_left >= byte_count)
        {
          std::copy(reader->data_, reader->data_ + byte_count, out_bytes);
          reader->data_ += byte_count;
        }
      }
    }

    void write_using_writer_struct(png_structp png_ptr, png_bytep out_bytes, png_size_t byte_count)
    {
      auto writer = static_cast<WriterStruct*>(png_ptr->io_ptr);
      if (writer)
      {
        writer->out_->write(reinterpret_cast<const char*>(out_bytes), byte_count);
      }
    }

    void flush_output(png_structp png_ptr)
    {
      auto writer = static_cast<WriterStruct*>(png_ptr->io_ptr);
      if (writer)
      {
        writer->out_->flush();
      }
    }
  }

  namespace resources
  {
    Pattern load_pattern(const std::string& file_name, IntRect rect)
    {
      auto stream = make_ifstream(file_name, std::ifstream::in | std::ifstream::binary);
      if (stream)
      {
        auto file_contents = read_stream_contents(stream);
        if (file_contents.size() >= 8 && png_check_sig(reinterpret_cast<png_bytep>(file_contents.data()), 8))
        {
          png::ReadInfo png_info;
          auto& read_ptr = png_info.png_ptr();
          auto& info_ptr = png_info.info_ptr();

          if (png_info && setjmp(png_jmpbuf(read_ptr)) == 0)
          {
            png::ReaderStruct reader;
            reader.data_ = reinterpret_cast<const char*>(file_contents.data() + 8);
            reader.end_ = reinterpret_cast<const char*>(file_contents.data() + file_contents.size());

            png_set_read_fn(read_ptr, static_cast<void*>(&reader), png::read_using_reader_struct);
            png_set_sig_bytes(read_ptr, 8);
            png_read_info(read_ptr, info_ptr);

            // Must be paletted image
            if (png_get_color_type(read_ptr, info_ptr) == PNG_COLOR_TYPE_PALETTE)
            {
              std::int32_t image_width = png_get_image_width(read_ptr, info_ptr);
              std::int32_t image_height = png_get_image_height(read_ptr, info_ptr);

              Pattern pattern({ image_width, image_height });

              std::vector<png_bytep> row_pointers(image_height);

              for (std::int32_t row = 0; row != image_height; ++row)
              {
                row_pointers[row] = reinterpret_cast<png_bytep>(pattern.row_begin(row));
              }

              // Read the bytes directly into the pattern object
              png_read_image(read_ptr, row_pointers.data());
              png_read_end(read_ptr, info_ptr);

              // Only if we reach this point, we can avoid throwing an exception.
              // Pretty god-awful, so many levels of indentation.
              return pattern;
            }
          }
        }
      }

      throw std::runtime_error(file_name);
    }

    Pattern copy_pattern(const Pattern& pattern, IntRect source_rect)
    {
      Pattern result(make_vector2(source_rect.width, source_rect.height));

      auto bottom = source_rect.bottom(), right = source_rect.right();
      for (auto dst_y = 0, src_y = source_rect.top; src_y != bottom; ++src_y, ++dst_y)
      {
        std::copy_n(pattern.row_begin(src_y) + source_rect.left, source_rect.width, result.row_begin(dst_y));
      }

      return result;
    }
    
    static std::array<png_color, 256> create_palette(const TerrainLibrary& terrain_library)
    {
      // Create a color palette based on terrain_library colors.
      std::array<png_color, 256> palette;
      for (std::uint32_t terrain_id = 0; terrain_id != 256; ++terrain_id)
      {
        const auto& terrain = terrain_library.terrain(terrain_id);
        const auto& color = terrain.color;

        palette[terrain_id].red = color.r;
        palette[terrain_id].green = color.g;
        palette[terrain_id].blue = color.b;
      }

      return palette;
    }


    void save_pattern(const Pattern& pattern, const std::string& file_name, const TerrainLibrary& terrain_library)
    {
      auto out = make_ofstream(file_name, std::ios::binary | std::ios::out);
      if (!out) throw std::runtime_error("could not save pattern file to '" + file_name + "'");

      png::WriterStruct writer{};
      writer.out_ = &out;
      png::WriteInfo write_info{};

      auto& png_ptr = write_info.png_ptr();
      auto& info_ptr = write_info.info_ptr();

      png_set_write_fn(png_ptr, &writer, png::write_using_writer_struct, png::flush_output);
      auto pattern_size = pattern.size();

      std::vector<png_bytep> pattern_rows(pattern_size.y);
      for (std::uint32_t y = 0; y != pattern_size.y; ++y)
      {
        // Oh my.
        pattern_rows[y] = const_cast<png_byte*>(pattern.row_begin(y));
      }

      png_set_IHDR(png_ptr, info_ptr, pattern_size.x, pattern_size.y, 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
                   PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

      auto palette = create_palette(terrain_library);
      png_set_PLTE(png_ptr, info_ptr, palette.data(), palette.size());

      png_write_info(png_ptr, info_ptr);
      png_write_image(png_ptr, pattern_rows.data());
      png_write_end(png_ptr, info_ptr);
    }

    Pattern::Pattern(Vector2i size)
      : size_(size), 
        bytes_(size.x * size.y, 0)
    {
    }

    Pattern::const_iterator ts::resources::Pattern::begin() const
    {
      return bytes_.data();
    }

    Pattern::const_iterator ts::resources::Pattern::end() const
    {
      return bytes_.data();
    }

    Pattern::iterator ts::resources::Pattern::begin()
    {
      return bytes_.data();
    }

    Pattern::iterator ts::resources::Pattern::end()
    {
      return bytes_.data();
    }

    Vector2i Pattern::size() const
    {
      return size_;
    }

    void Pattern::resize(Vector2i new_size)
    {
      bytes_.resize(new_size.x * new_size.y);

      size_.x = new_size.x;
      size_.y = new_size.y;
    }

    void Pattern::resize(std::int32_t width, std::int32_t height)
    {
      resize({ width, height });
    }
  }
}