# Isolated Linux tests

Build each `.pro` file with qmake6 and make in its own build directory. Set `QT_QPA_PLATFORM=offscreen`.

- `plugin_tests plugin.so /usr/bin/python3 /absolute/path/fixture_collector.py /temporary/screenshot.png` exercises persistence, rescan, ambiguity, segment reordering, explicit removal, autosave, rendering and arbitration. Set `HSAI_ABSENT_START=1` to include a fresh process starting with absent targets.
- `feature_tests plugin.so /usr/bin/python3 /absolute/path/fixture_collector.py` tests conditions, palette deadband, animation cadence, native capability intersection and previous-mode recovery.
- `standalone_tests plugin.so` validates a fresh install without an automation library. It performs read-only system sensor/loopback queries; RGB controllers are inert.

All configurations use QTemporaryDir; no suite writes to a physical RGB controller. Do not replace these isolated configurations with personal profiles. Manual rescan/physical tests are separate and must preserve the user's state.
