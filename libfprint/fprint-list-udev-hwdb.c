/*
 * Copyright (C) 2009 Red Hat <mjg@redhat.com>
 * Copyright (C) 2008 Bastien Nocera <hadess@hadess.net>
 * Copyright (C) 2008 Timo Hoenig <thoenig@suse.de>, <thoenig@nouse.net>
 * Copyright (C) 2019 Benjamin Berg <bberg@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <config.h>

#include "fpi-context.h"
#include "fpi-device.h"

static const FpIdEntry allowlist_id_table[] = {
  /* Currently known and unsupported devices.
   * You can generate this list from the wiki page using e.g.:
   *   gio cat https://gitlab.freedesktop.org/libfprint/wiki/-/wikis/Unsupported-Devices.md | sed -n 's!|.*\([0-9a-fA-F]\{4\}\):\([0-9a-fA-F]\{4\}\).*|.*!  { .vid = 0x\1, .pid = 0x\2 },!p'
   */
  { .vid = 0x0a5c, .pid = 0x5802 },
  { .vid = 0x047d, .pid = 0x00f2 },
  { .vid = 0x047d, .pid = 0x8054 },
  { .vid = 0x047d, .pid = 0x8055 },
  { .vid = 0x04e8, .pid = 0x730b },
  { .vid = 0x04f3, .pid = 0x036b },
  { .vid = 0x04f3, .pid = 0x0c00 },
  { .vid = 0x04f3, .pid = 0x0c4c },
  { .vid = 0x04f3, .pid = 0x0c57 },
  { .vid = 0x04f3, .pid = 0x0c5e },
  { .vid = 0x04f3, .pid = 0x0c5a },
  { .vid = 0x04f3, .pid = 0x0c60 },
  { .vid = 0x04f3, .pid = 0x0c6c },
  { .vid = 0x04f3, .pid = 0x0c70 },
  { .vid = 0x04f3, .pid = 0x0c72 },
  { .vid = 0x04f3, .pid = 0x0c77 },
  { .vid = 0x04f3, .pid = 0x0c7c },
  { .vid = 0x04f3, .pid = 0x0c7f },
  { .vid = 0x04f3, .pid = 0x0c80 },
  { .vid = 0x04f3, .pid = 0x0c85 },
  { .vid = 0x04f3, .pid = 0x0c90 },
  { .vid = 0x04f3, .pid = 0x2706 },
  { .vid = 0x04f3, .pid = 0x3032 },
  { .vid = 0x04f3, .pid = 0x3057 },
  { .vid = 0x04f3, .pid = 0x3104 },
  { .vid = 0x04f3, .pid = 0x310d },
  { .vid = 0x04f3, .pid = 0x3128 },
  { .vid = 0x04f3, .pid = 0x0c8a },
  { .vid = 0x05ba, .pid = 0x000e },
  { .vid = 0x06cb, .pid = 0x0051 },
  { .vid = 0x06cb, .pid = 0x0081 },
  { .vid = 0x06cb, .pid = 0x0088 },
  { .vid = 0x06cb, .pid = 0x008a },
  { .vid = 0x06cb, .pid = 0x009a },
  { .vid = 0x06cb, .pid = 0x009b },
  { .vid = 0x06cb, .pid = 0x00a2 },
  { .vid = 0x06cb, .pid = 0x00a8 },
  { .vid = 0x06cb, .pid = 0x00b7 },
  { .vid = 0x06cb, .pid = 0x00bb },
  { .vid = 0x06cb, .pid = 0x00be },
  { .vid = 0x06cb, .pid = 0x00cb },
  { .vid = 0x06cb, .pid = 0x00c9 },
  { .vid = 0x06cb, .pid = 0x00d8 },
  { .vid = 0x06cb, .pid = 0x00da },
  { .vid = 0x06cb, .pid = 0x00dc },
  { .vid = 0x06cb, .pid = 0x00e4 },
  { .vid = 0x06cb, .pid = 0x00e7 },
  { .vid = 0x06cb, .pid = 0x00e9 },
  { .vid = 0x06cb, .pid = 0x00fd },
  { .vid = 0x06cb, .pid = 0x00ff },
  { .vid = 0x0a5c, .pid = 0x5801 },
  { .vid = 0x0a5c, .pid = 0x5805 },
  { .vid = 0x0a5c, .pid = 0x5834 },
  { .vid = 0x0a5c, .pid = 0x5840 },
  { .vid = 0x0a5c, .pid = 0x5841 },
  { .vid = 0x0a5c, .pid = 0x5842 },
  { .vid = 0x0a5c, .pid = 0x5843 },
  { .vid = 0x0a5c, .pid = 0x5844 },
  { .vid = 0x0a5c, .pid = 0x5845 },
  { .vid = 0x0a5c, .pid = 0x5860 },
  { .vid = 0x0a5c, .pid = 0x5863 },
  { .vid = 0x0a5c, .pid = 0x5864 },
  { .vid = 0x0a5c, .pid = 0x5865 },
  { .vid = 0x0a5c, .pid = 0x5866 },
  { .vid = 0x0a5c, .pid = 0x5867 },
  { .vid = 0x0bda, .pid = 0x5812 },
  { .vid = 0x10a5, .pid = 0x0007 },
  { .vid = 0x10a5, .pid = 0x9200 },
  { .vid = 0x10a5, .pid = 0x9800 },
  { .vid = 0x10a5, .pid = 0xa120 },
  { .vid = 0x10a5, .pid = 0xa900 },
  { .vid = 0x10a5, .pid = 0xe340 },
  { .vid = 0x1188, .pid = 0x9545 },
  { .vid = 0x138a, .pid = 0x0007 },
  { .vid = 0x138a, .pid = 0x003a },
  { .vid = 0x138a, .pid = 0x003c },
  { .vid = 0x138a, .pid = 0x003d },
  { .vid = 0x138a, .pid = 0x003f },
  { .vid = 0x138a, .pid = 0x0090 },
  { .vid = 0x138a, .pid = 0x0092 },
  { .vid = 0x138a, .pid = 0x0094 },
  { .vid = 0x138a, .pid = 0x0097 },
  { .vid = 0x138a, .pid = 0x009d },
  { .vid = 0x138a, .pid = 0x00ab },
  { .vid = 0x138a, .pid = 0x00a6 },
  { .vid = 0x147e, .pid = 0x1002 },
  { .vid = 0x1491, .pid = 0x0088 },
  { .vid = 0x16d1, .pid = 0x1027 },
  { .vid = 0x1c7a, .pid = 0x0300 },
  { .vid = 0x1c7a, .pid = 0x0575 },
  { .vid = 0x1c7a, .pid = 0x0576 },
  { .vid = 0x1c7a, .pid = 0x0577 },
  { .vid = 0x1c7a, .pid = 0x057e },
  { .vid = 0x2541, .pid = 0x0236 },
  { .vid = 0x2541, .pid = 0x9711 },
  { .vid = 0x27c6, .pid = 0x5042 },
  { .vid = 0x27c6, .pid = 0x5110 },
  { .vid = 0x27c6, .pid = 0x5117 },
  { .vid = 0x27c6, .pid = 0x5120 },
  { .vid = 0x27c6, .pid = 0x5125 },
  { .vid = 0x27c6, .pid = 0x5201 },
  { .vid = 0x27c6, .pid = 0x521d },
  { .vid = 0x27c6, .pid = 0x5301 },
  { .vid = 0x27c6, .pid = 0x530c },
  { .vid = 0x27c6, .pid = 0x532d },
  { .vid = 0x27c6, .pid = 0x5335 },
  { .vid = 0x27c6, .pid = 0x533c },
  { .vid = 0x27c6, .pid = 0x5381 },
  { .vid = 0x27c6, .pid = 0x5385 },
  { .vid = 0x27c6, .pid = 0x538c },
  { .vid = 0x27c6, .pid = 0x538d },
  { .vid = 0x27c6, .pid = 0x5395 },
  { .vid = 0x27c6, .pid = 0x5503 },
  { .vid = 0x27c6, .pid = 0x550a },
  { .vid = 0x27c6, .pid = 0x550c },
  { .vid = 0x27c6, .pid = 0x5584 },
  { .vid = 0x27c6, .pid = 0x55a2 },
  { .vid = 0x27c6, .pid = 0x55a4 },
  { .vid = 0x27c6, .pid = 0x55b4 },
  { .vid = 0x27c6, .pid = 0x5740 },
  { .vid = 0x27c6, .pid = 0x5e0a },
  { .vid = 0x27c6, .pid = 0x581a },
  { .vid = 0x27c6, .pid = 0x589a },
  { .vid = 0x27c6, .pid = 0x5f10 },
  { .vid = 0x27c6, .pid = 0x6382 },
  { .vid = 0x2808, .pid = 0x9338 },
  { .vid = 0x2808, .pid = 0x9348 },
  { .vid = 0x2808, .pid = 0x93a9 },
  { .vid = 0x2808, .pid = 0xa658 },
  { .vid = 0x2808, .pid = 0xc652 },
  { .vid = 0x298d, .pid = 0x2020 },
  { .vid = 0x298d, .pid = 0x2033 },
  { .vid = 0x2df0, .pid = 0x0003 },
  { .vid = 0x3274, .pid = 0x8012 },
  { .vid = 0x3538, .pid = 0x0930 },
  { .vid = 0 },
};

