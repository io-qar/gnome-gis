/* gis-window.c
 *
 * Copyright 2024 koro
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "config.h"

#include "gis-window.h"

struct _GisWindow
{
	AdwApplicationWindow  parent_instance;
        GSettings *settings;

	/* Template widgets */
	AdwHeaderBar        *header_bar;
	GtkTextView *main_text_view;
	GtkButton *open_button;
};

G_DEFINE_FINAL_TYPE (GisWindow, gis_window, ADW_TYPE_APPLICATION_WINDOW)

static void
open_file_complete (GObject *src_obj, GAsyncResult *res, GisWindow *self)
{
        GFile *file = G_FILE (src_obj);
        g_autofree char *contents = NULL;
        gsize length = 0;
        g_autoptr (GError) err = NULL;

        g_file_load_contents_finish (file, res, &contents, &length, NULL, &err);
        if (err != NULL)
                {
                        g_printerr ("Unable to open '%s': %s\n", g_file_peek_path (file), err->message);
                        return;
                }

        if (!g_utf8_validate (contents, length, NULL))
                {
                        g_printerr ("Unable to load the contents of '%s':\nthe file is not encoded with UTF-8\n", g_file_peek_path (file));
                        return;
                }
        GtkTextBuffer *buf = gtk_text_view_get_buffer (self->main_text_view);
        gtk_text_buffer_set_text (buf, contents, length);

        GtkTextIter start;
        gtk_text_buffer_get_start_iter (buf, &start);
        gtk_text_buffer_place_cursor (buf, &start);
}

static void
open_file (GisWindow *self, GFile *file)
{
        g_file_load_contents_async (file, NULL, (GAsyncReadyCallback) open_file_complete, self);
}

static void
on_open_response (GObject *src, GAsyncResult *res, gpointer user_data)
{
        GtkFileDialog *dialog = GTK_FILE_DIALOG (src);
        GisWindow *self = user_data;

        g_autoptr (GFile) file = gtk_file_dialog_open_finish (dialog, res, NULL);
        if (file != NULL) open_file (self, file);
}

static void
gis_window__open_file_dialog (GAction *action,
			      GVariant *parameter,
			      GisWindow *self)
{
        g_autoptr (GtkFileDialog) dialog = gtk_file_dialog_new ();
        gtk_file_dialog_open (dialog, GTK_WINDOW (self), NULL, on_open_response, self);
}

static void
gis_window_finilize (GObject *gobj)
{
        GisWindow *self = GIS_WINDOW (gobj);
        g_clear_object (&self->settings);
        G_OBJECT_CLASS (gis_window_parent_class) -> finalize (gobj);
}

static void
gis_window_class_init (GisWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
        GObjectClass *gobj_class = G_OBJECT_CLASS (klass);
        gobj_class->finalize = gis_window_finilize;

	gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/gis/gis-window.ui");
	gtk_widget_class_bind_template_child (widget_class, GisWindow, header_bar);
	gtk_widget_class_bind_template_child (widget_class, GisWindow, main_text_view);
	gtk_widget_class_bind_template_child (widget_class, GisWindow, open_button);
}

static void
gis_window_init (GisWindow *self)
{
	gtk_widget_init_template (GTK_WIDGET (self));
        g_autoptr (GSimpleAction) open_action = g_simple_action_new ("open", NULL);
        g_signal_connect (open_action, "activate", G_CALLBACK (gis_window__open_file_dialog), self);
        g_action_map_add_action (G_ACTION_MAP (self), G_ACTION (open_action));

        self->settings = g_settings_new ("org.gnome.gis");
        g_settings_bind (self->settings, "window-width",
                   G_OBJECT (self), "default-width",
                   G_SETTINGS_BIND_DEFAULT);
  g_settings_bind (self->settings, "window-height",
                   G_OBJECT (self), "default-height",
                   G_SETTINGS_BIND_DEFAULT);

        g_settings_bind (self->settings, "window-maximized", G_OBJECT (self), "maximized", G_SETTINGS_BIND_DEFAULT);
}
