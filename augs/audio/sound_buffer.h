#pragma once
#include <vector>

typedef unsigned int ALuint;

namespace augs {
	class sound_buffer {
		sound_buffer(const sound_buffer&) = delete;
		sound_buffer(sound_buffer&&) = delete;
		sound_buffer& operator=(const sound_buffer&) = delete;
		sound_buffer& operator=(sound_buffer&&) = delete;
		
		struct data_type {
			std::vector<int16_t> samples;
			int frequency = 0;
			int channels = 0;

			int get_format() const;
		};

		data_type data;

		ALuint id = 0;
	public:
		sound_buffer();
		~sound_buffer();

		void from_file(const std::string filename);
		void set_data(const data_type);
		data_type get_data() const;

		double get_length_in_seconds() const;

		ALuint get_id() const;
		operator ALuint() const;
	};
}