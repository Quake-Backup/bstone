/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2019 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


//
// OpenGL Renderer utils.
//
// !!! Internal usage only. !!!
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_renderer_utils.h"
#include <cassert>
#include <algorithm>
#include <iterator>
#include <limits>
#include <sstream>
#include "SDL_video.h"
#include "glm/gtc/matrix_transform.hpp"
#include "bstone_exception.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// OglRendererUtils
//

void OglRendererUtils::load_library()
{
	const auto error_message_prefix = "Failed to load default OpenGL library. ";

	const auto sdl_result = ::SDL_GL_LoadLibrary(nullptr);

	if (sdl_result < 0)
	{
		auto error_message = std::string{error_message_prefix};
		error_message += ::SDL_GetError();

		throw Exception{std::move(error_message)};
	}
}

void OglRendererUtils::unload_library()
{
	::SDL_GL_UnloadLibrary();
}

void* OglRendererUtils::resolve_symbol(
	const char* const symbol)
{
	return ::SDL_GL_GetProcAddress(symbol);
}

SdlGlContextUPtr OglRendererUtils::create_context(
	SdlWindowPtr sdl_window)
{
	const auto error_message_prefix = "Failed to create OpenGL context. ";

	if (!sdl_window)
	{
		auto error_message = std::string{error_message_prefix};
		error_message += "Null SDL window.";

		throw Exception{std::move(error_message)};
	}

	auto sdl_gl_context = SdlGlContextUPtr{::SDL_GL_CreateContext(sdl_window)};

	if (!sdl_gl_context)
	{
		auto error_message = std::string{error_message_prefix};
		error_message += ::SDL_GetError();

		throw Exception{std::move(error_message)};
	}

	return sdl_gl_context;
}

void OglRendererUtils::make_context_current(
	SdlWindowPtr sdl_window,
	SdlGlContextPtr sdl_gl_context)
{
	const auto error_message_prefix = "Failed to make a context current. ";

	if (!sdl_window)
	{
		auto error_message = std::string{error_message_prefix};
		error_message += "Null SDL window.";

		throw Exception{std::move(error_message)};
	}

	const auto sdl_result = ::SDL_GL_MakeCurrent(sdl_window, sdl_gl_context);

	if (sdl_result < 0)
	{
		auto error_message = std::string{error_message_prefix};
		error_message += ::SDL_GetError();

		throw Exception{std::move(error_message)};
	}
}

void OglRendererUtils::create_window_and_context(
	const RendererUtilsCreateWindowParam& param,
	SdlWindowUPtr& sdl_window,
	SdlGlContextUPtr& sdl_gl_context)
{
	auto sdl_window_result = RendererUtils::create_window(param);
	auto sdl_gl_context_result = create_context(sdl_window_result.get());

	sdl_window = std::move(sdl_window_result);
	sdl_gl_context = std::move(sdl_gl_context_result);
}

void OglRendererUtils::create_probe_window_and_context(
	SdlWindowUPtr& sdl_window,
	SdlGlContextUPtr& sdl_gl_context)
{
	auto param = RendererUtilsCreateWindowParam{};
	param.is_opengl_ = true;
	param.window_.width_ = 1;
	param.window_.height_ = 1;
	param.aa_kind_ = RendererAaKind::none;
	param.aa_value_ = 0;

	create_window_and_context(param, sdl_window, sdl_gl_context);
}

void OglRendererUtils::window_get_drawable_size(
	SdlWindowPtr sdl_window,
	int& width,
	int& height)
{
	width = 0;
	height = 0;

	if (!sdl_window)
	{
		throw Exception{"Null window."};
	}

	::SDL_GL_GetDrawableSize(sdl_window, &width, &height);

	if (width <= 0 || height <= 0)
	{
		throw Exception{std::move(std::string{::SDL_GetError()})};
	}
}

OglContextKind OglRendererUtils::context_get_kind()
{
	const auto sdl_ogl_context = ::SDL_GL_GetCurrentContext();

	if (sdl_ogl_context == nullptr)
	{
		return OglContextKind::invalid;
	}

	auto sdl_attribute = 0;

	const auto sdl_result = ::SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &sdl_attribute);

	if (sdl_result != 0)
	{
		return OglContextKind::invalid;
	}

	switch (sdl_attribute)
	{
		case 0:
			return OglContextKind::none;

		case SDL_GL_CONTEXT_PROFILE_CORE:
			return OglContextKind::core;

		case SDL_GL_CONTEXT_PROFILE_COMPATIBILITY:
			return OglContextKind::compatibility;

		case SDL_GL_CONTEXT_PROFILE_ES:
			return OglContextKind::es;

		default:
			return OglContextKind::invalid;
	}
}

