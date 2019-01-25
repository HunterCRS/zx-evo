#pragma once
#include "sysdefs.h"
#include "debugger/libs/view.h"
#include "debugger/views/mem.h"

class DebugCore;
class MemView;
class DebugView;

struct TRegLayout final
{
	size_t offs;
	u8 width;
	u8 x, y;
	u8 lf, rt, up, dn;
};

extern const std::vector<TRegLayout> regs_layout;

class RegView final
{
	DebugView &view_;
	MemView& mem_;
	
	auto subscrible() -> void;

	auto rcodejump() const -> void;
	auto rdatajump() const -> void;
	auto renter() const -> void;
public:
	unsigned regs_curs{};

	RegView(DebugView& view, MemView& mem);

	auto render() const -> void;
	auto dispatch() const -> char;
};
