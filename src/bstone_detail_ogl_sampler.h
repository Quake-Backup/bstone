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
// OpenGL sampler.
//
// !!! Internal usage only !!!
//


#ifndef BSTONE_DETAIL_OGL_SAMPLER_INCLUDED
#define BSTONE_DETAIL_OGL_SAMPLER_INCLUDED


#include "bstone_renderer.h"


namespace bstone
{
namespace detail
{


class OglState;
using OglStatePtr = OglState*;


// =========================================================================
// OglSampler
//

class OglSampler :
	public RendererSampler
{
protected:
	OglSampler();


public:
	~OglSampler() override;


	virtual void bind() = 0;
}; // OglSampler

using OglSamplerPtr = OglSampler*;
using OglSamplerUPtr = std::unique_ptr<OglSampler>;

//
// OglSampler
// =========================================================================


// =========================================================================
// OglSamplerFactory
//

struct OglSamplerFactory final
{
	static OglSamplerUPtr create(
		OglStatePtr ogl_state,
		const RendererSamplerCreateParam& param);
}; // OglSamplerFactory

//
// OglSamplerFactory
// =========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_SAMPLER_INCLUDED
