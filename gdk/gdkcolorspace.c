/* gdkcolorspace.c
 *
 * Copyright 2021 (c) Benjamin Otte
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * GdkColorSpace:
 *
 * `GdkColorSpace` is used to describe color spaces.
 *
 * Tell developers what a color space is instead of just linking to
 * https://en.wikipedia.org/wiki/Color_space
 *
 * `GdkColorSpace` objects are immutable and therefore threadsafe.
 *
 * Since: 4.6
 */

#include "config.h"

#include "gdkcolorspaceprivate.h"

#include "gdkintl.h"
#include "gdklcmscolorspaceprivate.h"

enum {
  PROP_0,

  N_PROPS
};

//static GParamSpec *properties[N_PROPS];

G_DEFINE_TYPE (GdkColorSpace, gdk_color_space, G_TYPE_OBJECT)

static gboolean
gdk_color_space_default_supports_format (GdkColorSpace   *self,
                                         GdkMemoryFormat  format)
{
  return FALSE;
}

static GBytes *
gdk_color_space_default_save_to_icc_profile (GdkColorSpace  *self,
                                             GError        **error)
{
  g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                       _("This color space does not support ICC profiles"));
  return NULL;
}

static void
gdk_color_space_set_property (GObject      *gobject,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  //GdkColorSpace *self = GDK_COLOR_SPACE (gobject);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
gdk_color_space_get_property (GObject    *gobject,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  //GdkColorSpace *self = GDK_COLOR_SPACE (gobject);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
gdk_color_space_dispose (GObject *object)
{
  //GdkColorSpace *self = GDK_COLOR_SPACE (object);

  G_OBJECT_CLASS (gdk_color_space_parent_class)->dispose (object);
}

static void
gdk_color_space_class_init (GdkColorSpaceClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  klass->supports_format = gdk_color_space_default_supports_format;
  klass->save_to_icc_profile = gdk_color_space_default_save_to_icc_profile;

  gobject_class->set_property = gdk_color_space_set_property;
  gobject_class->get_property = gdk_color_space_get_property;
  gobject_class->dispose = gdk_color_space_dispose;
}

static void
gdk_color_space_init (GdkColorSpace *self)
{
}

/**
 * gdk_color_space_get_srgb:
 *
 * Returns the color profile representing the sRGB color space.
 *
 * If you don't know anything about color profiles but need one for
 * use with some function, this one is most likely the right one.
 *
 * Returns: (transfer none): the color profile for the sRGB
 *   color space.
 *
 * Since: 4.6
 */
GdkColorSpace *
gdk_color_space_get_srgb (void)
{
  static GdkColorSpace *srgb_profile;

  if (g_once_init_enter (&srgb_profile))
    {
      GdkColorSpace *new_profile;

      new_profile = gdk_lcms_color_space_new_from_lcms_profile (cmsCreate_sRGBProfile ());

      g_once_init_leave (&srgb_profile, new_profile);
    }

  return srgb_profile;
}

/**
 * gdk_color_space_supports_format:
 * @self: a `GdkColorSpace`
 * @format: the format to check
 *
 * Checks if this color space can be used with textures in the given format.
 *
 * Returns: %TRUE if this colorspace supports the format
 *
 * Since: 4.6
 **/
gboolean
gdk_color_space_supports_format (GdkColorSpace   *self,
                                 GdkMemoryFormat  format)
{
  g_return_val_if_fail (GDK_IS_COLOR_SPACE (self), FALSE);
  g_return_val_if_fail (format < GDK_MEMORY_N_FORMATS, FALSE);

  return GDK_COLOR_SPACE_GET_CLASS (self)->supports_format (self, format);
}

/**
 * gdk_color_space_save_to_icc_profile:
 * @self: a `GdkColorSpace`
 * @error: Return location for an error
 *
 * Saves the color space to an
 * [ICC profile](https://en.wikipedia.org/wiki/ICC_profile).
 *
 * Some color spaces cannot be represented as ICC profiles. In
 * that case, an error will be set and %NULL will be returned.
 *
 * Returns: A new `GBytes` containing the ICC profile
 *
 * Since: 4.6
 **/
GBytes *
gdk_color_space_save_to_icc_profile (GdkColorSpace  *self,
                                     GError        **error)
{
  g_return_val_if_fail (GDK_IS_COLOR_SPACE (self), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  return GDK_COLOR_SPACE_GET_CLASS (self)->save_to_icc_profile (self, error);
}

/**
 * gdk_color_space_equal:
 * @profile1: (type GdkColorSpace): a `GdkColorSpace`
 * @profile2: (type GdkColorSpace): another `GdkColorSpace`
 *
 * Compares two `GdkColorSpace`s for equality.
 *
 * Note that this function is not guaranteed to be perfect and two equal
 * profiles may compare not equal. However, different profiles will
 * never compare equal.
 *
 * Returns: %TRUE if the two color profiles compare equal
 *
 * Since: 4.6
 */
gboolean
gdk_color_space_equal (gconstpointer profile1,
                       gconstpointer profile2)
{
  return profile1 == profile2;
}
