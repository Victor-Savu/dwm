#ifndef CONFIG_H
#define CONFIG_H
#include <stddef.h>
#include "types.h"

/* appearance */
const char dmenufont[];
const size_t LENGTH_dmenufont;
const char normbordercolor[];
const size_t LENGTH_normbordercolor;
const char normbgcolor[];
const size_t LENGTH_normbgcolor;
const char normfgcolor[];
const size_t LENGTH_normfgcolor;
const char selbordercolor[];
const size_t LENGTH_selbordercolor;
const char selbgcolor[];
const size_t LENGTH_selbgcolor;
const char selfgcolor[];
const size_t LENGTH_selfgcolor;
const unsigned int borderpx;        /* border pixel of windows */
const unsigned int snap;       /* snap pixel */
const Bool showbar;     /* False means no bar */
const Bool topbar;     /* False means bottom bar */

/* tagging */
const char *tags[];
const size_t LENGTH_tags;

const Rule rules[];
const size_t LENGTH_rules;

/* layout(s) */
const float mfact; /* factor of master area size [0.05..0.95] */
const int nmaster;    /* number of clients in master area */
const Bool resizehints; /* True means respect size hints in tiled resizals */

const Layout layouts[];
const size_t LENGTH_layouts;

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
char dmenumon[2]; /* component of dmenucmd, manipulated in spawn() */
const char *dmenucmd[];
const size_t LENGTH_dmenucmd;
const char *termcmd[];
const size_t LENGTH_termcmd;

Key keys[];
const size_t LENGTH_keys;

/* button definitions */
/* click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
Button buttons[];
const size_t LENGTH_buttons;

#endif
