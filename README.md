# Hardware Sync AI

An independent GPL-2.0-or-later derivative of [OpenRGB Hardware Sync](https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin). It retains the upstream Qt interface, measurement palettes, zone selection and profile format, and adds conditional overlays based on local Ollama model residency. It is not an official OpenRGB release.

[Français](docs/README.fr.md) · [Upstream documentation](README.upstream.md)

## Install

Download the **Linux x86_64 / Qt 6 / plugin API 5** `.so` file from this repository's Releases. In OpenRGB select **Settings → Plugins → Install plugin**, choose the downloaded `.so`, then enable Hardware Sync AI. Do not install alongside the original Hardware Sync plugin controlling the same devices.

The binary's exact Qt, libc and dynamic dependencies are in its release BUILD-INFO. The initial binary targets a recent Linux environment; it is **not** a universal AppImage or a Windows/macOS build. A Qt 5 host cannot load a Qt 6 plugin. Build against your host environment when versions differ.

Python 3 is required for the bundled, read-only collector; it uses only the standard library. `lm_sensors`, `glib2` and `libgtop` are build/runtime dependencies inherited from the Linux plugin. NVIDIA metrics need `nvidia-smi`. Ollama is optional, at loopback `127.0.0.1:11434`; unavailable metrics remain unknown rather than fabricated as zero. No external account, API key, personal automation installation or hardware identifier is supplied.

Fresh installs start paused with no target selected. Add a measure, choose a sensor, select devices/zones, adjust the palette, start the measure and explicitly resume plugin writes. Device permissions are supplied by OpenRGB/your distribution; this plugin does not install privileged helpers.

## Measurements and overlays

Linux sensor discovery reuses the upstream libsensors implementation. CPU/RAM, GPU telemetry, load averages and AI residency are available alongside those detected sensor inputs. A separately configured collector remains supported for existing installations. Missing sensors do not enable another sensor implicitly.

- **No effect**: palette follows the chosen measurement, without a brightness overlay. One palette stop produces a fixed color.
- **Breathing**: smooth periodic brightness envelope; **Pulse**: repeated peak followed by decay.
- **Always**, **Model GPU**, **Model CPU** and **Model RAM** determine when the overlay runs. Detailed explanations are tooltips, not long option names. When false, ordinary measurement coloring continues. Model residency is not proof of active inference; CPU and host allocations are estimates from Ollama's reported sizes.
- **Device: …** effects are the intersection of modes advertised by all selected devices, offered only when every populated zone of each device is selected. They retain the device's existing OpenRGB effect parameters. Native timing is controlled by its firmware, not by the software overlay period. On a false condition the plugin restores the previous mode; it never substitutes an unsupported effect.
- Software breathing/pulse renders at least 30 frames/s independently of the sensor polling setting. A palette change threshold (default 1% of palette input range) rejects tiny input fluctuations; time-based smoothing handles meaningful changes.

Black/OFF and manually changed output stay protected; conflicting rules are inhibited. Topology changes pause plugin writes until explicit resume. A native firmware animation can continue while plugin writes are paused; change its mode in OpenRGB to stop it. No hardware mode is selected automatically on a fresh install.

## Remembered targets

Assignments are durable configuration, separate from detected controller pointers. Rescans and missing devices do not delete them. Unique serial/model identity takes priority over volatile device paths. A serial-less fallback requires a unique normalized path/model match. Ambiguous devices, changed LED counts and unavailable zones remain pending rather than being assigned arbitrarily. Named zones/segments can survive index changes.

Explicit deselection removes that assignment; **Forget unavailable targets** removes pending choices in the current measure. Pending selections are saved in profiles and settings but cannot emit RGB writes. No mapping can distinguish physically identical devices lacking reliable identifiers in every topology; ambiguous cases require the user.

## Build and tests (Linux)

```sh
git clone --recurse-submodules https://github.com/arthur469/HardwareSyncAI.git
cd HardwareSyncAI
mkdir build
cd build
qmake6 ../OpenRGBHardwareSyncPlugin.pro
make -j4
```

The `OpenRGB` git submodule pins SDK headers to a public upstream revision (plugin API 5). Use a compiler and Qt matching your host. Tests use inert controllers and isolated configuration; see `tests-linux/README.md`. No hardware writes are required by the simulated suites.

## Upstream tracking, license and disclosure

The Git history begins with the original project. `upstream/master` in this repository records the imported revision; our modifications are kept on `feature/hardware-sync-ai`. Updating upstream is a deliberate review, not an automatic merge. Keep a local remote named `upstream` pointing to the GitLab project and compare before integration.

Original copyright notices and GPL license are retained. Embedded SDK JSON code retains its MIT notices; libsensors-cpp retains its LGPL-2.1-or-later notices. This derivative was developed with AI assistance. It is published under its owner's authorization, not submitted to OpenRGB upstream. Respect each target project's current AI/contribution policy before contacting developers. No personal profiles, serial numbers, accounts or recordings belong in this repository.
