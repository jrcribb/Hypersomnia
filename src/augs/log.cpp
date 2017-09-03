#include <iostream>
#include <string>
#include <thread>
#include <mutex>

#include "augs/log.h"
#include "augs/math/vec2.h"

#include "augs/filesystem/file.h"
#include "augs/templates/string_templates.h"

#define ENABLE_LOG 1
#define LOG_TO_FILE 0

std::mutex log_mutex;

program_log program_log::global_instance = 10000;

program_log::program_log(const unsigned max_all_entries) 
	: max_all_entries(max_all_entries) 
{
}

void program_log::push_entry(const log_entry& new_entry) {
	all_entries.push_back(new_entry);

	if (all_entries.size() > max_all_entries) {
		all_entries.erase(all_entries.begin(), all_entries.begin() + max_all_entries/5);
	}
}

void program_log::save_complete_to(const augs::path_type& path) {
	std::string complete_log;

	for (const auto& e : all_entries) {
		complete_log += e.text + '\n';
	}
	
	augs::create_text_file(path, complete_log);
}

template<>
void LOG(const std::string& f) {
#if ENABLE_LOG 
	std::unique_lock<std::mutex> lock(log_mutex);

	program_log::get_current().push_entry({ console_color::WHITE, f });

	std::cout << f << std::endl;
#if LOG_TO_FILE
	std::ofstream recording_file("generated/logs/live_debug.txt", std::ios::out | std::ios::app);
	recording_file << f << std::endl;
#endif
#endif
}

void CALL_SHELL(const std::string& s) {
	std::unique_lock<std::mutex> lock(log_mutex);

	system(s.c_str());
}