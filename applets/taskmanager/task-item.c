/*
 * Copyright (C) 2008 Neil Jagdish Patel <njpatel@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as 
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by Hannes Verschore <hv1989@gmail.com>
 *
 */

#include "task-item.h"

#include <libawn/libawn.h>

#define TASK_ITEM_ICON_SCALE 0.65
#define MAX_TASK_ITEM_CHARS 50

G_DEFINE_ABSTRACT_TYPE (TaskItem, task_item, GTK_TYPE_BUTTON)

#define TASK_ITEM_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
  TASK_TYPE_ITEM, \
  TaskItemPrivate))

struct _TaskItemPrivate
{
  GtkWidget *box;
  GtkWidget *name;    /*name label*/
  GtkWidget *image;   /*placed in button (TaskItem) with label*/
  GdkPixbuf *icon;

  TaskIcon *task_icon;

};

enum
{
  PROP_0
};

enum
{
  NAME_CHANGED,
  ICON_CHANGED,
  VISIBLE_CHANGED,

  LAST_SIGNAL
};
static guint32 _item_signals[LAST_SIGNAL] = { 0 };

/* Forwards */

static void task_item_icon_changed      (TaskItem *item, GdkPixbuf     *icon);
static void task_item_visible_changed   (TaskItem *item, gboolean       visible);
static void task_item_name_changed      (TaskItem *item, const gchar   *name);
static gboolean  task_item_button_release_event (GtkWidget      *widget,
                                                 GdkEventButton *event);
static gboolean  task_item_button_press_event   (GtkWidget      *widget,
                                                 GdkEventButton *event);
static void task_item_activate (GtkWidget *widget, gpointer null);

/* GObject stuff */

