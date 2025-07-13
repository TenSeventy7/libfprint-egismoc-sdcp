#!/usr/bin/python3

# In order for all of the payloads to match, the SDCP connection claim from when the capture is
# taken needs to be the same as every time that the test is executed.
# For that, we will need to cache the claim that is used by the unit test, so we can build the file
# in this script and create it where the driver will pick it up every time the test is run.
# However, if we create the SDCP connection during capture, but use a cached claim during the test,
# then the driver-to-device workflow is different (we do not execute SDCP Connect command when using
# an already cached claim, so that packet from the capture will not match what the driver does in
# the test, so the test execution never matches the captured packets--it does not work!).
# So, this means that we need to create a cached claim before even capturing, and then save that
# claim here so that the same cached claim will be used both during capture as well as all
# test executions.
# Unfortunately this means that we will not be able to test the SDCP Connect command as part of this
# test. But perhaps we can create a second test that only tests SDCP Connect and nothing else?

# Here is how you can create your own for other variants that should work:
# 1. Wipe any existing cached claims under temp:
#        sudo rm -rf /tmp/.libfprint
# 2. Use the example app ./examples/enroll to enroll one or more prints
#        sudo ./examples/enroll
# 3. Copy the bytes from the latest "valid" cached claim to the claim_bytes value below. You can get
#    the value using a command like:
#        sudo hexdump -e '16/1 "%02x " "\n"' $(sudo find /tmp/.libfprint/egismoc/ -name sdcp-claim)

claim_bytes = bytes.fromhex("""
a6 12 97 2a 13 00 00 00
1c 72 86 5a fe 85 3a 28 68 1d 88 b3 0c cb 81 0b
84 19 75 38 b2 43 c7 57 60 27 05 66 ab ab d8 b7
04 2c 92 51 13 83 c8 aa 32 ee 51 ef 49 95 7a 6f
84 d3 03 1b 58 9c a1 6b 55 d1 48 37 8e c0 9e ba
a1 12 9c 4a 28 f8 e4 f9 5d 4d f4 e6 4d 4b e5 e7
85 7f 5c be be e4 2e 17 cc ad 87 b0 0f 0a c3 2d
2d 37 c9 ae 1e 83 39 16 8d 4c 7c 53 a1 89 f0 b1
db 89 f5 6b 73 0a d6 b9 bf 94 5e b3 da 9d 6e ca
c9 20 8c 5a ef ed 97 04 f1 7d ca d9 85 f1 31 8e
49 e5 1d 48 fa b9 ed 89 26 64 dc a5 e7 42 cd 94
e9 94 b6 64 33 bd 5c f7 e4 37 c4 fd 61 cb 58 09
8c 22 c6 c8 72 6a f4 df 69 6d 16 5a b6 fc 68 3c
b5 30 6d 02 f5 de ae 35 ba d2 ab 91 06 75 28 74
00 0c bb 99 62 40 7d 66 3e 46 be 11 7c e3 a2 c8
ff 59 7f f3 66 ed 13 a1 b6 91 a8 72 81 d9 f6 04
4c a4 b2 ad 13 b4 aa b9 4a b3 f2 73 2c 67 3c 13
92 e9 00 c9 00 a9 00 89 00 69 00 28 00
""")

# 4. Wipe the claims cache folder:
#      sudo rm -rf /tmp/.libfprint
# 5. Run the capture script
#      sudo tests/create-driver-test.py egismoc VARIANT
# 6. Wipe the claims cache folder again:
#      sudo rm -rf /tmp/.libfprint
# 7. Run the test and ensure it works:
#      meson test egismoc-VARIANT --verbose

# Now, on to the test script!!

import traceback
import sys
import time
import gi
from math import trunc
from os import makedirs, path, remove
from tempfile import gettempdir

gi.require_version('FPrint', '2.0')
from gi.repository import FPrint, GLib

# Exit with error on any exception, included those happening in async callbacks
sys.excepthook = lambda *args: (traceback.print_exception(*args), sys.exit(1))

ctx = GLib.main_context_default()

c = FPrint.Context()
c.enumerate()
devices = c.get_devices()

d = devices[0]
del devices

# We will need to set a new claim connected_time in the above claim bytes so that libfprint will not
# see the cached claim as "expired". This is an int64 number in little endian.

connected_time = GLib.get_monotonic_time()
connected_time_bytes = connected_time.to_bytes(8, byteorder = 'little')

# Swap out connected_time in the above payload with our new value
claim_bytes = connected_time_bytes + claim_bytes[8:]

