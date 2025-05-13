/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// A window.

#include "bstone_sys_window.h"

namespace bstone {
namespace sys {

Window::Window() = default;

Window::~Window() = default;

const char* Window::get_title()
{
	return do_get_title();
}

void Window::set_title(const char* title)
{
	do_set_title(title);
}

WindowPosition Window::get_position()
{
	return do_get_position();
}

void Window::set_position(WindowPosition position)
{
	do_set_position(position);
}

void Window::center()
{
	do_set_position(WindowPosition{WindowOffset::make_centered(), WindowOffset::make_centered()});
}

WindowSize Window::get_size()
{
	return do_get_size();
}

void Window::set_size(WindowSize size)
{
	do_set_size(size);
}

DisplayMode Window::get_display_mode()
{
	return do_get_display_mode();
}

void Window::set_display_mode(const DisplayMode& display_mode)
{
	do_set_display_mode(display_mode);
}

void Window::show(bool is_visible)
{
	do_show(is_visible);
}

void Window::set_rounded_corner_type(WindowRoundedCornerType value)
{
	do_set_rounded_corner_type(value);
}

WindowFullscreenType Window::get_fullscreen_mode()
{
	return do_get_fullscreen_mode();
}

void Window::set_fullscreen_mode(WindowFullscreenType fullscreen_mode)
{
	do_set_fullscreen_mode(fullscreen_mode);
}

GlContextUPtr Window::make_gl_context()
{
	return do_make_gl_context();
}

WindowSize Window::gl_get_drawable_size()
{
	return do_gl_get_drawable_size();
}

void Window::gl_swap_buffers()
{
	do_gl_swap_buffers();
}

RendererUPtr Window::make_renderer(const RendererInitParam& param)
{
	return do_make_renderer(param);
}

} // namespace sys
} // namespace bstone
