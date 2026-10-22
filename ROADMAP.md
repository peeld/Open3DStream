# Roadmap

## Principles

- **Thin.** You give it data and say what it is (axes, units, time). It turns the
  data into bytes, sends them, and hands back a structure on the other side.
- **Data is sent unmodified and annotated.** Only a receiver converts, and only
  when it asks to.
- **What a receiver can convert:** distance units, coordinate space (axes and
  handedness) and rotation representation (quaternion or euler). Time is never
  converted, only annotated.
- **Public headers use plain types.** Eigen, FlatBuffers and nng are internal.

## Plan

1. **Housekeeping.** Remove dead code and unused dependencies, and write the README,
   changelog and build options.
2. **Public API.** A small `Sender` / `Receiver` / `Stream` interface, configured by
   URL, with a target context and a choice of rotation output. The library decides
   when to send a full frame and when to send a delta.
3. **Library packaging.** Installable, exported CMake targets (`o3ds::core`,
   `o3ds::net`), with the tests run through CTest.
4. **Unreal Engine plugin (UE 5.3+), built on the new API:**
   - Map the time envelope to Live Link `SceneTime` and `WorldTime`, with the real
     rate and drop-frame flag.
   - Map rigid bodies to the Transform role and cameras to the Camera role.
   - Re-push static data when a hierarchy changes.
   - Build per platform from source, with no prebuilt `.lib` copying.
5. **Python bindings**, distributed as wheels.
6. **CI and releases.** Build and test on every push and PR, and publish packages
   and plugins on tags.
7. **Live Link parity (a goal, not a requirement):**
   - Property curves (blendshapes and facial)
   - Lights
   - Per-frame string metadata

## Open questions

- **Euler rotations on the wire.** Rotations are quaternion-only today, so a sender
  whose source data is euler (MotionBuilder, Maya) converts before sending. Under
  the "unmodified" principle, the wire would instead carry euler angles plus
  rotation order and angle unit, alongside the quaternion form. That would be an
  additive schema change. Deferred for now, to be revisited later.
- **Converted local transforms.** A receiver's conversion currently applies only to
  world matrices, and local TRS values stay in the sender's space. A receiver that
  asks for a conversion should get converted local values too.