void OglRendererUtils::context_get_version(
	int& major_version,
	int& minor_version)
{
	major_version = 0;
	minor_version = 0;

	const auto sdl_ogl_context = ::SDL_GL_GetCurrentContext();

	if (sdl_ogl_context == nullptr)
	{
		throw Exception{"No current context."};
	}

	auto sdl_result = 0;

	// Major.
	//
	auto sdl_major_version = 0;

	sdl_result = ::SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &sdl_major_version);

	if (sdl_result != 0)
	{
		auto error_message = std::string{"Failed to get major version. "};
		error_message += ::SDL_GetError();

		throw Exception{std::move(error_message)};
	}

	// Minor.
	//
	auto sdl_minor_version = 0;

	sdl_result = ::SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &sdl_minor_version);

	if (sdl_result != 0)
	{
		auto error_message = std::string{"Failed to get minor version. "};
		error_message += ::SDL_GetError();

		throw Exception{std::move(error_message)};
	}

	// Result.
	//
	major_version = sdl_major_version;
	minor_version = sdl_minor_version;
}

int OglRendererUtils::anisotropy_clamp_value(
	const int anisotropy_value,
	const RendererDeviceFeatures& device_features)
{
	auto clamped_value = anisotropy_value;

	if (clamped_value < RendererSampler::anisotropy_min)
	{
		clamped_value = RendererSampler::anisotropy_min;
	}
	else if (clamped_value > device_features.anisotropy_max_value_)
	{
		clamped_value = device_features.anisotropy_max_value_;
	}

	return clamped_value;
}

int OglRendererUtils::anisotropy_get_max_value()
{
	auto ogl_max_value = GLfloat{};

	::glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &ogl_max_value);

	assert(!OglRendererUtils::was_errors());

	if (ogl_max_value < static_cast<GLfloat>(RendererSampler::anisotropy_min))
	{
		return RendererSampler::anisotropy_min;
	}

	return static_cast<int>(ogl_max_value);
}

void OglRendererUtils::anisotropy_set_value(
	const GLenum ogl_target,
	const RendererDeviceFeatures& device_features,
	const int anisotropy_value)
{
	if (!device_features.anisotropy_is_available_)
	{
		return;
	}

	const auto clamped_value = anisotropy_clamp_value(anisotropy_value, device_features);

	const auto ogl_value = static_cast<GLfloat>(clamped_value);

	::glTexParameterf(ogl_target, GL_TEXTURE_MAX_ANISOTROPY, ogl_value);
	assert(!OglRendererUtils::was_errors());
}

void OglRendererUtils::anisotropy_probe(
	OglExtensionManagerPtr extension_manager,
	RendererDeviceFeatures& device_features)
{
	device_features.anisotropy_is_available_ = false;

	if (!device_features.anisotropy_is_available_)
	{
		extension_manager->probe_extension(OglExtensionId::arb_texture_filter_anisotropic);

		device_features.anisotropy_is_available_ =
			extension_manager->has_extension(OglExtensionId::arb_texture_filter_anisotropic);
	}

	if (!device_features.anisotropy_is_available_)
	{
		extension_manager->probe_extension(OglExtensionId::ext_texture_filter_anisotropic);

		device_features.anisotropy_is_available_ =
			extension_manager->has_extension(OglExtensionId::ext_texture_filter_anisotropic);
	}

	if (device_features.anisotropy_is_available_)
	{
		device_features.anisotropy_min_value_ = RendererSampler::anisotropy_min;
		device_features.anisotropy_max_value_ = anisotropy_get_max_value();
	}
}

void OglRendererUtils::npot_probe(
	OglExtensionManagerPtr extension_manager,
	RendererDeviceFeatures& device_features)
{
	device_features.npot_is_available_ = false;

	if (!device_features.npot_is_available_)
	{
		extension_manager->probe_extension(OglExtensionId::arb_texture_non_power_of_two);

		device_features.npot_is_available_ =
			extension_manager->has_extension(OglExtensionId::arb_texture_non_power_of_two);
	}
}

