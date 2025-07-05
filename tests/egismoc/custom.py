#!/usr/bin/python3

import traceback
import sys
import time
import gi
from math import trunc
from os import makedirs, path
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

# In order for all of the payloads to match, we will need cache the same
# SDCP claim as was used when the test was generated.

# You can fetch the claim value to paste below after capturing your test using something like this:
# sudo hexdump -e '16/1 "%02x " "\n"' /tmp/.libfprint/egismoc/emulated-device/sdcp-claim

claim_bytes = bytes.fromhex("""
ab 7f 71 1a 02 00 00 00 79 0d 65 48 2f 39 06 00
02 52 04 b8 9a d0 f8 e1 f2 81 f9 4b 02 94 92 7c
d7 2b 79 7e d1 03 73 0e 1b 50 ae c0 46 65 be e3
04 e2 33 77 fd ca e1 b7 a6 4f 8b ba 57 d8 95 af
e0 8d 34 fe 9a c6 82 14 ec a4 f7 ac 64 b9 85 c0
98 ff 0f c8 35 54 90 fd 17 75 bc ef 36 b2 e1 74
7b 7f 39 f9 8f 41 74 38 36 4e 39 99 78 49 1f 70
30 2b fc f4 80 06 24 8e 4e 6e 52 d8 36 17 2a f8
ad 33 cf d0 be 04 5c 06 c1 80 5c c4 3a eb 9c b2
3c dc 4a d0 d0 61 0a 1f 64 1d 7d 84 4d d6 e7 06
92 ac 28 09 98 5c 16 0e ec d0 98 1c 70 d0 f5 23
b4 c3 1d 20 15 1e cf 3c da 5b 18 2b 17 06 9d 07
4e 92 bb e1 36 ff a7 2a 3b 6c c8 19 d5 72 8d e5
ad 73 a9 ea 0a 52 11 48 fb 1e e4 a8 0c 52 62 5b
ad 05 80 26 c1 e2 c4 40 89 85 89 dc 0d 8a 2d 77
8e 24 84 34 6c 72 73 e8 44 be 7f 56 b4 ef df 70
90 4a 32 dd 3e 5f f2 7f fb ff cb ee d8 8d 48 22
1e f1 00 d1 00 b1 00 91 00 71 00 30 00
""")

# Set this to False when capturing or True when testing
write_claim_file=True

# Also, if you capture with sudo, then it might help to run this after capturing but before testing:
# sudo rm -rf /tmp/.libfprint

# Now we will strip out the first 16 bytes (uptime and realtime) and replace them with current
# values

uptime = trunc(time.clock_gettime(time.CLOCK_MONOTONIC_RAW) * 1000000) # g_get_monotonic_time()
realtime = trunc(time.clock_gettime(time.CLOCK_REALTIME) * 1000000) # g_get_real_time()
uptime_bytes = uptime.to_bytes(8, byteorder = 'little')
realtime_bytes = realtime.to_bytes(8, byteorder = 'little')
claim_bytes = uptime_bytes + realtime_bytes + claim_bytes[16:]

if write_claim_file:
    # Now we will pre-cache this claim file so that the test script will use the same host keys
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
