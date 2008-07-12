/*
 * Copyright (C) 2008 Rodney Cryderman <rcryderman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

/* awn-icons.h */

#ifndef _AWN_ICONS
#define _AWN_ICONS

#include <glib-object.h>
#include <gtk/gtk.h>
#include <libawn/awn-applet.h>

G_BEGIN_DECLS

#define LIBAWN_TYPE_AWN_ICONS awn_icons_get_type()

#define LIBAWN_AWN_ICONS(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), LIBAWN_TYPE_AWN_ICONS, AwnIcons))

#define LIBAWN_AWN_ICONS_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), LIBAWN_TYPE_AWN_ICONS, AwnIconsClass))

#define LIBAWN_IS_AWN_ICONS(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LIBAWN_TYPE_AWN_ICONS))

#define LIBAWN_IS_AWN_ICONS_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), LIBAWN_TYPE_AWN_ICONS))

#define LIBAWN_AWN_ICONS_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), LIBAWN_TYPE_AWN_ICONS, AwnIconsClass))

typedef struct { 
  GObject parent;
} AwnIcons;

typedef struct {
  GObjectClass parent_class;
} AwnIconsClass;

typedef void(* AwnIconsChange )(AwnIcons * fx,gpointer user_data);

GType awn_icons_get_type (void);

AwnIcons* awn_icons_new (void);

GdkPixbuf * awn_icons_get_icon(AwnIcons * icons, const gchar * state);
GdkPixbuf * awn_icons_get_icon_simple(AwnIcons * icons);

void awn_icons_set_icons_info(AwnIcons * icons,
                              GtkWidget * applet,
                              const gchar * applet_name,
                              const gchar * uid,
                              gint height,
                              const gchar **states,
                              const gchar **icon_names);

void awn_icons_set_icon_info(AwnIcons * icons,
                             GtkWidget * applet,
                             const gchar * applet_name,
                             const gchar * uid, 
                             gint height,
                             const gchar *icon_name);
                             
void awn_icons_set_changed_cb(AwnIcons * icons,AwnIconsChange fn,gpointer user_data);                             

G_END_DECLS

#endif /* _AWN_ICONS */