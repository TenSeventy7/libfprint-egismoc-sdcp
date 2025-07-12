/*
 * Secure Device Connection Protocol support unit tests
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

#define FP_COMPONENT "test_sdcp_device"

#include <openssl/crypto.h>

#include "fpi-byte-writer.h"
#include "fpi-log.h"
#include "test-fpi-sdcp-device.h"


#include "test-device-fake.h"

#define DEBUG

/*********************************************************/
/* FpiTestSdcpDevice device setup ************************/
/*********************************************************/

G_DEFINE_TYPE (FpiTestSdcpDevice, fpi_test_sdcp_device, FPI_TYPE_SDCP_DEVICE)

static const FpIdEntry id_table[] = {
  { .virtual_envvar = "FP_VIRTUAL_FAKE_DEVICE" },
  { .virtual_envvar = NULL }
};

static void
fpi_test_sdcp_device_resume (FpDevice *device)
{
  fp_dbg ("Resume");
  fpi_device_resume_complete (device, NULL);
}

static void
fpi_test_sdcp_device_suspend (FpDevice *device)
{
  fp_dbg ("Suspend");
  fpi_device_suspend_complete (device, NULL);
}

static void
fpi_test_sdcp_device_clear_storage (FpDevice *device)
{
  fp_dbg ("Clear Storage");
  fpi_device_clear_storage_complete (device, NULL);
}

static void
fpi_test_sdcp_device_cancel (FpDevice *device)
{
  fp_dbg ("Cancel");
}

static void
fpi_test_sdcp_device_delete (FpDevice *device)
{
  fp_dbg ("Delete");
  fpi_device_delete_complete (device, NULL);
}

static void
fpi_test_sdcp_device_list (FpDevice *device)
{
  fp_dbg ("List");
  fpi_device_list_complete (device, NULL, NULL);
}

static void
fpi_test_sdcp_device_identify (FpDevice *device)
{
  fp_dbg ("Identify");
  fpi_device_identify_complete (device, NULL);
}

static void
fpi_test_sdcp_device_verify (FpDevice *device)
{
  fp_dbg ("Verify");
  fpi_device_verify_complete (device, NULL);
}

static void
fpi_test_sdcp_device_enroll (FpDevice *device)
{
  fp_dbg ("Enroll");
  fpi_device_enroll_complete (device, NULL, NULL);
}

static void
fpi_test_sdcp_device_close (FpDevice *device)
{
  fp_dbg ("Close");
  fpi_device_close_complete (device, NULL);
}

static void
fpi_test_sdcp_device_open (FpDevice *device)
{
  fp_dbg ("Open");
  fpi_device_open_complete (device, NULL);
}

static void
fpi_test_sdcp_device_probe (FpDevice *device)
{
  fp_dbg ("Probe");
  FpDeviceClass *dev_class = FP_DEVICE_GET_CLASS (device);
  fpi_device_probe_complete (device, dev_class->id, dev_class->full_name, NULL);
}

static void
fpi_test_sdcp_device_init (FpiTestSdcpDevice *self)
{
  G_DEBUG_HERE ();
}

#define SDCP_TEST_CLAIM_EXPIRE_SECS 2

static void
fpi_test_sdcp_device_class_init (FpiTestSdcpDeviceClass *klass)
{
  FpDeviceClass *dev_class = FP_DEVICE_CLASS (klass);
  FpiSdcpDeviceClass *sdcp_dev_class = FPI_SDCP_DEVICE_CLASS (klass);

  dev_class->id = FP_COMPONENT;
  dev_class->full_name = "Virtual SDCP test device";

  dev_class->type = FP_DEVICE_TYPE_VIRTUAL;
  dev_class->scan_type = FP_SCAN_TYPE_PRESS;
  dev_class->id_table = id_table;
  dev_class->nr_enroll_stages = 5;

  dev_class->probe = fpi_test_sdcp_device_probe;
  dev_class->open = fpi_test_sdcp_device_open;
  dev_class->close = fpi_test_sdcp_device_close;
  dev_class->enroll = fpi_test_sdcp_device_enroll;
  dev_class->verify = fpi_test_sdcp_device_verify;
  dev_class->identify = fpi_test_sdcp_device_identify;
  dev_class->list = fpi_test_sdcp_device_list;
  dev_class->delete = fpi_test_sdcp_device_delete;
  dev_class->cancel = fpi_test_sdcp_device_cancel;
  dev_class->clear_storage = fpi_test_sdcp_device_clear_storage;
  dev_class->suspend = fpi_test_sdcp_device_suspend;
  dev_class->resume = fpi_test_sdcp_device_resume;

  fpi_device_class_auto_initialize_features (dev_class);

  sdcp_dev_class->supports_reconnect = TRUE;
  /* set a fast expiration time to support unit testing of expiration */
  sdcp_dev_class->claim_expiration_seconds = SDCP_TEST_CLAIM_EXPIRE_SECS;
}

