#pragma once
#include <charconv>
#include <string>

namespace util
{
bool read_file(const char *file_path, std::string &out_content);

inline constexpr uint16_t pack_as_int(char a, char b) noexcept
{
	return static_cast<uint16_t>(static_cast<uint8_t>(a) << 8 |
	                             static_cast<uint8_t>(b));
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
