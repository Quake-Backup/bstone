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
// OpenGL buffer object (implementation).
//
// !!! Internal usage only !!!
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_buffer.h"

#include "bstone_exception.h"
#include "bstone_unique_resource.h"

#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_detail_ogl_state.h"


namespace bstone
{
namespace detail
{


// =========================================================================
// OglBuffer
//

OglBuffer::OglBuffer() = default;

OglBuffer::~OglBuffer() = default;

//
// OglBuffer
// =========================================================================


// =========================================================================
// GenericOglBuffer
//

class GenericOglBuffer :
	public OglBuffer
{
public:
	GenericOglBuffer(
		const OglStatePtr ogl_state,
		const OglBufferFactory::InitializeParam& param);

	~GenericOglBuffer() override;


	RendererBufferKind get_kind() const noexcept override;

	RendererBufferUsageKind get_usage_kind() const noexcept override;

	int get_size() const noexcept override;

	void bind(
		const bool is_bind) override;

	void update(
		const RendererBufferUpdateParam& param) override;


	OglStatePtr ogl_state_get() const noexcept override;


private:
	static void resource_deleter(
		const GLuint& ogl_name) noexcept;

	using BufferResource = UniqueResource<GLuint, resource_deleter>;

	RendererBufferKind kind_;
	RendererBufferUsageKind usage_kind_;
	int size_;
	BufferResource ogl_resource_;
	GLenum ogl_target_;
	const OglStatePtr ogl_state_;


	void validate_param(
		const OglBufferFactory::InitializeParam& param);

	void validate_param(
		const RendererBufferUpdateParam& param);

	static GLenum ogl_get_target(
		const RendererBufferKind kind);

	static GLenum ogl_get_usage(
		const RendererBufferUsageKind usage_kind);

	void initialize(
		const OglBufferFactory::InitializeParam& param);

	void uninitialize();
}; // GenericOglBuffer


using GenericOglBufferUPtr = std::unique_ptr<GenericOglBuffer>;

//
// GenericOglBuffer
// =========================================================================


// =========================================================================
// GenericOglBuffer
//

GenericOglBuffer::GenericOglBuffer(
	const OglStatePtr ogl_state,
	const OglBufferFactory::InitializeParam& param)
	:
	kind_{},
	usage_kind_{},
	size_{},
	ogl_resource_{},
	ogl_target_{},
	ogl_state_{ogl_state}
{
	initialize(param);
}

GenericOglBuffer::~GenericOglBuffer()
{
	uninitialize();
}

RendererBufferKind GenericOglBuffer::get_kind() const noexcept
{
	return kind_;
}

RendererBufferUsageKind GenericOglBuffer::get_usage_kind() const noexcept
{
	return usage_kind_;
}

int GenericOglBuffer::get_size() const noexcept
{
	return size_;
}

void GenericOglBuffer::bind(
	const bool is_bind)
{
	const auto ogl_buffer = (is_bind ? this : nullptr);
	const auto ogl_resource = (is_bind ? ogl_resource_.get() : 0);

	if (ogl_state_->buffer_set_current(kind_, ogl_buffer))
	{
		::glBindBuffer(ogl_target_, ogl_resource);
		assert(!detail::OglRendererUtils::was_errors());
	}
}

void GenericOglBuffer::update(
	const RendererBufferUpdateParam& param)
{
	validate_param(param);

	if (param.size_ == 0)
	{
		return;
	}

	const auto is_index = (kind_ == RendererBufferKind::index);

	if (is_index)
	{
		ogl_state_->vao_push_current_set_default();
	}

	bind(this);

	::glBufferSubData(
		ogl_target_,
		param.offset_,
		param.size_,
		param.data_
	);

	assert(!detail::OglRendererUtils::was_errors());

	if (is_index)
	{
		ogl_state_->vao_pop();
	}
}

OglStatePtr GenericOglBuffer::ogl_state_get() const noexcept
{
	return ogl_state_;
}

void GenericOglBuffer::resource_deleter(
	const GLuint& ogl_name) noexcept
{
	::glDeleteBuffers(1, &ogl_name);
	assert(!detail::OglRendererUtils::was_errors());
}

void GenericOglBuffer::initialize(
	const OglBufferFactory::InitializeParam& param)
{
	if (!ogl_state_)
	{
		throw Exception{"Null OpenGL state."};
	}

	validate_param(param);

	auto ogl_name = GLuint{};
	::glGenBuffers(1, &ogl_name);
	assert(!detail::OglRendererUtils::was_errors());

	if (ogl_name == 0)
	{
		throw Exception{"Failed to create OpenGL buffer object."};
	}

	kind_ = param.kind_;
	usage_kind_ = param.usage_kind_;
	size_ = param.size_;
	ogl_resource_.reset(ogl_name);
	ogl_target_ = ogl_get_target(param.kind_);

	const auto is_index = (kind_ == RendererBufferKind::index);

	if (is_index)
	{
		ogl_state_->vao_push_current_set_default();
	}

	bind(true);

	const auto olg_usage = ogl_get_usage(param.usage_kind_);

	::glBufferData(ogl_target_, param.size_, nullptr, olg_usage);
	assert(!detail::OglRendererUtils::was_errors());

	if (is_index)
	{
		ogl_state_->vao_pop();
	}
}

void GenericOglBuffer::validate_param(
	const OglBufferFactory::InitializeParam& param)
{
	switch (param.kind_)
	{
		case RendererBufferKind::index:
		case RendererBufferKind::vertex:
			break;

		default:
			throw Exception{"Invalid kind."};
	}

	switch (param.usage_kind_)
	{
		case RendererBufferUsageKind::stream_draw:
		case RendererBufferUsageKind::static_draw:
		case RendererBufferUsageKind::dynamic_draw:
			break;

		default:
			throw Exception{"Invalid usage kind."};
	}

	if (param.size_ <= 0)
	{
		throw Exception{"Non-positive size."};
	}
}

void GenericOglBuffer::validate_param(
	const RendererBufferUpdateParam& param)
{
	if (param.offset_ < 0)
	{
		throw Exception{"Negative offset."};
	}

	if (param.size_ < 0)
	{
		throw Exception{"Negative size."};
	}

	if (param.offset_ > size_)
	{
		throw Exception{"Offset out of range."};
	}

	if (param.size_ > size_)
	{
		throw Exception{"Size out of range."};
	}

	if ((param.offset_ + param.size_) > size_)
	{
		throw Exception{"End offset out of range."};
	}

	if (param.size_ > 0 && param.data_ == nullptr)
	{
		throw Exception{"Null data."};
	}
}

GLenum GenericOglBuffer::ogl_get_target(
	const RendererBufferKind kind)
{
	switch (kind)
	{
		case RendererBufferKind::index:
			return GL_ELEMENT_ARRAY_BUFFER;

		case RendererBufferKind::vertex:
			return GL_ARRAY_BUFFER;

		default:
			throw Exception{"Unsupported buffer kind."};
	}
}

GLenum GenericOglBuffer::ogl_get_usage(
	const RendererBufferUsageKind usage_kind)
{
	switch (usage_kind)
	{
		case RendererBufferUsageKind::stream_draw:
			return GL_STREAM_DRAW;

		case RendererBufferUsageKind::static_draw:
			return GL_STATIC_DRAW;

		case RendererBufferUsageKind::dynamic_draw:
			return GL_DYNAMIC_DRAW;

		default:
			throw Exception{"Unsupported usage kind."};
	}
}

void GenericOglBuffer::uninitialize()
{
	if (ogl_resource_)
	{
		bind(false);
	}

	kind_ = {};
	usage_kind_ = {};
	size_ = 0;
	ogl_resource_.reset();
	ogl_target_ = {};
}

//
// GenericOglBuffer
// =========================================================================


// =========================================================================
// OglBufferFactory
//

OglBufferUPtr OglBufferFactory::create(
	const OglStatePtr ogl_state,
	const OglBufferFactory::InitializeParam& param)
{
	return std::make_unique<GenericOglBuffer>(ogl_state, param);
}

//
// OglBufferFactory
// =========================================================================


} // detail
} // bstone
