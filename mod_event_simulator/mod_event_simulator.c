/*
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2012, Anthony Minessale II <anthm@freeswitch.org>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Anthony Minessale II <anthm@freeswitch.org>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Ádám Gólya <adam.stork@gmail.com>
 *
 *
 * mod_event_simulator.c -- FS Event simulator
 *
 */
#include <switch.h>

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_event_simulator_shutdown);
SWITCH_MODULE_LOAD_FUNCTION(mod_event_simulator_load);
SWITCH_MODULE_DEFINITION(mod_event_simulator, mod_event_simulator_load, mod_event_simulator_shutdown, NULL);

static struct {
	char *path;
	int event_number;
	int test;

} globals;

static switch_xml_config_item_t instructions[] = {
	SWITCH_CONFIG_ITEM_STRING_STRDUP("path", CONFIG_RELOAD, &globals.path, NULL, "./", ""),
	SWITCH_CONFIG_ITEM("event-number", SWITCH_CONFIG_INT, CONFIG_RELOADABLE, &globals.event_number, (void *) 1, NULL, NULL, NULL),
	SWITCH_CONFIG_ITEM("test", SWITCH_CONFIG_BOOL, CONFIG_RELOAD, &globals.test, SWITCH_FALSE, NULL, "true|false", "test"),
	SWITCH_CONFIG_ITEM_END()
};

static switch_status_t do_config(switch_bool_t reload)
{
	memset(&globals, 0, sizeof(globals));
	if (switch_xml_config_parse_module_settings("event_simulator.conf", reload, instructions) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Could not open event_simulator.conf\n");
		return SWITCH_STATUS_FALSE;
	}

	return SWITCH_STATUS_SUCCESS;
}

SWITCH_STANDARD_API(event_simulator_function)
{
	switch_event_t *event;
	FILE *fp;
	long lSize;
	char *buffer;
	int i;

	fp = fopen ( globals.path , "rb" );
	if( !fp ) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Could not open file: %s\n", globals.path);
		return SWITCH_STATUS_SUCCESS;
	}

	fseek( fp , 0L , SEEK_END);
	lSize = ftell( fp );
	rewind( fp );

	buffer = calloc( 1, lSize+1 );
	if( !buffer ) {
		fclose(fp);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Memory alloc was failed\n");
		return SWITCH_STATUS_SUCCESS;
	}

	if( 1!=fread( buffer , lSize, 1 , fp) ) {
		fclose(fp);
		free(buffer);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Read the whole file was failed\n");
		return SWITCH_STATUS_SUCCESS;
	}

	fclose(fp);

	if (switch_event_create_json(&event, buffer) == SWITCH_STATUS_SUCCESS) {
		for (i = 0; i < globals.event_number; i++) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Fire event number: %i\n", i);
			switch_event_fire(&event);
		}
		switch_event_destroy(&event);
	}

	free(buffer);
	return SWITCH_STATUS_SUCCESS;

}

SWITCH_MODULE_LOAD_FUNCTION(mod_event_simulator_load)
{
	switch_api_interface_t *api_interface;
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	SWITCH_ADD_API(api_interface, "event_simulator", "event_simulator API", event_simulator_function, "syntax");

	do_config(SWITCH_TRUE);

	return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_event_simulator_shutdown)
{

	switch_safe_free(globals.path);

	return SWITCH_STATUS_SUCCESS;
}

/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4 noet
 */
