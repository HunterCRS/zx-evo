#include "std.h"
#include "emul.h"
#include "vars.h"
#include "draw.h"
#include "dx.h"
#include "debug.h"
#include "dbgpaint.h"
#include "dbgreg.h"
#include "dbgtrace.h"
#include "dbgmem.h"
#include "dbgoth.h"
#include "dbglabls.h"
#include "dbgbpx.h"
#include "dbgcmd.h"
#include "util.h"
#include "resource.h"
#include "emulkeys.h"
#include "dbgrwdlg.h"

#ifdef MOD_MONITOR

static HMENU debug_menu;

u8 trace_labels;

GDIBMP debug_gdibmp = { { { sizeof(BITMAPINFOHEADER), DEBUG_WND_WIDTH, -DEBUG_WND_HEIGHT, 1, 8, BI_RGB, 0 } } };
u8 debug_gdibuf[DBG_GDIBUFSZ];

unsigned show_scrshot;
unsigned user_watches[3] = { 0x4000, 0x8000, 0xC000 };

unsigned mem_sz = 8;
unsigned mem_disk, mem_track, mem_max;
u8 mem_ascii;
u8 mem_dump;
u8 editor = ed_mem;

unsigned regs_curs;
unsigned dbg_extport;
u8 dgb_extval; // extended memory port like 1FFD or DFFD

unsigned ripper; // ripper mode (none/read/write)

dbgwnd activedbg = dbgwnd::trace;

void show_tsconf();
void init_tsconf();

void debugscr()
{
	memset(txtscr, backgr_ch, sizeof txtscr / 2);
	memset(txtscr + sizeof txtscr / 2, backgr, sizeof txtscr / 2);
	nfr = 0;

	show_regs();
	show_trace();
	show_mem();
	show_watch();
	show_stack();
	show_ay();
	show_banks();
	show_ports();
	show_dos();
	show_tsconf();

	show_time();
}

void handle_mouse()
{
	auto& cpu = TCpuMgr::get_cpu();

	const unsigned mx = ((mousepos & 0xFFFF) - temp.gx) / 8;
	const unsigned my = (((mousepos >> 16) & 0x7FFF) - temp.gy) / 16;

	if (my >= trace_y && my < trace_y + trace_size && mx >= trace_x && mx < trace_x + 32)
	{
		needclr++; activedbg = dbgwnd::trace;
		cpu.trace_curs = cpu.trpc[my - trace_y];
		if (mx - trace_x < cs[1][0]) cpu.trace_mode = 0;
		else if (mx - trace_x < cs[2][0]) cpu.trace_mode = 1;
		else cpu.trace_mode = 2;
	}
	if (my >= mem_y && my < mem_y + mem_size && mx >= mem_x && mx < mem_x + 37)
	{
		needclr++; activedbg = dbgwnd::mem;
		const unsigned dx = mx - mem_x;
		if (mem_dump)
		{
			if (dx >= 5)
				cpu.mem_curs = cpu.mem_top + (dx - 5) + (my - mem_y) * 32;
		}
		else
		{
			const unsigned mem_se = (dx - 5) % 3;
			if (dx >= 29) cpu.mem_curs = cpu.mem_top + (dx - 29) + (my - mem_y) * 8, mem_ascii = 1;
			if (dx >= 5 && mem_se != 2 && dx < 29)
				cpu.mem_curs = cpu.mem_top + (dx - 5) / 3 + (my - mem_y) * 8,
				cpu.mem_second = mem_se, mem_ascii = 0;
		}
	}
	if (mx >= regs_x && my >= regs_y && mx < regs_x + 32 && my < regs_y + 4) {
		needclr++; activedbg = dbgwnd::regs;
		for (unsigned i = 0; i < regs_layout_count; i++) {
			unsigned delta = 1;
			if (regs_layout[i].width == 16) delta = 4;
			if (regs_layout[i].width == 8) delta = 2;
			if (my - regs_y == regs_layout[i].y && mx - regs_x - regs_layout[i].x < delta) regs_curs = i;
		}
	}
	if (mx >= banks_x && my >= banks_y + 1 && mx < banks_x + 7 && my < banks_y + 5) {
		needclr++; activedbg = dbgwnd::banks;
		selbank = my - (banks_y + 1); showbank = true;
	}
	else showbank = false;

	if (mousepos & 0x80000000) { // right-click
		enum { idm_bpx = 1, idm_some_other };
		const auto menu = CreatePopupMenu();
		if (activedbg == dbgwnd::trace) {
			AppendMenu(menu, MF_STRING, idm_bpx, "breakpoint");
		}
		else {
			AppendMenu(menu, MF_STRING, 0, "I don't know");
			AppendMenu(menu, MF_STRING, 0, "what to place");
			AppendMenu(menu, MF_STRING, 0, "to menu, so");
			AppendMenu(menu, MF_STRING, 0, "No Stuff Here");
		}
		POINT globalpos; GetCursorPos(&globalpos);
		const auto cmd = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_LEFTALIGN | TPM_TOPALIGN,
			globalpos.x,
			globalpos.y, 0, wnd, nullptr);
		DestroyMenu(menu);
		if (cmd == idm_bpx) cbpx();
		//if (cmd == IDM_SOME_OTHER) some_other();
		//needclr++;
	}
	mousepos = 0;
}

