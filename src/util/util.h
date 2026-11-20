#pragma once
#include <cassert>
#include <charconv>
#include <cstdint>
#include <string>

namespace util
{
bool read_file(const char *file_path, std::string &out_content);

inline constexpr uint32_t pack_as_int(const char *c4, const int count) noexcept
{
	assert(count > 0 && count < 5);
	return static_cast<uint32_t>(c4[0]) << 24 |
	       (count > 1 ? static_cast<uint32_t>(c4[1]) : 0 << 16) |
	       (count > 2 ? static_cast<uint32_t>(c4[2]) : 0 << 8) |
	       (count > 3 ? static_cast<uint32_t>(c4[3]) : 0);
}

template <typename T>
bool set_values(size_t &delim_idx, const std::string_view &line, T *values,
                const std::string_view &delims, int count)
{
	size_t nchars;
	size_t offset{delim_idx + 1};
	bool consumed_line = false;
	for (int i{0}; i < count; ++i)
	{
		delim_idx = line.find_first_of(delims, offset);

		consumed_line = delim_idx == std::string::npos;

		nchars = consumed_line ? line.length() - offset : delim_idx - offset;

		std::from_chars(line.data() + offset, line.data() + offset + nchars,
		                values[i]);

		if (consumed_line)
		{
			return true;
		}

		offset = delim_idx + 1;
	}

	return false;
}
} // namespace util