/*********************************************************/
/* Test data setup ***************************************/
/*********************************************************/

static FpiSdcpConnectResponse *
sdcp_test_get_test_response (void)
{
  FpiSdcpConnectResponse *response = g_new0 (FpiSdcpConnectResponse, 1);

  memcpy (response->device_random,
    &TEST_DEVICE_RANDOM, sizeof (TEST_DEVICE_RANDOM));

  response->model_certificate = g_malloc0 (sizeof (TEST_CERT));
  memcpy (response->model_certificate,
    &TEST_CERT, sizeof (TEST_CERT));
  response->model_certificate_len = sizeof (TEST_CERT);

  memcpy (response->device_public_key,
    &TEST_DEVICE_PUBLIC_KEY, sizeof (TEST_DEVICE_PUBLIC_KEY));

  memcpy (response->firmware_public_key,
    &TEST_DEVICE_FIRMWARE_PUBLIC_KEY, sizeof (TEST_DEVICE_FIRMWARE_PUBLIC_KEY));

  memcpy (response->firmware_hash,
    &TEST_DEVICE_FIRMWARE_HASH, sizeof (TEST_DEVICE_FIRMWARE_HASH));

  memcpy (response->model_signature,
    &TEST_DEVICE_MODEL_SIGNATURE, sizeof (TEST_DEVICE_MODEL_SIGNATURE));

  memcpy (response->device_signature,
    &TEST_DEVICE_DEVICE_SIGNATURE, sizeof (TEST_DEVICE_DEVICE_SIGNATURE));

  memcpy (response->mac,
    &TEST_DEVICE_CONNECT_MAC, sizeof (TEST_DEVICE_CONNECT_MAC));

  return g_steal_pointer (&response);
}

/*********************************************************/
/* Tests *************************************************/
/*********************************************************/

static void
sdcp_test_get_cert_length_from_buf (void)
{
  const guchar extra_bytes[] = "abcde";
  g_autofree guchar *cert_w_extra = g_malloc0 (sizeof (TEST_CERT) + sizeof (extra_bytes));

  memcpy (cert_w_extra, TEST_CERT, sizeof (TEST_CERT));
  memcpy (cert_w_extra + sizeof (TEST_CERT), extra_bytes, sizeof (extra_bytes));

  fp_dbg ("cert length is: %ld", sizeof (TEST_CERT));
  fp_dbg ("extra bytes length is: %ld", sizeof (extra_bytes));

  int len = fpi_sdcp_get_cert_length_from_buf (cert_w_extra);

  fp_dbg ("total cert_w_extra length is: %ld", sizeof (TEST_CERT) + sizeof (extra_bytes));
  fp_dbg ("discovered cert length from fpi_sdcp_get_cert_length_from_buf is: %d", len);
  fp_dbg ("Expected number of extra bytes: %ld", sizeof (extra_bytes));
  fp_dbg ("Actual number of extra bytes: %ld", (sizeof (TEST_CERT) + sizeof (extra_bytes)) - len);

  g_assert (len == sizeof (TEST_CERT));
}

