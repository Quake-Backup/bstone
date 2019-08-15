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
// OpenGL vertex array object manager (implementation interface).
//


#ifndef BSTONE_OGL_VAO_MANAGER_INCLUDED
#define BSTONE_OGL_VAO_MANAGER_INCLUDED


#include <memory>


namespace bstone
{


struct RendererDeviceFeatures;

class RendererBuffer;
using RendererBufferPtr = RendererBuffer*;


namespace detail
{


class OglState;
using OglStatePtr = OglState*;

struct OglDeviceFeatures;

class OglVao;
using OglVaoPtr = OglVao*;


// ==========================================================================
// OglVaoManager
//

class OglVaoManager
{
protected:
	OglVaoManager();


public:
	virtual ~OglVaoManager();


	virtual const RendererDeviceFeatures& get_device_features() const noexcept = 0;

	virtual const OglDeviceFeatures& get_ogl_device_features() const noexcept = 0;


	virtual OglVaoPtr create() = 0;

	virtual void destroy(
		const OglVaoPtr vao) = 0;

	virtual void push_current_set_default() = 0;

	virtual void pop() = 0;

	virtual void bind(
		const OglVaoPtr vao) = 0;

	virtual bool index_buffer_set_current(
		const RendererBufferPtr index_buffer) = 0;


	virtual void enable_location(
		const int location,
		const bool is_enable) = 0;
}; // OglVaoManager

using OglVaoManagerPtr = OglVaoManager*;
using OglVaoManagerUPtr = std::unique_ptr<OglVaoManager>;

//
// OglVaoManager
// ==========================================================================


// ==========================================================================
// OglVaoManagerFactory
//

struct OglVaoManagerFactory final
{
	static OglVaoManagerUPtr create(
		const OglStatePtr ogl_state,
		const RendererDeviceFeatures& device_features,
		const OglDeviceFeatures& ogl_device_features);
}; // OglVaoManagerFactory

//
// OglVaoManagerFactory
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_OGL_VAO_MANAGER_INCLUDED