void OglRendererUtils::mipmap_probe(
	OglExtensionManagerPtr extension_manager,
	RendererDeviceFeatures& device_features,
	OglDeviceFeatures& ogl_device_features)
{
	device_features.mipmap_is_available_ = false;
	ogl_device_features.mipmap_function_ = nullptr;

	if (!device_features.mipmap_is_available_)
	{
		extension_manager->probe_extension(OglExtensionId::arb_framebuffer_object);

		if (extension_manager->has_extension(OglExtensionId::arb_framebuffer_object))
		{
			device_features.mipmap_is_available_ = true;
			ogl_device_features.mipmap_function_ = ::glGenerateMipmap;
		}
	}

	if (!device_features.mipmap_is_available_)
	{
		extension_manager->probe_extension(OglExtensionId::ext_framebuffer_object);

		if (extension_manager->has_extension(OglExtensionId::ext_framebuffer_object))
		{
			device_features.mipmap_is_available_ = true;
			ogl_device_features.mipmap_function_ = ::glGenerateMipmapEXT;
		}
	}
}

void OglRendererUtils::framebuffer_probe(
	OglExtensionManagerPtr extension_manager,
	RendererDeviceFeatures& device_features,
	OglDeviceFeatures& ogl_device_features)
{
	auto is_arb = false;
	auto is_available = false;

	if (!is_available)
	{
		extension_manager->probe_extension(OglExtensionId::arb_framebuffer_object);

		is_available = extension_manager->has_extension(OglExtensionId::arb_framebuffer_object);

		if (is_available)
		{
			is_arb = true;
		}
	}

	if (!is_available)
	{
		extension_manager->probe_extension(OglExtensionId::ext_framebuffer_blit);
		extension_manager->probe_extension(OglExtensionId::ext_framebuffer_multisample);
		extension_manager->probe_extension(OglExtensionId::ext_framebuffer_object);
		extension_manager->probe_extension(OglExtensionId::ext_packed_depth_stencil);

		is_available =
			extension_manager->has_extension(OglExtensionId::ext_framebuffer_blit) &&
			extension_manager->has_extension(OglExtensionId::ext_framebuffer_multisample) &&
			extension_manager->has_extension(OglExtensionId::ext_framebuffer_object) &&
			extension_manager->has_extension(OglExtensionId::ext_packed_depth_stencil)
		;
	}

	device_features.msaa_min_value_ = RendererUtils::aa_get_min_value();
	device_features.msaa_max_value_ = msaa_get_max_value(extension_manager);

	if (device_features.msaa_min_value_ == device_features.msaa_max_value_)
	{
		is_available = false;
	}

	device_features.framebuffer_is_available_ = is_available;

	if (is_available)
	{
		ogl_device_features.framebuffer_is_arb_ = is_arb;
	}
}

int OglRendererUtils::msaa_get_max_value(
	OglExtensionManagerPtr extension_manager)
{
	auto max_value = GLint{};

	if (extension_manager->has_extension(OglExtensionId::arb_framebuffer_object))
	{
		::glGetIntegerv(GL_MAX_SAMPLES, &max_value);
		assert(!OglRendererUtils::was_errors());
	}
	else if (
		extension_manager->has_extension(OglExtensionId::ext_framebuffer_blit) &&
		extension_manager->has_extension(OglExtensionId::ext_framebuffer_multisample) &&
		extension_manager->has_extension(OglExtensionId::ext_framebuffer_object) &&
		extension_manager->has_extension(OglExtensionId::ext_packed_depth_stencil))
	{
		::glGetIntegerv(GL_MAX_SAMPLES_EXT, &max_value);
		assert(!OglRendererUtils::was_errors());
	}

	return std::max(max_value, RendererUtils::aa_get_min_value());
}

void OglRendererUtils::sampler_probe(
	OglExtensionManagerPtr extension_manager,
	RendererDeviceFeatures& device_features)
{
	return;
	extension_manager->probe_extension(OglExtensionId::arb_sampler_objects);

	device_features.sampler_is_available_ =
		extension_manager->has_extension(OglExtensionId::arb_sampler_objects);
}

