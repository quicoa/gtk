/*  Copyright 2023 Red Hat, Inc.
 *
 * GTK+ is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * GLib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GTK+; see the file COPYING.  If not,
 * see <http://www.gnu.org/licenses/>.
 *
 * Author: Matthias Clasen
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <glib/gi18n-lib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include "gtk-path-tool.h"


void
do_render (int          *argc,
           const char ***argv)
{
  GError *error = NULL;
  const char *fill = "winding";
  const char *fg_color = "black";
  const char *bg_color = "white";
  const char *output_file = NULL;
  char **args = NULL;
  GOptionContext *context;
  const GOptionEntry entries[] = {
    { "fill-rule", 0, 0, G_OPTION_ARG_STRING, &fill, N_("Fill rule (winding, even-odd)"), N_("VALUE") },
    { "fg-color", 0, 0, G_OPTION_ARG_STRING, &fg_color, N_("Foreground color"), N_("COLOR") },
    { "bg-color", 0, 0, G_OPTION_ARG_STRING, &bg_color, N_("Background color"), N_("COLOR") },
    { "output", 0, 0, G_OPTION_ARG_FILENAME, &output_file, N_("The output file"), N_("FILE") },
    { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &args, NULL, N_("PATH") },
    { NULL, }
  };
  GskPath *path;
  GskFillRule fill_rule;
  GdkRGBA fg, bg;
  graphene_rect_t bounds;
  GskRenderNode *fg_node, *nodes[2], *node;
  GdkSurface *surface;
  GskRenderer *renderer;
  GdkTexture *texture;
  const char *filename;

  if (gdk_display_get_default () == NULL)
    {
      g_printerr ("%s\n", _("Could not initialize windowing system"));
      exit (1);
    }

  g_set_prgname ("gtk4-path-tool render");
  context = g_option_context_new (NULL);
  g_option_context_set_translation_domain (context, GETTEXT_PACKAGE);
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_set_summary (context, _("Render the path to a png image."));

  if (!g_option_context_parse (context, argc, (char ***)argv, &error))
    {
      g_printerr ("%s\n", error->message);
      g_error_free (error);
      exit (1);
    }

  g_option_context_free (context);

  if (args == NULL)
    {
      g_printerr ("%s\n", _("No path specified"));
      exit (1);
    }

  if (g_strv_length (args) > 1)
    {
      g_printerr ("%s\n", _("Can only render a single path"));
      exit (1);
    }

  path = get_path (args[0]);

  fill_rule = get_enum_value (GSK_TYPE_FILL_RULE, _("fill rule"), fill);
  get_color (&fg, fg_color);
  get_color (&bg, bg_color);

  gsk_path_get_bounds (path, &bounds);
  graphene_rect_inset (&bounds, -10, -10);

  nodes[0] = gsk_color_node_new (&bg, &bounds);
  fg_node = gsk_color_node_new (&fg, &bounds);
  nodes[1] = gsk_fill_node_new (fg_node, path, fill_rule);

  node = gsk_container_node_new (nodes, 2);

  gsk_render_node_unref (fg_node);
  gsk_render_node_unref (nodes[0]);
  gsk_render_node_unref (nodes[1]);

  surface = gdk_surface_new_toplevel (gdk_display_get_default ());
  renderer = gsk_renderer_new_for_surface (surface);

  texture = gsk_renderer_render_texture (renderer, node, &bounds);

  filename = output_file ? output_file : "path.png";
  if (!gdk_texture_save_to_png (texture, filename))
    {
      char *msg = g_strdup_printf (_("Saving png to '%s' failed"), filename);
      g_printerr ("%s\n", msg);
      exit (1);
    }

  if (output_file == NULL)
    {
      char *msg = g_strdup_printf (_("Output written to '%s'."), filename);
      g_print ("%s\n", msg);
      g_free (msg);
    }

  g_object_unref (texture);
  gsk_renderer_unrealize (renderer);
  g_object_unref (renderer);
  g_object_unref (surface);
  gsk_render_node_unref (node);

  gsk_path_unref (path);

  g_strfreev (args);
}