# Now we will pre-cache this claim file so that the test script will re-use the same SDCP claim as the capture
claim_file_path = path.join(gettempdir(), ".libfprint", "egismoc", "emulated-device")
makedirs(claim_file_path, exist_ok=True)
claim_file_path = path.join(claim_file_path, "sdcp-claim")
with open(claim_file_path, "wb") as claim_file:
    claim_file.write(claim_bytes)

d.open_sync()

assert d.get_driver() == "egismoc"
assert not d.has_feature(FPrint.DeviceFeature.CAPTURE)
assert d.has_feature(FPrint.DeviceFeature.IDENTIFY)
assert d.has_feature(FPrint.DeviceFeature.VERIFY)
assert d.has_feature(FPrint.DeviceFeature.DUPLICATES_CHECK)
assert d.has_feature(FPrint.DeviceFeature.STORAGE)
assert d.has_feature(FPrint.DeviceFeature.STORAGE_LIST)
assert d.has_feature(FPrint.DeviceFeature.STORAGE_DELETE)
assert d.has_feature(FPrint.DeviceFeature.STORAGE_CLEAR)

def enroll_progress(*args):
    print("finger status: ", d.get_finger_status())
    print('enroll progress: ' + str(args))

def identify_done(dev, res):
    global identified
    identified = True
    identify_match, identify_print = dev.identify_finish(res)
    print('indentification_done: ', identify_match, identify_print)
    assert identify_match.equal(identify_print)

# Beginning with list and clear assumes you begin with >0 prints enrolled before capturing

print("listing - device should have prints")
stored = d.list_prints_sync()
assert len(stored) > 0
del stored

print("clear device storage")
d.clear_storage_sync()
print("clear done")

print("listing - device should be empty")
stored = d.list_prints_sync()
assert len(stored) == 0
del stored

print("enrolling")
template = FPrint.Print.new(d)
template.set_finger(FPrint.Finger.LEFT_INDEX)
assert d.get_finger_status() == FPrint.FingerStatusFlags.NONE
p1 = d.enroll_sync(template, None, enroll_progress, None)
assert d.get_finger_status() == FPrint.FingerStatusFlags.NONE
print("enroll done")
del template

print("listing - device should have 1 print")
stored = d.list_prints_sync()
assert len(stored) == 1
assert stored[0].equal(p1)

print("verifying")
assert d.get_finger_status() == FPrint.FingerStatusFlags.NONE
verify_res, verify_print = d.verify_sync(p1)
assert d.get_finger_status() == FPrint.FingerStatusFlags.NONE
print("verify done")
assert verify_res == True

identified = False
deserialized_prints = []
for p in stored:
    deserialized_prints.append(FPrint.Print.deserialize(p.serialize()))
    assert deserialized_prints[-1].equal(p)
del stored

print('async identifying')
d.identify(deserialized_prints, callback=identify_done)
del deserialized_prints

while not identified:
    ctx.iteration(True)

print("try to enroll duplicate")
template = FPrint.Print.new(d)
template.set_finger(FPrint.Finger.RIGHT_INDEX)
assert d.get_finger_status() == FPrint.FingerStatusFlags.NONE
try:
    d.enroll_sync(template, None, enroll_progress, None)
except GLib.Error as error:
    assert error.matches(FPrint.DeviceError.quark(),
                         FPrint.DeviceError.DATA_DUPLICATE)
except Exception as exc:
    raise
assert d.get_finger_status() == FPrint.FingerStatusFlags.NONE
print("duplicate enroll attempt done")

print("listing - device should still only have 1 print")
stored = d.list_prints_sync()
assert len(stored) == 1
assert stored[0].equal(p1)
del stored

print("enroll new finger")
template = FPrint.Print.new(d)
template.set_finger(FPrint.Finger.RIGHT_INDEX)
assert d.get_finger_status() == FPrint.FingerStatusFlags.NONE
p2 = d.enroll_sync(template, None, enroll_progress, None)
assert d.get_finger_status() == FPrint.FingerStatusFlags.NONE
print("enroll new finger done")
del template

print("listing - device should have 2 prints")
stored = d.list_prints_sync()
assert len(stored) == 2
assert (stored[0].equal(p1) and stored[1].equal(p2)) or (stored[0].equal(p2) and stored[1].equal(p1))
del stored

print("deleting first print")
d.delete_print_sync(p1)
print("delete done")
del p1

print("listing - device should only have second print")
stored = d.list_prints_sync()
assert len(stored) == 1
assert stored[0].equal(p2)
del stored
del p2

print("clear device storage")
d.clear_storage_sync()
print("clear done")

print("listing - device should be empty")
stored = d.list_prints_sync()
assert len(stored) == 0
del stored

d.close_sync()

del d
del c

# Clean up dummy cached SDCP claim file
remove(claim_file_path)