static void
task_item_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
/*  TaskItem        *icon = TASK_ITEM (object);
  TaskItemPrivate *priv = icon->priv;
*/
  switch (prop_id)
  {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
task_item_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
/*  TaskItem *icon = TASK_ITEM (object);
  TaskItemPrivate *priv = icon->priv;
  */
  switch (prop_id)
  {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}


static void
task_item_dispose (GObject *object)
{
  TaskItem *item = TASK_ITEM (object);
  TaskItemPrivate *priv = TASK_ITEM_GET_PRIVATE (object);

  if (priv->icon)
  {
    g_object_unref (priv->icon);
    priv->icon = NULL;
  }

  // this removes the overlays from the associated TaskIcon
  task_item_set_task_icon (item, NULL);

  item->text_overlay = NULL;
  item->progress_overlay = NULL;
  item->icon_overlay = NULL;

  G_OBJECT_CLASS (task_item_parent_class)->dispose (object);
}


static void
task_item_finalize (GObject *object)
{
  G_OBJECT_CLASS (task_item_parent_class)->finalize (object);
}

static void
task_item_constructed (GObject *object)
{
  TaskItemClass *klass;  
  klass = TASK_ITEM_GET_CLASS (object);  
  g_return_if_fail (klass->is_visible);
  
  if (G_OBJECT_CLASS (task_item_parent_class)->constructed)
  {
    G_OBJECT_CLASS (task_item_parent_class)->constructed (object);
  }

  /* connect to signals */
  g_signal_connect (G_OBJECT (object), "name-changed",
                    G_CALLBACK (klass->name_change), NULL);
  g_signal_connect (G_OBJECT (object), "visible-changed",
                    G_CALLBACK (task_item_visible_changed), NULL);
  g_signal_connect (G_OBJECT (object), "activate",
                    G_CALLBACK (task_item_activate), NULL);
  g_signal_connect (G_OBJECT (object), "icon-changed",
                    G_CALLBACK (task_item_icon_changed), NULL);  
}


static void
task_item_class_init (TaskItemClass *klass)
{
  //GParamSpec   *pspec;
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *wid_class = GTK_WIDGET_CLASS (klass);
//  GParamSpec     *pspec;

  obj_class->dispose = task_item_dispose;
  obj_class->finalize = task_item_finalize;
  obj_class->constructed = task_item_constructed;  
  obj_class->set_property = task_item_set_property;
  obj_class->get_property = task_item_get_property;

  wid_class->button_release_event = task_item_button_release_event;
  wid_class->button_press_event   = task_item_button_press_event;
  
  /* We implement the necessary funtions for a normal item */
  klass->get_name        = NULL;
  klass->get_icon        = NULL;
  klass->is_visible      = NULL;
  klass->match           = NULL;
  klass->name_change    = task_item_name_changed;
  
  /* Install signals */
  _item_signals[NAME_CHANGED] =
		g_signal_new ("name-changed",
			      G_OBJECT_CLASS_TYPE (obj_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (TaskItemClass, name_changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__STRING, 
			      G_TYPE_NONE,
            1, G_TYPE_STRING); 

  _item_signals[ICON_CHANGED] =
		g_signal_new ("icon-changed",
			      G_OBJECT_CLASS_TYPE (obj_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (TaskItemClass, icon_changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT, 
			      G_TYPE_NONE,
            1, GDK_TYPE_PIXBUF);

  _item_signals[VISIBLE_CHANGED] =
		g_signal_new ("visible-changed",
			      G_OBJECT_CLASS_TYPE (obj_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (TaskItemClass, visible_changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__BOOLEAN, 
			      G_TYPE_NONE, 
            1, G_TYPE_BOOLEAN);

  g_type_class_add_private (obj_class, sizeof (TaskItemPrivate));
}

static void
task_item_init (TaskItem *item)
{
  TaskItemPrivate *priv;
  TaskItemClass *klass;

  klass = TASK_ITEM_GET_CLASS (item);  
  
  /* get and save private struct */
  priv = item->priv = TASK_ITEM_GET_PRIVATE (item);

  /* let this button listen to every event */
  gtk_widget_add_events (GTK_WIDGET (item), GDK_ALL_EVENTS_MASK);

  /* for looks */
  gtk_button_set_relief (GTK_BUTTON (item), GTK_RELIEF_NONE);

  /* create content */
  priv->box = gtk_hbox_new (FALSE, 10);
  gtk_container_add (GTK_CONTAINER (item), priv->box);
  gtk_container_set_border_width (GTK_CONTAINER (priv->box), 1);

  priv->image = GTK_WIDGET (awn_image_new ());
  gtk_box_pack_start (GTK_BOX (priv->box), priv->image, FALSE, FALSE, 0);
  
  priv->name = gtk_label_new ("");
  
  /*
   TODO once get/set prop is available create this a config key and bind
   */
  gtk_label_set_max_width_chars (GTK_LABEL(priv->name), MAX_TASK_ITEM_CHARS);
  gtk_label_set_ellipsize (GTK_LABEL(priv->name),PANGO_ELLIPSIZE_END);
  gtk_box_pack_start (GTK_BOX (priv->box), priv->name, TRUE, FALSE, 10);
  
}


static void
unset_inhibit_focus_loss_cb (TaskIcon * icon)
{
      task_icon_set_inhibit_focus_loss (icon,FALSE);
}

static gboolean
task_item_button_release_event (GtkWidget      *widget,
                                GdkEventButton *event)
{
  GtkWidget * menu;
  TaskItemPrivate *priv = TASK_ITEM_GET_PRIVATE (widget);

  g_return_val_if_fail (TASK_IS_ITEM (widget), FALSE);

  switch (event->button)
  {
    case 1:
      task_item_left_click (TASK_ITEM (widget), event);
      break;
    case 2:
      task_item_middle_click (TASK_ITEM (widget), event);
      break;
    case 3:
      /*
       Crazy hoops to keep the dialog open when bringing up a context menu.
      Keeps it from closing due to focus loss.
       */
      task_icon_set_inhibit_focus_loss (priv->task_icon,TRUE);
      menu = task_item_right_click (TASK_ITEM (widget), event);
      g_signal_connect_swapped (menu,"deactivate", 
                                G_CALLBACK(gtk_widget_hide),
                                task_icon_get_dialog(priv->task_icon));
      g_signal_connect_swapped (menu,"deactivate",
                                G_CALLBACK(unset_inhibit_focus_loss_cb),
                                priv->task_icon);
      break;
  }

  return FALSE;
}

static gboolean
task_item_button_press_event (GtkWidget      *widget,
                              GdkEventButton *event)
{
  g_return_val_if_fail (TASK_IS_ITEM (widget), FALSE);
  gtk_widget_queue_draw (widget);

  return FALSE;
}

static void 
task_item_activate (GtkWidget *widget, gpointer null)
{
  g_return_if_fail (TASK_IS_ITEM (widget));
  
  /*
   FIXME TODO
   This works for now as the GdkEventButton arg has already
   been checked before this.. and it's not checked 
   by any of the called code.
   
   That being said... It probably makes sense to remove
   the event arg from the parameters to task_item_*_click()
   and the functions called by them.  If it doesn't make sense
   then this needs to this function call needs to be replaced 
   with something else :-)
   */
  task_item_left_click (TASK_ITEM(widget), NULL);
}

static void 
task_item_name_changed (TaskItem *item, const gchar *markup)
{
  TaskItemPrivate *priv = TASK_ITEM_GET_PRIVATE (item);
  gtk_label_set_markup (GTK_LABEL (priv->name), markup);
}

static void 
task_item_icon_changed (TaskItem *item, GdkPixbuf *icon)
{
  TaskItemPrivate *priv = TASK_ITEM_GET_PRIVATE (item);
  GdkPixbuf * scaled;
  gint  height;
  gint  width;
  gint  scaled_height;
  gint  scaled_width;
  
  g_return_if_fail (icon);
  g_return_if_fail (GDK_IS_PIXBUF(icon));
  if (priv->icon)
  {
    g_object_unref (priv->icon);
  }
  priv->icon = icon;
  g_object_ref (icon);
  
  /* height should be equal to width... but just in case */
  height = gdk_pixbuf_get_height (icon);
  width = gdk_pixbuf_get_width (icon);
  gtk_icon_size_lookup (GTK_ICON_SIZE_BUTTON,&scaled_width,&scaled_height);  
  if (height != scaled_height)
  {
    scaled = gdk_pixbuf_scale_simple (icon,scaled_width,scaled_height,GDK_INTERP_BILINEAR);    
  }
  else
  {
    scaled = icon;
    g_object_ref (scaled);
  }
  
  gtk_image_set_from_pixbuf (GTK_IMAGE (priv->image), scaled);
  g_object_unref (scaled);
  
}

static void
task_item_visible_changed (TaskItem *item, gboolean visible)
{
  if (visible)
    gtk_widget_show (GTK_WIDGET (item));
  else
    gtk_widget_hide (GTK_WIDGET (item));
}

/**
 * Public functions
 */

const gchar * 
task_item_get_name (TaskItem    *item)
{
  TaskItemClass *klass;

  g_return_val_if_fail (TASK_IS_ITEM (item), NULL);
  
  klass = TASK_ITEM_GET_CLASS (item);
  g_return_val_if_fail (klass->get_name, NULL);
        
  return klass->get_name (item);
}

GdkPixbuf *
task_item_get_icon (TaskItem    *item)
{
  TaskItemClass *klass;

  g_return_val_if_fail (TASK_IS_ITEM (item), NULL);
  
  klass = TASK_ITEM_GET_CLASS (item);
  g_return_val_if_fail (klass->get_icon, NULL);
        
  return klass->get_icon (item);
}

gboolean
task_item_is_visible (TaskItem    *item)
{
  TaskItemClass *klass;

  g_return_val_if_fail (TASK_IS_ITEM (item), FALSE);
  
  klass = TASK_ITEM_GET_CLASS (item);  
  g_return_val_if_fail (klass->is_visible, FALSE);
        
  return klass->is_visible (item);
}

void
task_item_left_click (TaskItem *item, GdkEventButton *event)
{
  TaskItemClass *klass;

  g_return_if_fail (TASK_IS_ITEM (item));
  
  klass = TASK_ITEM_GET_CLASS (item);
  g_return_if_fail (klass->left_click);
        
  klass->left_click (item, event);
}

GtkWidget *
task_item_right_click (TaskItem *item, GdkEventButton *event)
{
  TaskItemClass *klass;

  g_return_val_if_fail (TASK_IS_ITEM (item),NULL);
  
  klass = TASK_ITEM_GET_CLASS (item);
  g_return_val_if_fail (klass->right_click,NULL);
        
  return klass->right_click (item, event);
}

void
task_item_middle_click (TaskItem *item, GdkEventButton *event)
{
  TaskItemClass *klass;

  g_return_if_fail (TASK_IS_ITEM (item));
  
  klass = TASK_ITEM_GET_CLASS (item);
  g_return_if_fail (klass->right_click);

  if (klass->middle_click)
  {
    klass->middle_click (item, event);
  }
}

guint
task_item_match (TaskItem *item, TaskItem *item_to_match)
{
  TaskItemClass *klass;

  g_return_val_if_fail (TASK_IS_ITEM (item), 0);
  
  klass = TASK_ITEM_GET_CLASS (item);
  g_return_val_if_fail (klass->match, 0);
        
  return klass->match (item, item_to_match);
}

void
task_item_set_task_icon (TaskItem *item, TaskIcon *icon)
{
  g_return_if_fail (TASK_IS_ITEM (item));

  TaskItemPrivate *priv = TASK_ITEM_GET_PRIVATE (item);

  // add/remove overlays
  if (priv->task_icon)
  {
    AwnOverlayable *over = AWN_OVERLAYABLE (priv->task_icon);
    if (item->text_overlay)
      awn_overlayable_remove_overlay (over, AWN_OVERLAY (item->text_overlay));
    if (item->progress_overlay)
      awn_overlayable_remove_overlay (over, AWN_OVERLAY (item->progress_overlay));
    if (item->icon_overlay)
      awn_overlayable_remove_overlay (over, AWN_OVERLAY (item->icon_overlay));
  }

  priv->task_icon = icon;
  if (icon)
  {
    AwnOverlayable *over = AWN_OVERLAYABLE (icon);
    // we can control what's on top here
    if (item->icon_overlay)
      awn_overlayable_add_overlay (over, AWN_OVERLAY (item->icon_overlay));
    if (item->progress_overlay)
      awn_overlayable_add_overlay (over, AWN_OVERLAY (item->progress_overlay));
    if (item->text_overlay)
      awn_overlayable_add_overlay (over, AWN_OVERLAY (item->text_overlay));
  }
}

TaskIcon*
task_item_get_task_icon (TaskItem *item)
{
  g_return_val_if_fail (TASK_IS_ITEM (item), NULL);

  TaskItemPrivate *priv = TASK_ITEM_GET_PRIVATE (item);

  return priv->task_icon; 
}

GtkWidget*
task_item_get_image_widget (TaskItem *item)
{
  g_return_val_if_fail (TASK_IS_ITEM (item), NULL);

  TaskItemPrivate *priv = TASK_ITEM_GET_PRIVATE (item);

  return priv->image;
}

/**
 * Protected functions (used only by derived classes)
 */

void
task_item_emit_name_changed (TaskItem *item, const gchar *name)
{
  g_return_if_fail (TASK_IS_ITEM (item));
  
  g_signal_emit (item, _item_signals[NAME_CHANGED], 0, name);
}

void
task_item_emit_icon_changed (TaskItem *item, GdkPixbuf *icon)
{
  g_return_if_fail (TASK_IS_ITEM (item));

  g_signal_emit (item, _item_signals[ICON_CHANGED], 0, icon);
}

void
task_item_emit_visible_changed (TaskItem *item, gboolean visible)
{
  g_return_if_fail (TASK_IS_ITEM (item));

  g_signal_emit (item, _item_signals[VISIBLE_CHANGED], 0, visible);
}
