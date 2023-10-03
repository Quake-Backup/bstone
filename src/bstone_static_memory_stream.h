/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Non-resizeable read-write memory stream.

#if !defined(BSTONE_STATIC_MEMORY_STREAM_INCLUDED)
#define BSTONE_STATIC_MEMORY_STREAM_INCLUDED

#include "bstone_int.h"
#include "bstone_stream.h"

namespace bstone {

class StaticMemoryStream final : public Stream
{
public:
	StaticMemoryStream() = default;
	StaticMemoryStream(void* buffer, IntP size);
	~StaticMemoryStream() override = default;

	const UInt8* get_data() const;
	UInt8* get_data();

	void open(void* buffer, IntP size);

private:
	bool is_open_{};
	UInt8* buffer_{};
	Int64 size_{};
	IntP capacity_{};
	Int64 position_{};

private:
	void do_close() override;
	bool do_is_open() const override;
	IntP do_read(void* buffer, IntP count) override;
	IntP do_write(const void* buffer, IntP count) override;
	Int64 do_seek(Int64 offset, StreamOrigin origin) override;
	Int64 do_get_size() const override;
	void do_set_size(Int64 size) override;
	void do_flush() override;

private:
	void ensure_is_open() const;
	void close_internal() noexcept;
};

} // namespace bstone

#endif // BSTONE_STATIC_MEMORY_STREAM_INCLUDED