static const FpIdEntry denylist_id_table[] = {
  { .vid = 0x0483, .pid = 0x2016 },
  /* https://bugs.freedesktop.org/show_bug.cgi?id=66659 */
  { .vid = 0x045e, .pid = 0x00bb },
  { .vid = 0 },
};

static const FpDeviceClass allowlist = {
  .type = FP_DEVICE_TYPE_USB,
  .id_table = allowlist_id_table,
  .id = "allowlist",
  .full_name = "Hardcoded allowlist"
};

GHashTable *printed = NULL;

static void
print_driver (const FpDeviceClass *cls)
{
  const FpIdEntry *entry;
  gint num_printed = 0;

  if (cls->type != FP_DEVICE_TYPE_USB)
    return;

  for (entry = cls->id_table; entry->vid != 0; entry++)
    {
      const FpIdEntry *bl_entry;
      char *key;

      for (bl_entry = denylist_id_table; bl_entry->vid != 0; bl_entry++)
        if (entry->vid == bl_entry->vid && entry->pid == bl_entry->pid)
          break;

      if (bl_entry->vid != 0)
        continue;

      key = g_strdup_printf ("%04x:%04x", entry->vid, entry->pid);

      if (g_hash_table_lookup (printed, key) != NULL)
        {
          if (cls == &allowlist)
            g_warning ("%s implemented by driver %s",
                       key, (const char *) g_hash_table_lookup (printed, key));
          g_free (key);
          continue;
        }

      g_hash_table_insert (printed, key, (void *) cls->id);

      if (num_printed == 0)
        {
          if (cls != &allowlist)
            g_print ("\n# Supported by libfprint driver %s\n", cls->id);
          else
            g_print ("\n# Known unsupported devices\n");
        }

      g_print ("usb:v%04Xp%04X*\n",
               entry->vid, entry->pid);
      num_printed++;
    }

  if (num_printed > 0)
    {
      g_print (" ID_AUTOSUSPEND=1\n");
      g_print (" ID_PERSIST=0\n");
    }
}

static int
driver_compare (gconstpointer p1, gconstpointer p2)
{
  g_autoptr(FpDeviceClass) cls1 = g_type_class_ref (*(GType *) p1);
  g_autoptr(FpDeviceClass) cls2 = g_type_class_ref (*(GType *) p2);

  return g_strcmp0 (cls1->id, cls2->id);
}

int
main (int argc, char **argv)
{
  g_autoptr(GArray) drivers = fpi_get_driver_types ();
  g_autofree char *program_name = NULL;
  guint i;

  program_name = g_path_get_basename (argv[0]);

  g_print ("# SPDX-License-Identifier: LGPL-2.1-or-later\n");
  g_print ("# This file has been generated using %s with all drivers enabled\n",
           program_name);

  printed = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  g_array_sort (drivers, driver_compare);

  for (i = 0; i < drivers->len; i++)
    {
      GType driver = g_array_index (drivers, GType, i);
      g_autoptr(FpDeviceClass) cls = g_type_class_ref (driver);

      if (cls->type != FP_DEVICE_TYPE_USB)
        continue;

      print_driver (cls);
    }

  print_driver (&allowlist);

  g_hash_table_destroy (printed);

  return 0;
}
