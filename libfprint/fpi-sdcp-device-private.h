/*
 * FpiSdcpDevice - Secure Device Connection Protocol (SDCP) supported FpDevice
 * Copyright (C) 2025 Joshua Grisham <josh@joshuagrisham.com>
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

#pragma once

#include "fpi-sdcp-device.h"

#include <openssl/evp.h>

typedef struct
{
  EVP_PKEY *host_key;
  guchar host_private_key[SDCP_PRIVATE_KEY_SIZE];
  guchar host_public_key[SDCP_PUBLIC_KEY_SIZE];
  guchar host_random[SDCP_RANDOM_SIZE];

  guchar key_agreement[SDCP_KEY_AGREEMENT_SIZE];
  guchar master_secret[SDCP_MASTER_SECRET_SIZE];
  guchar application_secret[SDCP_APPLICATION_SECRET_SIZE];
  guchar application_symmetric_key[SDCP_APPLICATION_SYMMETRIC_KEY_SIZE];

  gboolean is_connected;
  gint64 connected_uptime;
  gint64 connected_realtime;
  gboolean supports_reconnect;

  gchar *claim_storage_path;
  gint32 claim_expiration_seconds;
} FpiSdcpDevicePrivate;

gboolean fpi_sdcp_set_host_keys (FpiSdcpDevice *device,
                                 const guchar  *host_private_key_bytes,
                                 const guchar  *host_random);

void fpi_sdcp_device_delete_cached_claim (FpiSdcpDevice *self);