void TCpuMgr::copy_to_prev()
{
	for (unsigned i = 0; i < count; i++)
		prev_cpus_[i] = TZ80State(*cpus_[i]);
}

/* ------------------------------------------------------------- */
void debug(Z80 *cpu)
{
	sound_stop();
	temp.mon_scale = temp.scale;
	temp.scale = 1;
	needclr = 1;
	dbgbreak = 1;
	flip();
	const auto oldrflags = temp.rflags;
	temp.rflags = RF_MONITOR;
	//set_video();
	ShowWindow(debug_wnd, SW_SHOW);

	TCpuMgr::set_current_cpu(cpu->GetIdx());
	auto prevcpu = &TCpuMgr::prev_cpu(cpu->GetIdx());
	cpu->trace_curs = cpu->pc;
	cpu->dbg_stopsp = cpu->dbg_stophere = -1;
	cpu->dbg_loop_r1 = 0, cpu->dbg_loop_r2 = 0xFFFF;
	mousepos = 0;

	while (dbgbreak) // debugger event loop
	{
		if (trace_labels)
			mon_labels.notify_user_labels();

		cpu = &TCpuMgr::get_cpu();
		prevcpu = &TCpuMgr::prev_cpu(cpu->GetIdx());
	repaint_dbg:
		cpu->trace_top &= 0xFFFF;
		cpu->trace_curs &= 0xFFFF;

		debugscr();
		if (cpu->trace_curs < cpu->trace_top || cpu->trace_curs >= cpu->trpc[trace_size] || asmii == UINT_MAX)
		{
			cpu->trace_top = cpu->trace_curs;
			debugscr();
		}

		debugflip();

	sleep:
		while (!dispatch(nullptr))
		{
			if (mousepos)
				handle_mouse();
			if (needclr)
			{
				needclr--;
				goto repaint_dbg;
			}
			if (!dbgbreak)
				goto leave_dbg;	/* ugh... too much gotos... */
			Sleep(20);
		}
		if (activedbg == dbgwnd::regs && dispatch_more(ac_regs) > 0)
		{
			continue;
		}
		if (activedbg == dbgwnd::trace && dispatch_more(ac_trace) > 0)
		{
			continue;
		}
		if (activedbg == dbgwnd::mem && dispatch_more(ac_mem) > 0)
		{
			continue;
		}
		if (activedbg == dbgwnd::banks && dispatch_more(ac_banks) > 0)
		{
			continue;
		}
		if (activedbg == dbgwnd::regs && dispatch_regs())
		{
			continue;
		}
		if (activedbg == dbgwnd::trace && dispatch_trace())
		{
			continue;
		}
		if (activedbg == dbgwnd::mem && dispatch_mem())
		{
			continue;
		}
		if (activedbg == dbgwnd::banks && dispatch_banks())
		{
			continue;
		}
		if (needclr)
		{
			needclr--;
			continue;
		}
		goto sleep;
	}

leave_dbg:
	*prevcpu = TZ80State(*cpu);
	//   CpuMgr.CopyToPrev();
	cpu->SetLastT();
	temp.scale = temp.mon_scale;
	//temp.rflags = RF_GDI; // facepalm.jpg
	temp.rflags = oldrflags;
	//apply_video();
	ShowWindow(debug_wnd, SW_HIDE);
	sound_play();
	input.nokb = 20;
}

void debug_cond_check(Z80 *cpu)
{
	if (cpu->cbpn)
	{
		cpu->r_low = (cpu->r_low & 0x7F) + cpu->r_hi;
		for (unsigned i = 0; i < cpu->cbpn; i++)
		{
			if (calc(cpu, cpu->cbp[i]))
			{
				cpu->dbgbreak |= 1;
				dbgbreak |= 1;
			}
		}
	}
}

void debug_events(Z80 *cpu)
{
	const auto pc = cpu->pc & 0xFFFF;
	const auto membit = cpu->membits + pc;
	*membit |= MEMBITS_X;
	cpu->dbgbreak |= (*membit & MEMBITS_BPX);
	dbgbreak |= (*membit & MEMBITS_BPX);

	if (pc == cpu->dbg_stophere)
	{
		cpu->dbgbreak = 1;
		dbgbreak = 1;
	}

	if ((cpu->sp & 0xFFFF) == cpu->dbg_stopsp)
	{
		if (pc > cpu->dbg_stophere && pc < cpu->dbg_stophere + 0x100)
		{
			cpu->dbgbreak = 1;
			dbgbreak = 1;
		}
		if (pc < cpu->dbg_loop_r1 || pc > cpu->dbg_loop_r2)
		{
			cpu->dbgbreak = 1;
			dbgbreak = 1;
		}
	}

	debug_cond_check(cpu);
	brk_port_in = brk_port_out = -1; // reset only when breakpoints active
	brk_mem_rd = brk_mem_wr = -1;	// reset only when breakpoints active

	if (cpu->dbgbreak)
		debug(cpu);
}