void OglRendererUtils::sampler_set_anisotropy(
	const GLenum ogl_sampler,
	const RendererDeviceFeatures& device_features,
	const int anisotropy_value)
{
	if (!device_features.sampler_is_available_ ||
		!device_features.anisotropy_is_available_)
	{
		return;
	}

	const auto clamped_value = anisotropy_clamp_value(anisotropy_value, device_features);

	const auto ogl_value = static_cast<GLfloat>(clamped_value);

	::glSamplerParameterf(ogl_sampler, GL_TEXTURE_MAX_ANISOTROPY, ogl_value);
	assert(!OglRendererUtils::was_errors());
}

void OglRendererUtils::vertex_input_vao_probe(
	OglExtensionManagerPtr extension_manager,
	OglDeviceFeatures& ogl_device_features)
{
	extension_manager->probe_extension(OglExtensionId::arb_vertex_array_object);

	ogl_device_features.vao_is_available_ =
		extension_manager->has_extension(OglExtensionId::arb_vertex_array_object);
}

void OglRendererUtils::vertex_input_probe_max_locations(
	RendererDeviceFeatures& device_features)
{
	auto ogl_count = GLint{};

	::glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &ogl_count);
	assert(!OglRendererUtils::was_errors());

	device_features.vertex_input_max_locations_ = 0;

	if (ogl_count > 0)
	{
		device_features.vertex_input_max_locations_ = ogl_count;
	}
}

void OglRendererUtils::vsync_probe(
	RendererDeviceFeatures& device_features)
{
	device_features.vsync_is_available_ = false;
	device_features.vsync_is_requires_restart_ = false;

	if (vsync_set(true))
	{
		device_features.vsync_is_available_ = true;
	}
}

bool OglRendererUtils::vsync_get()
{
	const auto sdl_result = ::SDL_GL_GetSwapInterval();

	return sdl_result > 0;
}

bool OglRendererUtils::vsync_set(
	const bool is_enabled)
{
	const auto sdl_result = ::SDL_GL_SetSwapInterval(is_enabled);

	return sdl_result == 0;
}

void OglRendererUtils::clear_buffers()
{
	assert(::glClear != nullptr);

	::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	assert(!OglRendererUtils::was_errors());
}

void OglRendererUtils::swap_window(
	SdlWindowPtr sdl_window)
{
	assert(sdl_window != nullptr);

	::SDL_GL_SwapWindow(sdl_window);
}

bool OglRendererUtils::was_errors()
{
	assert(::glGetError != nullptr);

	const auto max_error_count = 32;

	auto was_error = false;

	for (int i = 0; i < max_error_count; ++i)
	{
		const auto last_error = ::glGetError();

		if (last_error == GL_NO_ERROR)
		{
			break;
		}

		was_error = true;
	}

	return was_error;
}

void OglRendererUtils::clear_errors()
{
	static_cast<void>(was_errors());
}

void OglRendererUtils::set_color_buffer_clear_color(
	const R8g8b8a8& color)
{
	assert(::glClearColor != nullptr);

	const auto reciprocal_255 = 1.0F / 255.0F;

	::glClearColor(
		static_cast<float>(color.r) * reciprocal_255,
		static_cast<float>(color.g) * reciprocal_255,
		static_cast<float>(color.b) * reciprocal_255,
		static_cast<float>(color.a) * reciprocal_255
	);

	assert(!OglRendererUtils::was_errors());
}

void OglRendererUtils::scissor_enable(
	const bool is_enabled)
{
	if (is_enabled)
	{
		::glEnable(GL_SCISSOR_TEST);
		assert(!OglRendererUtils::was_errors());
	}
	else
	{
		::glDisable(GL_SCISSOR_TEST);
		assert(!OglRendererUtils::was_errors());
	}
}

void OglRendererUtils::scissor_set_box(
	const int x,
	const int y,
	const int width,
	const int height)
{
	assert(x >= 0);
	assert(y >= 0);
	assert(width > 0);
	assert(height > 0);

	::glScissor(x, y, width, height);
	assert(!OglRendererUtils::was_errors());
}

void OglRendererUtils::viewport_set_rectangle(
	const int x,
	const int y,
	const int width,
	const int height)
{
	assert(x >= 0);
	assert(y >= 0);
	assert(width > 0);
	assert(height > 0);

	::glViewport(x, y, width, height);
	assert(!OglRendererUtils::was_errors());
}