static void
sdcp_test_generate_enrollment_id (void)
{
  g_autoptr(FpDevice) device = g_object_new (FPI_TYPE_TEST_SDCP_DEVICE, NULL);
  FpiSdcpDevice *sdcp_dev = FPI_SDCP_DEVICE (device);
  g_autoptr(FpiSdcpConnectResponse) response = sdcp_test_get_test_response();
  g_autofree guchar *enrollment_id = NULL;
  gchar *hex = NULL;

  fp_device_open_sync (device, NULL, NULL);

  /* claim should have been cached from prior test */
  g_assert (fpi_sdcp_device_is_connected (sdcp_dev));

  enrollment_id = fpi_sdcp_generate_enrollment_id (sdcp_dev, TEST_ENROLLMENT_NONCE);

  hex = OPENSSL_buf2hexstr (TEST_ENROLLMENT_ENROLLMENT_ID, SDCP_ENROLLMENT_ID_SIZE);
  fp_dbg ("Expected enrollment_id:\n%s", hex);
  g_free (hex);

  hex = OPENSSL_buf2hexstr (enrollment_id, SDCP_ENROLLMENT_ID_SIZE);
  fp_dbg ("Actual enrollment_id:\n%s", hex);
  g_free (hex);

  g_assert_cmpmem (enrollment_id, SDCP_ENROLLMENT_ID_SIZE,
    TEST_ENROLLMENT_ENROLLMENT_ID, SDCP_ENROLLMENT_ID_SIZE);

  /* clean up cached claim after last test */
  fpi_sdcp_device_delete_cached_claim (sdcp_dev);

  fp_device_close_sync (device, NULL, NULL);
}

static void
sdcp_test_generate_random (void)
{
  g_autofree guchar *random = fpi_sdcp_generate_random ();
  g_autofree guchar *random_zeroes = g_malloc0 (SDCP_RANDOM_SIZE);
  g_autofree gchar *hex = OPENSSL_buf2hexstr (random, SDCP_RANDOM_SIZE);

  fp_dbg ("Generated random:\n%s", hex);

  g_assert (memcmp (random, random_zeroes, SDCP_RANDOM_SIZE) != 0);
}

static void
sdcp_test_verify_authorized_identity (void)
{
  g_autoptr(FpDevice) device = g_object_new (FPI_TYPE_TEST_SDCP_DEVICE, NULL);
  FpiSdcpDevice *sdcp_dev = FPI_SDCP_DEVICE (device);
  g_autoptr(FpiSdcpConnectResponse) response = sdcp_test_get_test_response();

  fp_device_open_sync (device, NULL, NULL);

  /* claim should have been cached from prior test */
  g_assert (fpi_sdcp_device_is_connected (sdcp_dev));

  g_assert (fpi_sdcp_verify_authorized_identity (sdcp_dev, TEST_IDENTIFY_NONCE,
    TEST_IDENTIFY_ENROLLMENT_ID, TEST_IDENTIFY_MAC));

  fp_device_close_sync (device, NULL, NULL);

}

static void
sdcp_test_verify_reconnect (void)
{
  g_autoptr(FpDevice) device = g_object_new (FPI_TYPE_TEST_SDCP_DEVICE, NULL);
  FpiSdcpDevice *sdcp_dev = FPI_SDCP_DEVICE (device);
  g_autoptr(FpiSdcpConnectResponse) response = sdcp_test_get_test_response();

  fp_device_open_sync (device, NULL, NULL);

  /* claim should have been cached from prior test */
  g_assert (fpi_sdcp_device_is_connected (sdcp_dev));

  g_assert (fpi_sdcp_verify_reconnect (sdcp_dev, TEST_RECONNECT_RANDOM, TEST_RECONNECT_MAC));

  fp_device_close_sync (device, NULL, NULL);
}

