# Changelog

Versions are wire-protocol versions (`O3DS_VERSION_TAG` in `CMakeLists.txt`).

## Unreleased

- Repository tidy-up:
  - Removed the Repeater and XSensTest apps, the xsens parser and the unbuilt
    websocket connector.
  - Removed the CML and nlohmann_json submodules.
  - Removed the NSIS installer and `package.py`. Releases come from CI.
  - Added CMake build options.
  - FbxStream finds the FBX SDK through `FBX_SDK_ROOT` and is skipped if the
    SDK is missing.

## 1.0.6

- `Subject::parse()` clears its own transform list. This is a receiver-side
  fix, not a wire change. `SubjectList::parse()` cleared performers and
  rigidbodies at the call site but never cleared cameras, so a camera's
  transform list grew by its own size on every full packet.

## 1.0.5

- Time envelope on `SubjectList`: clock domain and instance, exact ns, quality,
  device time, and the timecode's rate and drop-frame flag.
- Additive: a 1.0.4 receiver reads a 1.0.5 packet unchanged, and a 1.0.5
  receiver sees `time_version` 0 from a 1.0.4 sender.

## 1.0.4

- Removed unused subjects and added all matrices.
