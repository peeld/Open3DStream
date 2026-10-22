# Open3DStream

Open3DStream (o3ds) is a small library for streaming live 3D animation (skeletons,
rigid bodies and cameras) between applications, devices and locations.

It takes care of the tedious parts:

- **Annotated, unmodified data.** A sender describes its data as-is: its axes, its
  distance units and its time and timecode. Nothing is converted on the way out.
- **Conversion on request.** A receiver gets the data back exactly as it was sent,
  or asks for it in its own coordinate space and units, for example Unreal's
  Z-up, X-forward, centimetres.
- **Compact wire format.** Data is serialised with [FlatBuffers](https://flatbuffers.dev).
  After the first full frame, only the transforms that changed are sent.
- **Transport.** Data is carried over [nng](https://nng.nanomsg.org): pub/sub, pair
  or pipeline, over TCP, IPC or TLS.

The main consumer is the **Unreal Engine Live Link** plugin, which is being
brought up to date for UE 5.3+. There is also a MotionBuilder device plugin.

> **Status:** the public C++ API is being reworked into a smaller, simpler
> interface, and Python bindings are planned. Expect breaking changes until 2.0.
> See [ROADMAP.md](ROADMAP.md).

## Repository layout

| Path | Contents |
|---|---|
| `src/o3ds.fbs` | The wire format (FlatBuffers schema) |
| `src/o3ds/` | The library: model, coordinate conversion and transports |
| `apps/SubscribeTest` | Command line receiver that prints incoming subjects |
| `apps/FbxStream` | Streams an FBX file on a loop, for testing (needs the FBX SDK) |
| `apps/Test1` | Unit tests for the data model and coordinate conversion |
| `apps/Test2` | Interactive demo: an animated three-joint hierarchy (Ctrl+C to stop) |
| `plugins/unreal` | Unreal Engine Live Link source plugin |
| `plugins/mobu` | MotionBuilder device plugin |
| `plugins/maya` | Maya plugin (not maintained, off by default) |
| `sphinx/`, `Doxyfile` | Documentation sources |
| `thirdparty/` | Dependencies, as git submodules |

## Building

You need CMake 3.13 or later, a C++17 compiler and git.

```sh
git clone --recursive <repo-url>
cd Open3DStream
```

**Linux**

```sh
./build.sh            # RelWithDebInfo by default; also Debug, Release or MinSizeRel
```

**Windows** (Visual Studio 2017, 2019 or 2022)

```bat
build.bat
```

Both scripts first build the third-party libraries (mbedtls, nng, flatbuffers and
crccpp) into `./usr`, then build o3ds in `./build` (`./vsbuild` on Windows) and
install the results into `./usr`. The third-party step is skipped once `./usr`
exists, so delete `./usr` to rebuild it.

### Build options

| Option | Default | |
|---|---|---|
| `O3DS_BUILD_TESTS` | `ON` | Test1 and Test2 |
| `O3DS_BUILD_APPS` | `ON` | SubscribeTest |
| `O3DS_BUILD_FBXSTREAM` | `ON` | Skipped if the FBX SDK is not found. Set `FBX_SDK_ROOT` to its location. |
| `O3DS_BUILD_MOBU` | `ON` | Builds for each MotionBuilder version found |
| `O3DS_BUILD_MAYA` | `OFF` | |

### Running the tests

```sh
./build/apps/Test1/Test1
```

## Concepts

A **SubjectList** is one packet: a set of subjects plus the sender's coordinate
context (which way X, Y and Z point, and the distance unit) and its time envelope
(clock, timecode, rate and drop-frame flag).

A **subject** is a performer (a skeleton), a rigid body or a camera. Each one
holds a hierarchy of transforms, identified by a stable UUID.

Rotations are carried as quaternions. A receiver can convert axes and units on
ingest. Time is annotated, never converted.

## License

MIT. See [LICENSE](LICENSE).

## Contact

- Discord: https://discord.gg/UdEbyFM9wD
- Created by Alastair Macleod ([peeldev.com](https://peeldev.com/))
- Contributors: Darin Velarde ([singularityhitech.com](http://singularityhitech.com/)), Matin Lotfaliei