static void
sdcp_test_verify_connect_buf (void)
{
  g_auto(FpiByteWriter) writer = {0};
  gboolean written = TRUE;
  g_autofree guchar *buf;
  int buf_len;

  g_autoptr(FpDevice) device = g_object_new (FPI_TYPE_TEST_SDCP_DEVICE, NULL);
  FpiSdcpDevice *sdcp_dev = FPI_SDCP_DEVICE (device);

  buf_len = sizeof (TEST_DEVICE_RANDOM)
            + sizeof (TEST_CERT)
            + sizeof (TEST_DEVICE_PUBLIC_KEY)
            + sizeof (TEST_DEVICE_FIRMWARE_PUBLIC_KEY)
            + sizeof (TEST_DEVICE_FIRMWARE_HASH)
            + sizeof (TEST_DEVICE_MODEL_SIGNATURE)
            + sizeof (TEST_DEVICE_DEVICE_SIGNATURE)
            + sizeof (TEST_DEVICE_CONNECT_MAC);

  /* pre-fill writer buffer with 00s */
  fpi_byte_writer_init_with_size (&writer, buf_len, TRUE);

  written &= fpi_byte_writer_put_data (&writer, TEST_DEVICE_RANDOM,
                                       sizeof (TEST_DEVICE_RANDOM));

  written &= fpi_byte_writer_put_data (&writer, TEST_CERT,
                                       sizeof (TEST_CERT));

  written &= fpi_byte_writer_put_data (&writer, TEST_DEVICE_PUBLIC_KEY,
                                       sizeof (TEST_DEVICE_PUBLIC_KEY));

  written &= fpi_byte_writer_put_data (&writer, TEST_DEVICE_FIRMWARE_PUBLIC_KEY,
                                       sizeof (TEST_DEVICE_FIRMWARE_PUBLIC_KEY));

  written &= fpi_byte_writer_put_data (&writer, TEST_DEVICE_FIRMWARE_HASH,
                                       sizeof (TEST_DEVICE_FIRMWARE_HASH));

  written &= fpi_byte_writer_put_data (&writer, TEST_DEVICE_MODEL_SIGNATURE,
                                       sizeof (TEST_DEVICE_MODEL_SIGNATURE));

  written &= fpi_byte_writer_put_data (&writer, TEST_DEVICE_DEVICE_SIGNATURE,
                                       sizeof (TEST_DEVICE_DEVICE_SIGNATURE));

  written &= fpi_byte_writer_put_data (&writer, TEST_DEVICE_CONNECT_MAC,
                                       sizeof (TEST_DEVICE_CONNECT_MAC));

  g_assert (written);

  buf = fpi_byte_writer_reset_and_get_data (&writer);

  fp_device_open_sync (device, NULL, NULL);

  fpi_sdcp_device_delete_cached_claim (sdcp_dev);

  g_assert (fpi_sdcp_set_host_keys (sdcp_dev, TEST_HOST_PRIVATE_KEY, TEST_HOST_RANDOM));
  g_assert (fpi_sdcp_derive_keys_and_verify_connect_buf (sdcp_dev, buf, buf_len));
  g_assert (fpi_sdcp_device_is_connected (sdcp_dev));

  fp_device_close_sync (device, NULL, NULL);
}

static void
sdcp_test_verify_connect_ex (void)
{
  g_autoptr(FpDevice) device = g_object_new (FPI_TYPE_TEST_SDCP_DEVICE, NULL);
  FpiSdcpDevice *sdcp_dev = FPI_SDCP_DEVICE (device);

  fp_device_open_sync (device, NULL, NULL);

  fpi_sdcp_device_delete_cached_claim (sdcp_dev);

  g_assert (fpi_sdcp_set_host_keys (sdcp_dev, TEST_HOST_PRIVATE_KEY, TEST_HOST_RANDOM));

  g_assert (fpi_sdcp_derive_keys_and_verify_connect_ex (sdcp_dev,
    TEST_DEVICE_RANDOM,
    TEST_CERT, sizeof (TEST_CERT),
    TEST_DEVICE_PUBLIC_KEY,
    TEST_DEVICE_FIRMWARE_PUBLIC_KEY,
    TEST_DEVICE_FIRMWARE_HASH,
    TEST_DEVICE_MODEL_SIGNATURE,
    TEST_DEVICE_DEVICE_SIGNATURE,
    TEST_DEVICE_CONNECT_MAC));
  g_assert (fpi_sdcp_device_is_connected (sdcp_dev));

  fp_device_close_sync (device, NULL, NULL);
}

