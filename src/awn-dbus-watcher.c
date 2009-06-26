/*
 * Copyright (C) 2009 Michal Hruby <michal.mhr@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include "awn-dbus-watcher.h"

static AwnDBusWatcher *singleton_instance = NULL;

G_DEFINE_TYPE (AwnDBusWatcher, awn_dbus_watcher, G_TYPE_OBJECT)

#define AWN_DBUS_WATCHER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        AWN_TYPE_DBUS_WATCHER, AwnDBusWatcherPrivate))

struct _AwnDBusWatcherPrivate
{
  DBusGConnection *connection;
  DBusGProxy *proxy;
};

enum
{
  CONNECTION_OPENED,
  CONNECTION_CLOSED,

  LAST_SIGNAL
};

static guint _dbus_watcher_signals[LAST_SIGNAL] = { 0 };

static void
on_name_owner_changed (DBusGProxy *proxy,
                       gchar *name, gchar *old_owner, gchar *new_owner,
                       AwnDBusWatcher *watcher)
{
  g_return_if_fail (old_owner && new_owner);

#if 0
  g_debug ("NameOwnerChanged (%s): '%s' -> '%s'",
           name, old_owner, new_owner);
#endif

  if (old_owner[0] == '\0')
  {
    // there's a new object on the bus
#if 0
    // We don't really need this signal atm, so let's ignore it completely.
    g_signal_emit (watcher, _dbus_watcher_signals[CONNECTION_OPENED],
                   g_quark_from_string (new_owner), new_owner);
#endif
  }
  else if (new_owner[0] == '\0')
  {
    // an object disappeared from the bus
    g_signal_emit (watcher, _dbus_watcher_signals[CONNECTION_CLOSED],
                   g_quark_from_string (old_owner), old_owner);
  }
}

static void
awn_dbus_watcher_init (AwnDBusWatcher *watcher)
{
  AwnDBusWatcherPrivate *priv = AWN_DBUS_WATCHER_GET_PRIVATE (watcher);
  GError *error = NULL;

  watcher->priv = priv;

  priv->connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (priv->connection == NULL)
  {
    g_warning ("Unable to make connection to the D-Bus session bus: %s",
               error->message);
    g_error_free (error);
    return;
  }

  priv->proxy = dbus_g_proxy_new_for_name (priv->connection,
                                           "org.freedesktop.DBus", 
                                           "/org/freedesktop/DBus",
                                           "org.freedesktop.DBus");

  g_return_if_fail (priv->proxy);

  dbus_g_proxy_add_signal (priv->proxy, "NameOwnerChanged",
                           G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                           G_TYPE_INVALID);

  dbus_g_proxy_connect_signal (priv->proxy, "NameOwnerChanged",
                               G_CALLBACK (on_name_owner_changed), watcher,
                               NULL);
}

static void
awn_dbus_watcher_finalize (GObject *object)
{
  AwnDBusWatcherPrivate *priv = AWN_DBUS_WATCHER_GET_PRIVATE (object);

  if (priv->proxy)
  {
    g_object_unref (priv->proxy);
    priv->proxy = NULL;
  }

  if (priv->connection)
  {
    dbus_g_connection_unref (priv->connection);
    priv->connection = NULL;
  }

  singleton_instance = NULL;

  G_OBJECT_CLASS (awn_dbus_watcher_parent_class)->finalize (object);
}

static void
awn_dbus_watcher_class_init (AwnDBusWatcherClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->finalize = awn_dbus_watcher_finalize;

  /* Add signals to the class */
  _dbus_watcher_signals[CONNECTION_OPENED] =
                g_signal_new ("connection-opened",
                              G_OBJECT_CLASS_TYPE (obj_class),
                              G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED,
                              G_STRUCT_OFFSET (AwnDBusWatcherClass,
                                               connection_opened),
                              NULL, NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1, G_TYPE_STRING);

  _dbus_watcher_signals[CONNECTION_CLOSED] =
                g_signal_new ("connection-closed",
                              G_OBJECT_CLASS_TYPE (obj_class),
                              G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED,
                              G_STRUCT_OFFSET (AwnDBusWatcherClass,
                                               connection_closed),
                              NULL, NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1, G_TYPE_STRING);

  g_type_class_add_private (obj_class, sizeof (AwnDBusWatcherPrivate));
}

AwnDBusWatcher*
awn_dbus_watcher_get_default (void)
{
  if (singleton_instance == NULL)
  {
    singleton_instance = g_object_new (AWN_TYPE_DBUS_WATCHER, NULL);
  }

  return singleton_instance;
}

