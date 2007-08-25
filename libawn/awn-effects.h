/*
 *  Copyright (C) 2007 Michal Hruby <michal.mhr@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.
 *
*/

#ifndef __AWN_EFFECTS_H__
#define __AWN_EFFECTS_H__

#include <gtk/gtk.h>

#include "awn-defines.h"
#include "awn-gconf.h"
#include "awn-title.h"

G_BEGIN_DECLS

typedef enum {
        AWN_EFFECT_NONE,
        AWN_EFFECT_OPENING,
	AWN_EFFECT_LAUNCHING,
        AWN_EFFECT_HOVER,
        AWN_EFFECT_ATTENTION,
        AWN_EFFECT_CLOSING,
        AWN_EFFECT_CHANGE_NAME
} AwnEffect;

typedef const gchar* (*AwnTitleCallback)(GObject*);

typedef struct _AwnEffects AwnEffects;

struct _AwnEffects
{
	GObject *self;
	AwnSettings *settings;
	AwnTitle *title;
	AwnTitleCallback get_title;
	
	gboolean needs_attention;
	gboolean is_closing;
	gboolean hover;
	
	gint icon_width, icon_height;
	
	 /* EFFECT VARIABLES */
	gboolean effect_lock;
	AwnEffect effect_sheduled;
	AwnEffect current_effect;
	gint effect_direction;
	gint count;

	gdouble x_offset;
	gdouble y_offset;
	gint width;
	gint height;
	gdouble rotate_degrees;
	gfloat alpha;

	guint enter_notify;
	guint leave_notify;
};

void
awn_effects_init(GObject *, AwnEffects *);

void
awn_register_effects (GObject *, AwnEffects *);

void
awn_unregister_effects (GObject *, AwnEffects *);

void
awn_schedule_effect(const gint timeout, const AwnEffect effect, AwnEffects *fx, const gint max_loops);

void
awn_effects_set_title(AwnEffects *, AwnTitle*, AwnTitleCallback);

G_END_DECLS

#endif
