#include "../plugin_sdk/plugin_sdk.hpp"

#include "yoyo.h"

// Declare plugin name & supported champions
//
PLUGIN_NAME("Yoyo Utility");
PLUGIN_TYPE(plugin_type::utility);

PLUGIN_API bool on_sdk_load(plugin_sdk_core* plugin_sdk_good) {
	DECLARE_GLOBALS(plugin_sdk_good);

	yoyo::load();
	console->print_success("Yoyo Utility loaded.");
	return true;
}

PLUGIN_API void on_sdk_unload() {
	yoyo::unload();
	console->print_success("Yoyo Utility unloaded.");
	console->print("===");
}