void OglRendererUtils::viewport_set_depth_range(
	const float min_depth,
	const float max_depth)
{
	assert(min_depth >= 0.0F && min_depth <= 1.0F);
	assert(max_depth >= 0.0F && max_depth <= 1.0F);

	::glDepthRange(min_depth, max_depth);
	assert(!OglRendererUtils::was_errors());
}

void OglRendererUtils::texture_2d_set(
	const GLuint ogl_texture_name)
{
	::glBindTexture(GL_TEXTURE_2D, ogl_texture_name);
	assert(!OglRendererUtils::was_errors());
}

void OglRendererUtils::texture_2d_unbind()
{
	::glBindTexture(GL_TEXTURE_2D, 0);
	assert(!OglRendererUtils::was_errors());
}

void OglRendererUtils::blending_set_function(
	const RendererBlendingFactor src_factor,
	const RendererBlendingFactor dst_factor)
{
	auto ogl_src_factor = blending_get_factor(src_factor);
	auto ogl_dst_factor = blending_get_factor(dst_factor);

	::glBlendFunc(ogl_src_factor, ogl_dst_factor);
	assert(!OglRendererUtils::was_errors());
}

GLenum OglRendererUtils::index_buffer_get_element_type_by_byte_depth(
	const int byte_depth)
{
	switch (byte_depth)
	{
	case 1:
		return GL_UNSIGNED_BYTE;

	case 2:
		return GL_UNSIGNED_SHORT;

	case 4:
		return GL_UNSIGNED_INT;

	default:
		assert(!"Invalid byte depth.");

		return GL_NONE;
	}
}

void OglRendererUtils::renderer_features_set(
	RendererDeviceFeatures& device_features)
{
	// Max texture dimension.
	//
	auto ogl_texture_dimension = GLint{};

	::glGetIntegerv(GL_MAX_TEXTURE_SIZE, &ogl_texture_dimension);

	if (OglRendererUtils::was_errors() || ogl_texture_dimension == 0)
	{
		throw Exception{"Failed to get maximum texture dimension."};
	}


	// Max viewport dimensions.
	//
	using OglViewportDimensions = std::array<GLint, 2>;
	auto ogl_viewport_dimensions = OglViewportDimensions{};

	::glGetIntegerv(GL_MAX_VIEWPORT_DIMS, ogl_viewport_dimensions.data());

	if (OglRendererUtils::was_errors() ||
		ogl_viewport_dimensions[0] == 0 || ogl_viewport_dimensions[1] == 0)
	{
		throw Exception{"Failed to get viewport dimensions."};
	}


	// Set the values.
	//
	device_features.max_texture_dimension_ = ogl_texture_dimension;

	device_features.max_viewport_width_ = ogl_viewport_dimensions[0];
	device_features.max_viewport_height_ = ogl_viewport_dimensions[1];
}

RendererDeviceInfo OglRendererUtils::device_info_get()
{
	auto result = RendererDeviceInfo{};


	// Name.
	//
	const auto ogl_name = reinterpret_cast<const char*>(::glGetString(GL_RENDERER));

	assert(!OglRendererUtils::was_errors());

	if (ogl_name != nullptr)
	{
		result.name_ = ogl_name;
	}


	// Vendor.
	//
	const auto ogl_vendor = reinterpret_cast<const char*>(::glGetString(GL_VENDOR));

	assert(!OglRendererUtils::was_errors());

	if (ogl_vendor != nullptr)
	{
		result.vendor_ = ogl_vendor;
	}


	// Version.
	//
	const auto ogl_version = reinterpret_cast<const char*>(::glGetString(GL_VERSION));

	assert(!OglRendererUtils::was_errors());

	if (ogl_version != nullptr)
	{
		result.version_ = ogl_version;
	}


	// Result.
	//
	return result;
}

