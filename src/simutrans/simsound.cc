/*
 * This file is part of the Simutrans project under the Artistic License.
 * (see LICENSE.txt)
 */

#include <stdio.h>
#include <string.h>
#include "macros.h"
#include "music/music.h"
#include "descriptor/sound_desc.h"
#include "sound/sound.h"
#include "simsound.h"
#include "sys/simsys.h"
#include "simio.h"
#include "simdebug.h"

#include "dataobj/environment.h"
#include "utils/plainstring.h"
#include "utils/simrandom.h"
#include "utils/simstring.h"


static bool new_midi = false;

static struct midi_info_t midi_list[MAX_MIDI];

static int max_midi = -1; // number of MIDI files

static int current_midi = -1;  // init with error condition, reset during loading


void sound_set_global_volume(int volume)
{
	env_t::global_volume = volume;
}


void sound_set_specific_volume( int volume, sound_type_t t)
{
	env_t::specific_volume[t] = volume;
}


int sound_get_global_volume()
{
	return env_t::global_volume;
}


int sound_get_specific_volume( sound_type_t t )
{
	return env_t::specific_volume[t];
}


void sound_set_mute(bool f)
{
	env_t::global_mute_sound = f;
}


bool sound_get_mute()
{
	return (  env_t::global_mute_sound  );
}


void sound_play(const uint16 sound_index, const uint8 vol, const sound_type_t type)
{
	if(sound_index != (uint16)NO_SOUND && env_t::global_mute_sound == false) {
        const uint32 type_volume = env_t::specific_volume[type];
        const uint32 sound_volume = vol;
		dr_play_sample(sound_index, ((sound_volume * env_t::global_volume * type_volume) >> 16));
	}
}


/**
 * Clean up data structures, free resources
 */
void close_sound()
{
    dr_destroy_sound();
}


bool sound_get_shuffle_midi()
{
	return env_t::shuffle_midi;
}


void sound_set_shuffle_midi( bool shuffle )
{
	env_t::shuffle_midi = shuffle;
}


void sound_set_midi_volume(int volume)
{
	if(  !env_t::mute_midi  &&  max_midi > -1  ) {
		dr_set_midi_volume(volume);
	}
	env_t::midi_volume = volume;
}



int sound_get_midi_volume()
{
	return env_t::midi_volume;
}



/**
 * gets midi title
 */
struct midi_info_t sound_get_midi_info(int index)
{
	if (  index >= 0  &&  index <= max_midi  ) {
		return midi_list[index];
	}
	return { "Invalid MIDI Index!", "-", "-" };
}


/**
 * gets current midi number
 */
int get_current_midi()
{
	return current_midi;
}


/**
 * Load MIDI files
 */
int midi_init(const char *directory)
{
	// read a list of soundfiles
	std::string full_path = std::string(directory) + "music" + PATH_SEPARATOR + "music.tab";

	if(  FILE* const file = dr_fopen(full_path.c_str(), "rb")  ) {
		while(!feof(file)) {
			char buf[256], title[256], composer[256], arranger[256];
			size_t len;

			read_line(buf,   sizeof(buf),   file);
			read_line(title, sizeof(title), file);
			read_line(composer, sizeof(composer), file);
			read_line(arranger, sizeof(arranger), file);
			if(  !feof(file)  ) {
				clear_invalid_ending_chars(buf);
				len = strlen(buf);
				if(  len > 1  ) {
					full_path = std::string(directory) + buf;
					dbg->message("midi_init()", "  Reading MIDI file '%s' - %s", full_path.c_str(), title);
					max_midi = dr_load_midi(full_path.c_str());

					if(  max_midi >= 0  ) {
						midi_list[max_midi].title = (std::string) clear_invalid_ending_chars(title);
						midi_list[max_midi].composer = (std::string) clear_invalid_ending_chars(composer);
						midi_list[max_midi].arranger = (std::string) clear_invalid_ending_chars(arranger);
					}
				}
			}
		}

		fclose(file);
	}
	else {
		dbg->warning("midi_init()","can't open file '%s' for reading.", full_path.c_str() );
	}

	if(  max_midi >= 0  ) {
		current_midi = 0;
	}
	// success?
	return (  max_midi >= 0  );
}


void midi_play(const int no)
{
	if(  no > max_midi  ) {
		dbg->warning("midi_play()", "MIDI index %d too high (total loaded: %d)", no, max_midi);
	}
	else if(  !midi_get_mute()  ) {
		current_midi = (no < 0) ? sim_async_rand( max_midi ) : no;
		dr_play_midi( current_midi );
	}
}


void midi_stop()
{
	if(  !midi_get_mute()  ) {
		dr_stop_midi();
	}
}



void midi_set_mute(bool on)
{
	on |= (  max_midi == -1  );
	if(  on  ) {
		if(  !env_t::mute_midi  ) {
			dr_stop_midi();
		}
		env_t::mute_midi = true;
	}
	else {
		if(  env_t::mute_midi  ) {
			env_t::mute_midi = false;
			midi_play(current_midi);
		}
		dr_set_midi_volume(env_t::midi_volume);
	}
}



bool midi_get_mute()
{
	return  (  env_t::mute_midi  ||  max_midi == -1  );
}



/*
 * Check if need to play new MIDI
 * Max Kielland:
 * Made it possible to get next song
 * even if we are muted.
 */
void check_midi()
{
	// Check for next sound
	if (new_midi || (!midi_get_mute() && dr_midi_pos() < 0)) {
		if(  env_t::shuffle_midi  &&  max_midi > 1  ) {

			// shuffle songs (must not use simrand()!)
			int new_song = sim_async_rand(max_midi);

			if(  new_song >= current_midi  ) {
				new_song ++;
			}
			current_midi = new_song;
		}
		else {
			current_midi++;
			if(  current_midi > max_midi  ) {
				current_midi = 0;
			}
		}

		// Are we in playing mode?
		if(  false == midi_get_mute()  ) {
			midi_play(current_midi);
			DBG_MESSAGE("check_midi()", "Playing MIDI %d", current_midi);
		}
	}

	new_midi = false;
}


/**
 * shuts down midi playing
 */
void close_midi()
{
	if(  max_midi > -1  ) {
		dr_destroy_midi();
	}
}


void midi_next_track()
{
	new_midi = true;
}


void midi_last_track()
{
	if (  current_midi == 0  ) {
		current_midi = max_midi - 1;
	}
	else {
		current_midi = current_midi - 2;
	}
	new_midi = true;
}
