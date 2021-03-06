/*************************************************************************/
/*  os_vita.cpp                                                          */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "os_vita.h"

#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/message_dialog.h>
#include <psp2/rtc.h> 

#include "servers/visual/visual_server_wrap_mt.h"
#include "drivers/unix/file_access_unix.h"
#include "drivers/unix/dir_access_unix.h"
#include "drivers/unix/ip_unix.h"
#include "drivers/unix/net_socket_posix.h"
#include "drivers/unix/thread_posix.h"
#include "drivers/gles2/rasterizer_gles2.h"
#include "main/main.h"
#include "drivers/unix/dir_access_unix.h"
#include "drivers/unix/file_access_unix.h"
#include "core/os/dir_access.h"
#include "core/os/file_access.h"
#include "drivers/dummy/rasterizer_dummy.h"
#include "drivers/dummy/texture_loader_dummy.h"
#include "servers/audio_server.h"

void OS_Vita::initialize_core()
{
	FileAccess::make_default<FileAccessUnix>(FileAccess::ACCESS_RESOURCES);
	FileAccess::make_default<FileAccessUnix>(FileAccess::ACCESS_USERDATA);
	FileAccess::make_default<FileAccessUnix>(FileAccess::ACCESS_FILESYSTEM);
	DirAccess::make_default<DirAccessUnix>(DirAccess::ACCESS_RESOURCES);
	DirAccess::make_default<DirAccessUnix>(DirAccess::ACCESS_USERDATA);
	DirAccess::make_default<DirAccessUnix>(DirAccess::ACCESS_FILESYSTEM);
}

void OS_Vita::finalize_core()
{
}

void OS_Vita::finalize()
{
	visual_server->finish();
	memdelete(visual_server);
	memdelete(gl_context);
}

Error OS_Vita::execute(const String &p_path, const List<String> &p_arguments, bool p_blocking, int64_t *r_child_id, String *r_pipe, int *r_exitcode, bool read_stderr, Mutex *p_pipe_mutex)
{
    return FAILED;
}

Error OS_Vita::kill(const ProcessID &p_pid)
{
	return FAILED;
}

bool OS_Vita::has_environment(const String &p_var) const
{
	return false;
}

String OS_Vita::get_environment(const String &p_var) const
{
	return "";
}

bool OS_Vita::set_environment(const String &p_var, const String &p_value) const
{
	return false;
}

OS::Date OS_Vita::get_date(bool local) const
{
	return OS::Date();
}

OS::Time OS_Vita::get_time(bool local) const
{
	return OS::Time();
}

OS::TimeZoneInfo OS_Vita::get_time_zone_info() const
{
	return OS::TimeZoneInfo();
}

void OS_Vita::delay_usec(uint32_t p_usec) const
{
	sceKernelDelayThread(p_usec);
}

uint64_t OS_Vita::get_ticks_usec() const
{
    static int tick_resolution = sceRtcGetTickResolution();
    SceRtcTick current_tick;
    sceRtcGetCurrentTick(&current_tick);
    return current_tick.tick / (tick_resolution / 1000000);
}

String OS_Vita::get_stdin_string(bool p_block) { return ""; }

void OS_Vita::swap_buffers() {
	glClearColor(1.0, 0.0, 0.0, 1.0);
    gl_context->swap_buffers();
}

int OS_Vita::get_audio_driver_count() const {
	return 1;
}

const char *OS_Vita::get_audio_driver_name(int p_driver) const {

	return "Dummy";
}

Error OS_Vita::initialize(const VideoMode &p_desired, int p_video_driver, int p_audio_driver) {

	//args = OS::get_singleton()->get_cmdline_args();
	//current_videomode = p_desired;
	
    bool gles3_context = false;

    gl_context = NULL;

    gl_context = memnew(Context_VitaGL());

    gl_context->initialize();
	
	RasterizerDummy::make_current();
	
	AudioDriverManager::initialize(p_audio_driver);

	RasterizerGLES2::register_config();
	RasterizerGLES2::make_current();

	visual_server = memnew(VisualServerRaster);
    
    video_driver_index = p_video_driver;

	visual_server->init();

	return OK;
}

