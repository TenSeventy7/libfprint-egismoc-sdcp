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
  guchar host_private_key[FP_SDCP_PRIVATE_KEY_SIZE];
  guchar host_public_key[FP_SDCP_PUBLIC_KEY_SIZE];
  guchar host_random[FP_SDCP_RANDOM_SIZE];

  guchar key_agreement[FP_SDCP_KEY_AGREEMENT_SIZE];
  guchar master_secret[FP_SDCP_MASTER_SECRET_SIZE];
  guchar application_secret[FP_SDCP_APPLICATION_SECRET_SIZE];
  guchar application_symmetric_key[FP_SDCP_APPLICATION_SYMMETRIC_KEY_SIZE];

  gboolean is_connected;
  gint64 connected_uptime;
  gint64 connected_realtime;
  gboolean supports_reconnect;

  gchar *claim_storage_path;
  gint32 claim_expiration_seconds;
} FpiSdcpDevicePrivate;
