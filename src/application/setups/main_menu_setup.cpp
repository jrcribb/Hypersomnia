#include "augs/log.h"
#include "augs/misc/http/http_requests.h"
#include "augs/drawing/drawing.hpp"
#include "augs/misc/action_list/standard_actions.h"

#include "augs/filesystem/file.h"
#include "augs/string/string_templates.h"

#include "augs/audio/sound_data.h"
#include "augs/audio/sound_buffer.h"
#include "augs/audio/sound_source.h"

#include "augs/gui/text/caret.h"
#include "augs/gui/text/printer.h"
#include "augs/drawing/drawing.h"

#include "game/assets/all_logical_assets.h"

#include "game/organization/all_component_includes.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "game/organization/all_messages_includes.h"
#include "game/cosmos/data_living_one_step.h"

#include "game/detail/visible_entities.h"

#include "application/config_lua_table.h"

#include "application/setups/main_menu_setup.h"

#include "application/gui/menu/menu_root.h"
#include "application/gui/menu/menu_context.h"
#include "application/gui/menu/appearing_text.h"
#include "application/gui/menu/creators_screen.h"

#include "augs/readwrite/lua_readwrite.h"
#include "hypersomnia_version.h"
#include "application/main/application_updates.h"

using namespace augs::event::keys;
using namespace augs::gui::text;
using namespace augs::gui;

entity_id main_menu_setup::get_viewed_character_id() const {
	return viewed_character_id;
}

void main_menu_setup::customize_for_viewing(config_lua_table& config) const {
	const auto previous_sfx_volume = config.audio_volume.sound_effects;
	augs::read_lua(menu_config_patch, config);
	
	/* Treat new volume as a multiplier */

	config.audio_volume.sound_effects *= previous_sfx_volume;
}

void main_menu_setup::apply(const config_lua_table& config) {
	menu_theme_source.set_gain(config.audio_volume.music);
}

void main_menu_setup::launch_creators_screen() {
#if TODO
	if (credits_actions.is_complete()) {
		creators = creators_screen();
		creators.setup(textes_style, style(assets::font_id::GUI_FONT, { 0, 180, 255, 255 }), window.get_screen_size());

		credits_actions.push_blocking(act(new augs::tween_value_action<rgba_channel>(fade_overlay_color.a, 170, 500.f)));
		creators.push_into(credits_actions);
		credits_actions.push_blocking(act(new augs::tween_value_action<rgba_channel>(fade_overlay_color.a, 20, 500.f)));
	}
#endif
}

void main_menu_setup::query_latest_news(const std::string& url) {
	// TODO: FIX THIS!!!
	if (latest_news.valid()) {
		latest_news.wait();
	}

	latest_news = std::async(std::launch::async, [&url]() noexcept {
		return std::string("");

		auto html = augs::http_get_request(url);
		const auto delimiter = std::string("newsbegin");

		const auto it = html.find(delimiter);

		if (it != std::string::npos) {
			html = html.substr(it + delimiter.length());

			str_ops(html)
				.multi_replace_all({ "\r", "\n" }, "")
			;

			if (html.size() > 0) {
				return html;
			}
		}

		return std::string("Couldn't download and/or parse the latest news.");
	});
}