static void
sdcp_test_expired_claim (void)
{
  g_autoptr(FpDevice) device = g_object_new (FPI_TYPE_TEST_SDCP_DEVICE, NULL);
  FpiSdcpDevice *sdcp_dev = FPI_SDCP_DEVICE (device);
  g_autoptr(FpiSdcpConnectResponse) response = sdcp_test_get_test_response();
  gint64 waitusecs;

  fp_device_open_sync (device, NULL, NULL);

  /* claim should have been cached from sdcp_test_verify_connect */
  g_assert (fpi_sdcp_device_is_connected (sdcp_dev));

  waitusecs = (SDCP_TEST_CLAIM_EXPIRE_SECS + 1) * G_USEC_PER_SEC;
  fp_dbg ("Waiting %ld seconds for the claim to expire...", waitusecs / G_USEC_PER_SEC);
  g_usleep (waitusecs);

  /* claim should now be expired */
  g_assert (!fpi_sdcp_device_is_connected (sdcp_dev));

  fp_device_close_sync (device, NULL, NULL);
}

static void
sdcp_test_verify_connect_cached (void)
{
  g_autoptr(FpDevice) device = g_object_new (FPI_TYPE_TEST_SDCP_DEVICE, NULL);
  FpiSdcpDevice *sdcp_dev = FPI_SDCP_DEVICE (device);
  g_autoptr(FpiSdcpConnectResponse) response = sdcp_test_get_test_response();

  fp_device_open_sync (device, NULL, NULL);

  /* claim should have been cached from sdcp_test_verify_connect */
  g_assert (fpi_sdcp_device_is_connected (sdcp_dev));

  fp_device_close_sync (device, NULL, NULL);
}

static void
sdcp_test_verify_connect (void)
{
  g_autoptr(FpDevice) device = g_object_new (FPI_TYPE_TEST_SDCP_DEVICE, NULL);
  FpiSdcpDevice *sdcp_dev = FPI_SDCP_DEVICE (device);
  g_autoptr(FpiSdcpConnectResponse) response = sdcp_test_get_test_response();

  fp_device_open_sync (device, NULL, NULL);

  g_assert (fpi_sdcp_set_host_keys (sdcp_dev, TEST_HOST_PRIVATE_KEY, TEST_HOST_RANDOM));
  g_assert (!fpi_sdcp_device_is_connected (sdcp_dev));

  g_assert (fpi_sdcp_derive_keys_and_verify_connect (sdcp_dev, response));
  g_assert (fpi_sdcp_device_is_connected (sdcp_dev));

  fp_device_close_sync (device, NULL, NULL);
}

static void
sdcp_test_get_host_random (void)
{
  g_autoptr(FpDevice) device = g_object_new (FPI_TYPE_TEST_SDCP_DEVICE, NULL);
  FpiSdcpDevice *sdcp_dev = FPI_SDCP_DEVICE (device);
  g_autoptr(FpiSdcpConnectResponse) response = sdcp_test_get_test_response();
  g_autofree guchar* random = NULL;
  gchar *hex = NULL;

  fp_device_open_sync (device, NULL, NULL);

  g_assert (fpi_sdcp_set_host_keys (sdcp_dev, TEST_HOST_PRIVATE_KEY, TEST_HOST_RANDOM));

  random = fpi_sdcp_get_host_random (sdcp_dev);

  hex = OPENSSL_buf2hexstr (TEST_HOST_RANDOM, SDCP_RANDOM_SIZE);
  fp_dbg ("Expected random:\n%s", hex);
  g_free (hex);

  hex = OPENSSL_buf2hexstr (random, SDCP_RANDOM_SIZE);
  fp_dbg ("Actual random:\n%s", hex);
  g_free (hex);

  g_assert_cmpmem (TEST_HOST_RANDOM, SDCP_RANDOM_SIZE, random, SDCP_RANDOM_SIZE);

  fp_device_close_sync (device, NULL, NULL);
}