std::string OglRendererUtils::get_log(
	const bool is_shader,
	const GLuint ogl_name)
{
	const auto ogl_info_function = (is_shader ? ::glGetShaderInfoLog : ::glGetProgramInfoLog);
	const auto ogl_size_function = (is_shader ? ::glGetShaderiv : ::glGetProgramiv);

	auto size = GLint{};

	ogl_size_function(ogl_name, GL_INFO_LOG_LENGTH, &size);
	assert(!OglRendererUtils::was_errors());

	auto result = std::string{};

	if (size > 0)
	{
		result.resize(size);

		auto info_size = GLsizei{};

		ogl_info_function(ogl_name, size, &info_size, &result[0]);
		assert(!OglRendererUtils::was_errors());

		if (info_size <= 0)
		{
			result.clear();
		}
	}

	return result;
}

const glm::mat4& OglRendererUtils::csc_get_texture()
{
	constexpr auto m_11 = 1.0F;

	// Flip sign to allow to upload top-to-bottom texture images.
	constexpr auto m_22 = -1.0F;
	constexpr auto m_24 = 1.0F;

	constexpr auto m_33 = 1.0F;

	static const auto result = glm::mat4
	{
		m_11, 0.0F, 0.0F, 0.0F,
		0.0F, m_22, 0.0F, m_24,
		0.0F, 0.0F, m_33, 0.0F,
		0.0F, 0.0F, 0.0F, 1.0F,
	};

	return result;
}

const glm::mat4& OglRendererUtils::csc_get_projection()
{
	static const auto result = glm::identity<glm::mat4>();

	return result;
}

GLenum OglRendererUtils::filter_get_mag(
	const RendererFilterKind mag_filter)
{
	switch (mag_filter)
	{
	case RendererFilterKind::nearest:
		return GL_NEAREST;

	case RendererFilterKind::linear:
		return GL_LINEAR;

	default:
		throw Exception{"Unsupported magnification filter."};
	}
}

GLenum OglRendererUtils::filter_get_min(
	const RendererFilterKind min_filter,
	const RendererMipmapMode mipmap_mode)
{
	switch (mipmap_mode)
	{
	case RendererMipmapMode::none:
		switch (min_filter)
		{
		case RendererFilterKind::nearest:
			return GL_NEAREST;

		case RendererFilterKind::linear:
			return GL_LINEAR;

		default:
			throw Exception{"Unsupported minification filter."};
		}

		break;

	case RendererMipmapMode::nearest:
		switch (min_filter)
		{
		case RendererFilterKind::nearest:
			return GL_NEAREST_MIPMAP_NEAREST;

		case RendererFilterKind::linear:
			return GL_LINEAR_MIPMAP_NEAREST;

		default:
			throw Exception{"Unsupported minification mipmap filter."};
		}

		break;

	case RendererMipmapMode::linear:
		switch (min_filter)
		{
		case RendererFilterKind::nearest:
			return GL_NEAREST_MIPMAP_LINEAR;

		case RendererFilterKind::linear:
			return GL_LINEAR_MIPMAP_LINEAR;

		default:
			throw Exception{"Unsupported minification mipmap filter."};
		}

		break;

	default:
		throw Exception{"Unsupported mipmap mode."};
	}
}

GLenum OglRendererUtils::address_mode_get(
	const RendererAddressMode address_mode)
{
	switch (address_mode)
	{
	case RendererAddressMode::clamp:
		return GL_CLAMP_TO_BORDER;

	case RendererAddressMode::repeat:
		return GL_REPEAT;

	default:
		throw Exception{"Unsupported address mode."};
	}
}

GLenum OglRendererUtils::texture_wrap_get_axis(
	const RendererTextureAxis texture_axis)
{
	switch (texture_axis)
	{
		case RendererTextureAxis::u:
			return GL_TEXTURE_WRAP_S;

		case RendererTextureAxis::v:
			return GL_TEXTURE_WRAP_T;

		default:
			throw Exception{"Unsupported texture axis."};
	}
}

GLenum OglRendererUtils::blending_get_factor(
	const RendererBlendingFactor factor)
{
	switch (factor)
	{
		case RendererBlendingFactor::zero:
			return GL_ZERO;

		case RendererBlendingFactor::one:
			return GL_ONE;

		case RendererBlendingFactor::src_color:
			return GL_SRC_COLOR;

		case RendererBlendingFactor::src_alpha:
			return GL_SRC_ALPHA;

		case RendererBlendingFactor::one_minus_src_alpha:
			return GL_ONE_MINUS_SRC_ALPHA;

		default:
			return GL_NONE;
	}
}

//
// OglRendererUtils
// ==========================================================================


} // detail
} // bstone