void OS_Vita::run() {
    if (!main_loop)
		return;

	main_loop->init();

	while (true) {

		if (Main::iteration())
			break;
	};

	main_loop->finish();
}

String OS_Vita::get_name() const {

	return "Vita";
}

MainLoop *OS_Vita::get_main_loop() const {

	return main_loop;
}

void OS_Vita::delete_main_loop() {

	if (main_loop)
		memdelete(main_loop);
	main_loop = NULL;
}

void OS_Vita::set_main_loop(MainLoop *p_main_loop) {

	main_loop = p_main_loop;
	//input->set_main_loop(p_main_loop);
}

bool OS_Vita::_check_internal_feature_support(const String &p_feature) {
	if (p_feature == "mobile") {
		//TODO support etc2 only if GLES3 driver is selected
		return true;
	}
	return false;
}

/*
	SceInt32 sdkVersion;
	SceChar8 audioPath[0x80];           //Path to audio file that will be played during dialog, .mp3, .at9, m4a. Can be NULL
	SceChar8 titleid[0x10];             //TitleId of the application to open when "accept" button has been pressed. Can be NULL
	SceInt32 unk_BC;                    //Can be set to 0
	SceUInt32 dialogTimer;              //Time to show dialog in seconds
	SceChar8 reserved1[0x3E];
	SceWChar16 buttonRightText[0x1F];   //Text for "accept" button
	SceInt16 separator0;                //must be 0
	SceWChar16 buttonLeftText[0x1F];    //Text for "reject" button. If NULL, only "accept" button will be created
	SceInt16 separator1;                //must be 0
	SceWChar16 dialogText[0x80];        //Text for dialog window, also shared with notification
	SceInt16 separator2;                //must be 0
*/

#include <locale>

char16_t* str16cpy(char16_t* destination, const char16_t* source)
{
    char16_t* temp = destination;
    while((*temp++ = *source++) != 0)
    ;
    return destination;
}

void utf8_to_utf16(uint8_t *src, uint16_t *dst) {
	int i;
	for (i = 0; src[i];) {
		if ((src[i] & 0xE0) == 0xE0) {
			*(dst++) = ((src[i] & 0x0F) << 12) | ((src[i + 1] & 0x3F) << 6) | (src[i + 2] & 0x3F);
			i += 3;
		} else if ((src[i] & 0xC0) == 0xC0) {
			*(dst++) = ((src[i] & 0x1F) << 6) | (src[i + 1] & 0x3F);
			i += 2;
		} else {
			*(dst++) = src[i];
			i += 1;
		}
	}

	*dst = '\0';
}


void OS_Vita::alert(const String &p_alert, const String &p_title)
{
	return;
}

Point2 OS_Vita::get_mouse_position() const
{
	return Point2(0, 0);
}

int OS_Vita::get_mouse_button_state() const
{
	return 0;
}

void OS_Vita::set_window_title(const String &p_title) {
    return;
}

void OS_Vita::set_video_mode(const OS::VideoMode &p_video_mode, int p_screen) {}
OS::VideoMode OS_Vita::get_video_mode(int p_screen) const
{
	return VideoMode(960, 544);
}

void OS_Vita::get_fullscreen_mode_list(List<OS::VideoMode> *p_list, int p_screen) const {}

Size2 OS_Vita::get_window_size() const { return Size2(960, 544); }

bool OS_Vita::can_draw() const
{
	return true;
}

int OS_Vita::get_current_video_driver() const { return video_driver_index; }

OS_Vita::OS_Vita()
{
    video_driver_index = 0;
	main_loop = nullptr;
	gl_context = nullptr;
}