#endif // MOD_MONITOR

u8 isbrk(const Z80 &cpu) // is there breakpoints active or any other reason to use debug z80 loop?
{
#ifndef MOD_DEBUGCORE
	return 0;
#else

#ifdef MOD_MEMBAND_LED
	if (conf.led.memband & 0x80000000)
		return 1;
#endif

	if (conf.mem_model == MM_PROFSCORP)
		return 1; // breakpoint on read ROM switches ROM bank

#ifdef MOD_MONITOR
	if (cpu.cbpn)
		return 1;
	u8 res = 0;
	for (int i = 0; i < 0x10000; i++)
		res |= cpu.membits[i];
	return (res & (MEMBITS_BPR | MEMBITS_BPW | MEMBITS_BPX));
#endif

#endif
}


/* ===================== */

static LRESULT APIENTRY DebugWndProc(HWND hwnd, UINT uMessage, WPARAM wparam, LPARAM lparam)
{
	PAINTSTRUCT ps;

	if (uMessage == WM_CLOSE)
	{
		mon_emul();
		return 0;
	}

	if (uMessage == WM_PAINT)
	{
		const auto bptr = debug_gdibuf;
		const auto hdc = BeginPaint(hwnd, &ps);
		SetDIBitsToDevice(hdc, 0, 0, DEBUG_WND_WIDTH, DEBUG_WND_HEIGHT, 0, 0, 0, DEBUG_WND_HEIGHT, bptr, &debug_gdibmp.header, DIB_RGB_COLORS);
		EndPaint(hwnd, &ps);
		return 0L;
	}

	if (uMessage == WM_COMMAND)
	{
		switch (wparam) {
		case IDM_DEBUG_RUN: mon_emul(); break;

		case IDM_DEBUG_STEP: mon_step(); break;
		case IDM_DEBUG_STEPOVER: mon_stepover(); break;
		case IDM_DEBUG_TILLRETURN: mon_exitsub(); break;
		case IDM_DEBUG_RUNTOCURSOR: chere(); break;

		case IDM_BREAKPOINT_TOGGLE: cbpx(); break;
		case IDM_BREAKPOINT_MANAGER: mon_bpdialog(); break;

		case IDM_MON_LOADBLOCK: mon_load(); break;
		case IDM_MON_SAVEBLOCK: mon_save(); break;
		case IDM_MON_FILLBLOCK: mon_fill(); break;

		case IDM_MON_RIPPER: mon_tool(); break;
		default: ;
		}
		needclr = 1;
	}

	return DefWindowProc(hwnd, uMessage, wparam, lparam);
}

void init_debug()
{
	WNDCLASS  wc{};
	RECT cl_rect;
	const DWORD dw_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	wc.lpfnWndProc = WNDPROC(DebugWndProc);
	wc.hInstance = hIn;
	wc.lpszClassName = "DEBUG_WND";
	wc.hIcon = LoadIcon(hIn, MAKEINTRESOURCE(IDI_MAIN));
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	RegisterClass(&wc);

	debug_menu = LoadMenu(hIn, MAKEINTRESOURCE(IDR_DEBUGMENU));

	debug_wnd = CreateWindow("DEBUG_WND", "UnrealSpeccy debugger", dw_style,
		CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, nullptr, debug_menu, hIn, NULL);

	cl_rect.left = 0;
	cl_rect.top = 0;
	cl_rect.right = DEBUG_WND_WIDTH - 1;
	cl_rect.bottom = DEBUG_WND_HEIGHT - 1;
	AdjustWindowRect(&cl_rect, dw_style, GetMenu(debug_wnd) != nullptr);
	SetWindowPos(debug_wnd, nullptr, 0, 0, cl_rect.right - cl_rect.left + 1, cl_rect.bottom - cl_rect.top + 1, SWP_NOMOVE);

	for (unsigned i = 0; i < 0x100; i++)
	{
		const unsigned y = (i & 8) ? 0xFF : 0xC0;
		const unsigned r = (i & 2) ? y : 0;
		const unsigned g = (i & 4) ? y : 0;
		const unsigned b = (i & 1) ? y : 0;

		debug_gdibmp.header.bmiColors[i].rgbRed = r;
		debug_gdibmp.header.bmiColors[i].rgbGreen = g;
		debug_gdibmp.header.bmiColors[i].rgbBlue = b;
	}

	init_tsconf();
}