/* See LICENSE file for copyright and license details. */
#include <stddef.h>
#include "types.h"

#include "fwd.h"

#define LENGTH(X)               (sizeof X / sizeof X[0])

/* appearance */
const char dmenufont[] = "Source Code Pro:style=Regular";
const size_t LENGTH_dmenufont = LENGTH(dmenufont);
const char normbordercolor[] = "#586e75";
const size_t LENGTH_normbordercolor = LENGTH(normbordercolor);
const char normbgcolor[]     = "#fdf6e3";
const size_t LENGTH_normbgcolor = LENGTH(normbgcolor);
const char normfgcolor[]     = "#657b83";
const size_t LENGTH_normfgcolor = LENGTH(normfgcolor);
const char selbordercolor[]  = "#005577";
const size_t LENGTH_selbordercolor = LENGTH(selbordercolor);
const char selbgcolor[]      = "#005577";
const size_t LENGTH_selbgcolor = LENGTH(selbgcolor);
const char selfgcolor[]      = "#eeeeee";
const size_t LENGTH_selfgcolor = LENGTH(selfgcolor);
const unsigned int borderpx  = 1;        /* border pixel of windows */
const unsigned int snap      = 32;       /* snap pixel */
const Bool showbar           = True;     /* False means no bar */
const Bool topbar            = True;     /* False means bottom bar */

/* tagging */
const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
const size_t LENGTH_tags = LENGTH(tags);
/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags { char limitexceeded[LENGTH(tags) > 31 ? -1 : 1]; };

const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Gimp",     NULL,       NULL,       0,            True,        -1 },
	{ "Firefox",  NULL,       NULL,       1 << 8,       False,       -1 },
};
const size_t LENGTH_rules = LENGTH(rules);

/* layout(s) */
const float mfact      = 0.55; /* factor of master area size [0.05..0.95] */
const int nmaster      = 1;    /* number of clients in master area */
const Bool resizehints = True; /* True means respect size hints in tiled resizals */

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
const char *dmenucmd[] = { "dmenu_run", "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL };
const size_t LENGTH_dmenucmd = LENGTH(dmenucmd);
const char *termcmd[]  = { "st", NULL };
const size_t LENGTH_termcmd = LENGTH(termcmd);


