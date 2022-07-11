#include "yoyo.h"
#include "../plugin_sdk/plugin_sdk.hpp"

#define DEFAULT_PING_ENEMY_COUNT 2
#define DEFAULT_PING_ALLY_DIED_COUNT 3

namespace yoyo {

	std::map<uint16_t, bool> enemies_visible;
	std::map <uint16_t, vector> enemies_last_position;
	std::map<uint16_t, bool> allies_dead;
	std::map <uint16_t, vector> allies_last_position;

	TreeTab* main_tab = nullptr;

	namespace ping_settings {
		TreeEntry* ping_enemy_appear = nullptr;
		TreeEntry* ping_enemy_miss = nullptr;
		TreeEntry* ping_enemy_count = nullptr;
		TreeEntry* ping_ally_died_switch = nullptr;
		TreeEntry* ping_ally_died_myself = nullptr;
		TreeEntry* ping_ally_died_count = nullptr;
	}

	namespace default_settings {
		// int ping_enemy_count = nullptr;
	}

	// pingEnemyStatus: cast ping when enemies appear and disappear
	void pingEnemyStatus() {
		auto ping_miss = ping_settings::ping_enemy_miss->get_bool();
		auto ping_appear = ping_settings::ping_enemy_appear->get_bool();
		auto ping_count = ping_settings::ping_enemy_count->get_int();

		for (auto&& enemy : entitylist->get_enemy_heroes()) {
			const auto enemy_id = enemy->get_id();
			const auto old_visible = enemies_visible[enemy_id];
			const auto visible = enemy->is_visible();
			// update visibility
			enemies_visible[enemy->get_id()] = visible;
			if (visible) {
				auto pos = enemy->get_position();
				if (pos.is_valid()) {
					// update last position
					enemies_last_position[enemy_id] = pos;
					if (!old_visible) {
						// danger
						if (ping_appear && ping_count > 0) {
							for (int i = 0; i < ping_count; ++i) {
								myhero->cast_ping(pos, enemy, _player_ping_type::danger);
							}
						}
					}
				}
			}
			else if (old_visible) {
				const auto last_pos = enemies_last_position[enemy_id];
				if (last_pos.is_valid()) {
					// miss
					if (ping_miss && ping_count > 0) {
						for (int i = 0; i < ping_count; ++i) {
							myhero->cast_ping(last_pos, nullptr, _player_ping_type::missing_enemy);
						}
					}
				}
			}
		}
	}

	// pingAllyDied: cast ping when allies die (trick allies)
	void pingAllyDied() {
		auto ping_died = ping_settings::ping_ally_died_switch->get_bool();
		auto ping_myself_died = ping_settings::ping_ally_died_myself->get_bool();
		auto ping_count = ping_settings::ping_ally_died_count->get_int();

		for (auto&& ally : entitylist->get_ally_heroes()) {
			const auto ally_id = ally->get_id();
			const auto old_dead = allies_dead[ally_id];
			const auto dead = ally->is_dead();
			// update dead
			allies_dead[ally->get_id()] = dead;
			if (!dead) {
				auto pos = ally->get_position();
				if (pos.is_valid()) {
					// update last position
					allies_last_position[ally_id] = pos;
				}
			}
			else if (!old_dead) {
				const auto last_pos = allies_last_position[ally_id];
				const auto is_me = ally->is_me();
				if (last_pos.is_valid()) {
					// trick them
					if (ping_died && (ping_count > 0) && (!is_me || ping_myself_died)) {
						for (int i = 0; i < ping_count; ++i) {
							myhero->cast_ping(last_pos, nullptr, _player_ping_type::missing_enemy);
						}
					}
				}
			}
		}
	}

	void on_update() {
		// if (myhero->is_dead()) {
		//     return;
		// }
		pingEnemyStatus();
		pingAllyDied();
	}

	void load() {
		main_tab = menu->create_tab("yoyo_util", "Yoyo Utility");

		auto ping_enemy_tab = main_tab->add_tab("yoyo_util.ping.enemy", "Enemy ping");
		{
			ping_settings::ping_enemy_appear = ping_enemy_tab->add_checkbox("yoyo_util.ping.enemy.appear",
				"When enemy appear",
				true);
			ping_settings::ping_enemy_miss = ping_enemy_tab->add_checkbox("yoyo_util.ping.enemy.miss",
				"When enemy miss",
				true);
			ping_settings::ping_enemy_count = ping_enemy_tab->add_slider("yoyo_util.ping.enemy.count",
				"Ping count every time",
				DEFAULT_PING_ENEMY_COUNT, 0, 5);
		}

		auto ping_ally_died_tab = main_tab->add_tab("yoyo_util.ping.ally.died", "Ally died ping");
		{
			ping_settings::ping_ally_died_switch = ping_ally_died_tab->add_checkbox("yoyo_util.ping.ally.died.switch",
				"Ally died ping", true);
			ping_settings::ping_ally_died_switch->set_tooltip("Taunt allies when they die...");
			ping_settings::ping_ally_died_myself = ping_ally_died_tab->add_checkbox("yoyo_util.ping.ally.died.myself",
				"Include myself", true);
			ping_settings::ping_ally_died_count = ping_ally_died_tab->add_slider("yoyo_util.ping.ally.died.count",
				"Ping count every time",
				DEFAULT_PING_ALLY_DIED_COUNT, 1, 8);
		}

		event_handler<events::on_update>::add_callback(on_update);
	}

	void unload() {
		event_handler<events::on_update>::remove_handler(on_update);

		menu->delete_tab(main_tab);
	}
}