main_menu_setup::main_menu_setup(
	sol::state& lua,
	const main_menu_settings settings
) {
	try {
		menu_theme.emplace(settings.menu_theme_path);
	}
	catch (const std::runtime_error& err) {
		LOG("Warning: could not load the main menu theme:\n%x", err.what());
	}

	query_latest_news(settings.latest_news_url);

	if (settings.skip_credits) {
		gui.show = true;
	}

	const auto menu_config_patch_path = "content/menu/config.lua";

	try {
		auto pfr = lua.do_string(augs::file_to_string(menu_config_patch_path));
		
		if (pfr.valid()) {
			menu_config_patch = pfr;
		}
		else {
			LOG("Warning: problem loading %x: \n%x.", menu_config_patch_path, pfr.operator std::string());
		}
	}
	catch (const augs::file_open_error& err) {
		LOG("Failed to load %x:\n%x\nMenu will apply no patch to config.", menu_config_patch_path, err.what());
	}

	if (menu_theme) {
		menu_theme_source.bind_buffer(*menu_theme);
		menu_theme_source.set_direct_channels(true);
		menu_theme_source.set_spatialize(false);
		menu_theme_source.seek_to(static_cast<float>(settings.start_menu_music_at_secs));
		menu_theme_source.set_gain(0.f);
		menu_theme_source.play();
	}

	// TODO: actually load a cosmos with its resources from a file/folder
	const bool is_intro_scene_available = settings.menu_intro_scene_intercosm_path.string().size() > 0;

	auto& cosm = intro.world;

	if (is_intro_scene_available) {
#if BUILD_TEST_SCENES
		intro.make_test_scene(lua, { false, 60 }, ruleset);
		viewed_character_id = cosm[mode.lookup(mode.add_player({ ruleset, cosm }, faction_type::RESISTANCE))].get_id();
#endif
	}

	const bool is_recording_available = is_intro_scene_available && false;
	initial_step_number = cosm.get_total_steps_passed();

	for (auto& m : gui.root.buttons) {
		m.hover_highlight_maximum_distance = 10.f;
		m.hover_highlight_duration_ms = 300.f;
	}

	gui.root.buttons[main_menu_button_type::CONNECT_TO_OFFICIAL_UNIVERSE].set_appearing_caption("Login to official universe");
	gui.root.buttons[main_menu_button_type::BROWSE_COMMUNITY_SERVERS].set_appearing_caption("Browse unofficial universes");
	gui.root.buttons[main_menu_button_type::HOST_UNIVERSE].set_appearing_caption("Host universe");
	gui.root.buttons[main_menu_button_type::CONNECT_TO_UNIVERSE].set_appearing_caption("Connect to universe");
	gui.root.buttons[main_menu_button_type::LOCAL_UNIVERSE].set_appearing_caption("Local universe");
	gui.root.buttons[main_menu_button_type::EDITOR].set_appearing_caption("Editor");
	gui.root.buttons[main_menu_button_type::SETTINGS].set_appearing_caption("Settings");
	gui.root.buttons[main_menu_button_type::CREATORS].set_appearing_caption("Founders");
	gui.root.buttons[main_menu_button_type::QUIT].set_appearing_caption("Quit");

	if (is_recording_available) {
		while (cosm.get_total_seconds_passed() < settings.rewind_intro_scene_by_secs) {
			mode.advance(
				{ ruleset, cosm },
				mode_entropy(),
				solver_callbacks(),
				solve_settings()
			);
		}
	}
}

void main_menu_setup::draw_overlays(
	const application_update_result& last_update_result,
	const augs::drawer_with_default output,
	const necessary_images_in_atlas_map& necessarys,
	const augs::baked_font& gui_font,
	const vec2i screen_size
) const {
	const auto game_logo = necessarys.at(assets::necessary_image_id::MENU_GAME_LOGO);
	const auto game_logo_size = game_logo.get_original_size();

	ltrb game_logo_rect;
	game_logo_rect.set_position({ screen_size.x / 2.f - game_logo_size.x / 2.f, 50.f });
	game_logo_rect.set_size(game_logo_size);

	output.aabb(game_logo, game_logo_rect);

	if (is_ready(latest_news)) {
		print_stroked(
			output,
			vec2i(latest_news_pos),
			from_bbcode ( latest_news.get(), { gui_font, cyan } )
		);
	};

	const auto s = style { gui_font, white };

	using FS = formatted_string;
	const auto build_number_text = FS(typesafe_sprintf("Build %x", hypersomnia_version().get_version_number()), s);

	vec2i padding = { 6, 6 };

	print_stroked(
		output,
		vec2i(screen_size.x, screen_size.y) - padding,
		build_number_text,
		{ augs::ralign::R, augs::ralign::B }
	);

	const auto bbox = get_text_bbox(build_number_text);

	auto colored = [&](const auto& text, const auto& color) {
		return FS(std::string(text) + " ", { gui_font, color });
	};

	const auto description = [&]() {
		const auto t = last_update_result.type;
		using R = application_update_result_type;

		switch (t) {
			case R::NONE:
				return colored("Automatic updates are disabled.", gray);
			case R::CANCELLED:
				return colored("Automatic update was cancelled!", orange);
			case R::FIRST_LAUNCH_AFTER_UPGRADE:
				return colored("Success! Hypersomnia was upgraded to the latest version.", green);
			case R::UP_TO_DATE:
				return colored("Hypersomnia is up to date.", green);
			case R::FAILED:
				return colored("Failed to connect with the update server.", red);

			default: return colored("", white);
		}
	}();
	
	print_stroked(
		output,
		vec2i(screen_size.x - bbox.x, screen_size.y) - padding,
		description,
		{ augs::ralign::R, augs::ralign::B }
	);
}