static void
sdcp_test_get_host_public_key (void)
{
  g_autoptr(FpDevice) device = g_object_new (FPI_TYPE_TEST_SDCP_DEVICE, NULL);
  FpiSdcpDevice *sdcp_dev = FPI_SDCP_DEVICE (device);
  g_autoptr(FpiSdcpConnectResponse) response = sdcp_test_get_test_response();
  g_autofree guchar* public_key = NULL;
  gchar *hex = NULL;

  fp_device_open_sync (device, NULL, NULL);

  g_assert (fpi_sdcp_set_host_keys (sdcp_dev, TEST_HOST_PRIVATE_KEY, TEST_HOST_RANDOM));

  public_key = fpi_sdcp_get_host_public_key (sdcp_dev);

  hex = OPENSSL_buf2hexstr (TEST_HOST_PUBLIC_KEY, SDCP_PUBLIC_KEY_SIZE);
  fp_dbg ("Expected public_key:\n%s", hex);
  g_free (hex);

  hex = OPENSSL_buf2hexstr (public_key, SDCP_PUBLIC_KEY_SIZE);
  fp_dbg ("Actual public_key:\n%s", hex);
  g_free (hex);
  
  g_assert_cmpmem (TEST_HOST_PUBLIC_KEY, SDCP_PUBLIC_KEY_SIZE,
    public_key, SDCP_PUBLIC_KEY_SIZE);

  fp_device_close_sync (device, NULL, NULL);
}

static void
sdcp_test_set_host_keys (void)
{
  g_autoptr(FpDevice) device = g_object_new (FPI_TYPE_TEST_SDCP_DEVICE, NULL);
  FpiSdcpDevice *sdcp_dev = FPI_SDCP_DEVICE (device);
  g_autoptr(FpiSdcpConnectResponse) response = sdcp_test_get_test_response();

  fp_device_open_sync (device, NULL, NULL);

  g_assert (fpi_sdcp_set_host_keys (sdcp_dev, TEST_HOST_PRIVATE_KEY, TEST_HOST_RANDOM));
  g_assert (!fpi_sdcp_device_is_connected (sdcp_dev));

  fp_device_close_sync (device, NULL, NULL);
}

static void
sdcp_test_new_device (void)
{
  g_autoptr(FpDevice) device = g_object_new (FPI_TYPE_TEST_SDCP_DEVICE, NULL);
  FpiSdcpDevice *sdcp_dev = FPI_SDCP_DEVICE (device);

  fp_device_open_sync (device, NULL, NULL);

  /* before any tests, remove cached claim if it exists */
  fpi_sdcp_device_delete_cached_claim (sdcp_dev);

  g_assert (!fpi_sdcp_device_is_connected (sdcp_dev));

  fp_device_close_sync (device, NULL, NULL);
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/sdcp/new_device", sdcp_test_new_device);
  g_test_add_func ("/sdcp/set_host_keys", sdcp_test_set_host_keys);
  g_test_add_func ("/sdcp/get_host_public_key", sdcp_test_get_host_public_key);
  g_test_add_func ("/sdcp/get_host_random", sdcp_test_get_host_random);
  g_test_add_func ("/sdcp/verify_connect", sdcp_test_verify_connect);
  g_test_add_func ("/sdcp/verify_connect_cached", sdcp_test_verify_connect_cached);
  g_test_add_func ("/sdcp/expired_claim", sdcp_test_expired_claim);
  g_test_add_func ("/sdcp/verify_connect_ex", sdcp_test_verify_connect_ex);
  g_test_add_func ("/sdcp/verify_connect_buf", sdcp_test_verify_connect_buf);
  g_test_add_func ("/sdcp/verify_reconnect", sdcp_test_verify_reconnect);
  g_test_add_func ("/sdcp/verify_authorized_identity", sdcp_test_verify_authorized_identity);
  g_test_add_func ("/sdcp/generate_random", sdcp_test_generate_random);
  g_test_add_func ("/sdcp/generate_enrollment_id", sdcp_test_generate_enrollment_id);
  g_test_add_func ("/sdcp/get_cert_length_from_buf", sdcp_test_get_cert_length_from_buf);

  return g_test_run ();
}
