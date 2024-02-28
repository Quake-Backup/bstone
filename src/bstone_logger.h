/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// A logger.
//


#ifndef BSTONE_LOGGER_INCLUDED
#define BSTONE_LOGGER_INCLUDED


#include <memory>
#include <string>


namespace bstone
{


enum class LoggerMessageType
{
	information,
	warning,
	error,
}; // LoggerMessageType


//
// Logger interface.
//
// Writes messages to standard output, file and
// shows message box on critical error.
//
class Logger
{
public:
	Logger() noexcept = default;

	virtual ~Logger() = default;


	// Writes a message of the specified type.
	virtual void write(
		LoggerMessageType message_type,
		const std::string& message) noexcept = 0;

	// Write a new line.
	virtual void write() noexcept = 0;

	// Writes an informational message.
	virtual void write(
		const std::string& message) noexcept = 0;

	// Writes a warning message.
	virtual void write_warning(
		const std::string& message) noexcept = 0;

	// Writes an error message.
	virtual void write_error(
		const std::string& message) noexcept = 0;
}; // Logger

using LoggerPtr = Logger*;
using LoggerUPtr = std::unique_ptr<Logger>;


struct LoggerFactory
{
	LoggerUPtr create();
}; // LoggerFactory


extern LoggerPtr logger_;

void log_exception() noexcept;


} // bstone


#endif // !BSTONE_LOGGER_INCLUDED
