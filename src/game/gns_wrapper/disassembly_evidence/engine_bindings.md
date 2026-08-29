# Engine Binding Disassembly Evidence

This is the consolidated evidence record for the fixed engine bindings used by
the GNS lifecycle code. It separates binary observations from interpretations.
All addresses below are for the PE32 image identified here; an address or
offset must not be reused for another engine build.

## Evidence basis and PE identity

The binary examined is:

```text
engine.dll (local binary under review; identify by SHA-256 below)
```

| PE/file item | Verified value |
| --- | --- |
| File length | `4,468,736` bytes (`0x00443000`) |
| SHA-256 | `D41ABC19018D2A2E62BBC1E71C53E310458B43971BAE132A14EB290278142FE0` |
| DOS signature | `0x5A4D` |
| `e_lfanew` | `0x00000110` |
| NT signature | `0x00004550` (`PE\0\0`) |
| Machine | `0x014C` (`IMAGE_FILE_MACHINE_I386`) |
| Optional-header magic | `0x010B` (PE32) |
| COFF timestamp | `0x5F363761` |
| Image base | `0x10000000` |
| `SizeOfImage` | `0x006F2000` |

Relevant section mapping:

| Section | RVA | Virtual size | Raw pointer | Raw size | Characteristics |
| --- | ---: | ---: | ---: | ---: | ---: |
| `.text` | `0x00001000` | `0x002F4B0E` | `0x00001000` | `0x002F5000` | `0x60000020` |
| `.rdata` | `0x002F6000` | `0x000DF1B2` | `0x002F6000` | `0x000E0000` | `0x40000040` |
| `.data` | `0x003D6000` | `0x002CA9E4` | `0x003D6000` | `0x0001C000` | `0xC0000040` |

For every backed `.text` and `.rdata` item in this document, the raw file
offset equals the RVA because the section virtual address and raw pointer are
equal. The server global at RVA `0x00598480` is in the zero-filled virtual
tail of `.data`: it has no file offset and no file bytes. Its value can only be
checked in a loaded image.

The source-side image guards compare the machine, PE32 magic, image base,
timestamp, and image size. The runtime hook additionally checks target bounds,
the exact target signature, and executable section membership. The SHA-256
above is the offline identity of the checked file; it is not computed by the
runtime guards.

## `src/game/shared/asrd_gns_runtime_hook.cpp`

### Connect-intent detour

| Binding | RVA | File offset | Source use |
| --- | ---: | ---: | --- |
| Connect-intent target | `0x000D69F0` | `0x000D69F0` | Patched by `RuntimeHookThunk` |

The 20-byte expected signature is exactly:

```text
8B 44 24 08 8B 54 24 04 6A 01 50 52 E8 EF FE FF FF C2 08 00
```

The bytes decode on complete instruction boundaries as:

```text
000D69F0: 8B 44 24 08          mov eax, dword ptr [esp + 8]
000D69F4: 8B 54 24 04          mov edx, dword ptr [esp + 4]
000D69F8: 6A 01                push 1
000D69FA: 50                   push eax
000D69FB: 52                   push edx
000D69FC: E8 EF FE FF FF       call 0x100D68F0
000D6A01: C2 08 00             ret 8
```

Observation: the target reads two explicit arguments from `[ESP+4]` and
`[ESP+8]`, does not alter ECX in this window, and returns with `ret 8`.

Interpretation: the source fastcall thunk receives the object in ECX, consumes
the unused EDX register position, leaves the two explicit arguments on the
stack, and has the callee clean eight bytes. The thunk calls
`ASRD_GNS_ClientConnectIntent(self, endpoint, secondary)`, returns `1` only
for a successful GNS takeover, and returns `0` otherwise. It does not call the
saved original as a legacy fallback.

### Message-registration detour

| Binding | RVA | File offset | Source use |
| --- | ---: | ---: | --- |
| Message-registration target | `0x00079920` | `0x00079920` | Patched by `RuntimeRegisterMessageThunk` |

The 8-byte expected signature is exactly:

```text
56 57 8B 7C 24 0C 8B 07
```

The entry and both native return paths decode as follows:

```text
00079920: 56                   push esi
00079921: 57                   push edi
00079922: 8B 7C 24 0C          mov edi, dword ptr [esp + 0x0C]
00079926: 8B 07                mov eax, dword ptr [edi]
00079928: 8B 50 1C             mov edx, dword ptr [eax + 0x1C]
0007992B: 8B F1                mov esi, ecx
0007992D: 8B CF                mov ecx, edi
0007992F: FF D2                call edx
00079931: 50                   push eax
00079932: 8B CE                mov ecx, esi
00079934: E8 37 B5 FF FF       call 0x10074E70
00079939: 85 C0                test eax, eax
0007993B: 74 07                je 0x10079944
0007993D: 5F                   pop edi
0007993E: 32 C0                xor al, al
00079940: 5E                   pop esi
00079941: C2 04 00             ret 4

00079944: 8B 96 1C 42 00 00    mov edx, dword ptr [esi + 0x421C]
0007994A: 8D 8E 10 42 00 00    lea ecx, [esi + 0x4210]
00079950: 8D 44 24 0C          lea eax, [esp + 0x0C]
00079954: 50                   push eax
00079955: 52                   push edx
00079956: E8 45 D6 00 00       call 0x10086FA0
0007995B: 8B 07                mov eax, dword ptr [edi]
0007995D: 8B 50 04             mov edx, dword ptr [eax + 4]
00079960: 56                   push esi
00079961: 8B CF                mov ecx, edi
00079963: FF D2                call edx
00079965: 5F                   pop edi
00079966: B0 01                mov al, 1
00079968: 5E                   pop esi
00079969: C2 04 00             ret 4
```

Observation: after the two register saves, the message pointer originally at
`[ESP+4]` is addressed at `[ESP+0x0C]`. ECX is retained as the channel. The
target calls through message-object vtable offsets `+0x1C` and `+0x04`, and
both return paths clean one four-byte stack argument. These offsets establish
indirect calls only; they do not establish public method names.

Interpretation: `RuntimeRegisterMessageThunk` uses
`bool __fastcall(channel, unused_edx, message)`, calls the original through
the trampoline with ECX restored to the channel and the message at `[ESP+4]`,
and captures the message in the local registry only when the original returns
true. A missing trampoline returns false.

### Patch and trampoline mechanics

`kPatchSize` is eight bytes for both targets.

* At `0x000D69F0`, the copied span is the two complete four-byte `mov`
  instructions. The trampoline resumes at `0x000D69F8`, the first byte of
  `push 1`.
* At `0x00079920`, the copied span is `push esi`, `push edi`,
  `mov edi,[esp+0x0C]`, and `mov eax,[edi]`. The trampoline resumes at
  `0x00079928`, the first byte of `mov edx,[eax+0x1C]`.

`WriteRelativeJump` writes exactly eight bytes:

```text
E9 <signed little-endian rel32> 90 90 90
```

The displacement is `destination - (source + 5)` and must fit a signed
32-bit value. The source changes protection for the eight-byte span, writes
the jump and three NOPs, flushes the instruction cache, and restores the
previous protection. `InstallHook` pins the module containing the thunk,
allocates 16 executable bytes, copies the verified eight-byte prologue,
writes a trampoline jump from offset `+8` to `target+8`, and patches the
target only after the trampoline jump succeeds. Any allocation or jump failure
frees the trampoline without leaving the target patched. The resumed native
code reaches its own `ret 8` or `ret 4`; the trampoline does not synthesize a
return. Successful detours remain installed for the process lifetime.

### Shared hook state and guards

`SharedHookState` is packed to `0x4C` bytes. The mapping is process-local and
does not describe an engine object.

| Offset | Field | Meaning in this source |
| ---: | --- | --- |
| `0x00` | `magic` | Mapping format marker |
| `0x04` | `version` | Mapping format version |
| `0x08` | `state` | Connect-hook state |
| `0x0C` | `installCount` | Connect-hook install count |
| `0x10` | `initCount` | Initialization attempts |
| `0x14` | `callCount` | Connect-thunk calls |
| `0x18` | `targetRva` | Connect target RVA |
| `0x1C` | `targetAddress` | Loaded connect target address |
| `0x20` | `trampolineAddress` | Loaded connect trampoline address |
| `0x24` | `ownerPid` | Connect-hook owner process |
| `0x28` | `ownerTid` | Connect-hook owner thread |
| `0x2C` | `registrationState` | Registration-hook state |
| `0x30` | `registrationInstallCount` | Registration install count |
| `0x34` | `registrationCallCount` | Registration-thunk calls |
| `0x38` | `registrationTargetRva` | Registration target RVA |
| `0x3C` | `registrationTargetAddress` | Loaded registration target address |
| `0x40` | `registrationTrampolineAddress` | Loaded registration trampoline address |
| `0x44` | `registrationOwnerPid` | Registration-hook owner process |
| `0x48` | `registrationOwnerTid` | Registration-hook owner thread |

The state values are `EMPTY=0`, `INSTALLING=1`, `INSTALLED=2`, and
`FAILED=3`. Initialization serializes access with the process-named mapping
mutex. A concurrent or failed installation transitions to `FAILED`; an image,
signature, address-range, or executable-section mismatch leaves the relevant
hook disabled.

## `src/game/shared/asrd_gns_client_lifecycle.cpp`

### Client-state vtable and lifecycle targets

The client-state table is at RVA `0x00329B44`, file offset `0x00329B44`. The
first 16 entries are the following exact 64 bytes:

```text
D0 9C 10 10 E0 57 0D 10 70 5D 0A 10 50 5E 0A 10
C0 5E 0A 10 E0 5E 0A 10 10 68 0A 10 80 5E 0A 10
A0 5E 0A 10 A0 0D 0D 10 60 45 0D 10 50 8D 10 10
30 71 10 10 F0 69 0D 10 10 6A 0D 10 A0 91 10 10
```

The relevant entries are:

| Slot | Entry file offset | Raw entry bytes | VA | Target RVA | Source use |
| ---: | ---: | --- | ---: | ---: | --- |
| 1 | `0x00329B48` | `E0 57 0D 10` | `0x100D57E0` | `0x000D57E0` | `ConnectionStart` / message registration |
| 4 | `0x00329B54` | `C0 5E 0A 10` | `0x100A5EC0` | `0x000A5EC0` | Packet start |
| 5 | `0x00329B58` | `E0 5E 0A 10` | `0x100A5EE0` | `0x000A5EE0` | Packet end |
| 13 | `0x00329B78` | `F0 69 0D 10` | `0x100D69F0` | `0x000D69F0` | Same target as the runtime connect-intent detour |
| 15 | `0x00329B80` | `A0 91 10 10` | `0x101091A0` | `0x001091A0` | Client signon-state method |

The table address and the target entries are accepted only after the PE image
check and exact pointer comparisons. The table also provides the binary link
between the client object's slot 13 and the runtime hook target; the hook
patches the fixed target address rather than writing a vtable entry.

### Client `ConnectionStart` and registration

The target at RVA `0x000D57E0`, file offset `0x000D57E0`, begins with these
exact bytes:

```text
53 55 56 6A 1C 8B E9 E8 24 52 FB FF 33 DB 83 C4
04 3B C3 74 1B 0F 57 C0 89 58 08 C7 00 EC 27 30
10 88 58 04 F3 0F 11 40 14 F3 0F 11 40 18 EB 02
33 C0 3B EB 74 05 8D 4D 08 EB 02 33 C9 8B 74 24
```

The entry decodes as `push ebx; push ebp; push esi; push 0x1C; mov
ebp,ecx; call 0x1008AA10; xor ebx,ebx; add esp,4; cmp eax,ebx`, followed by
the allocation/initialization path. The registration call site at RVA and
file offset `0x000D581D` is:

```text
8B 74 24 10 89 48 0C 8B 16 50 8B 42 74 8B CE FF D0
```

```text
000D581D: 8B 74 24 10          mov esi, dword ptr [esp + 0x10]
000D5821: 89 48 0C             mov dword ptr [eax + 0x0C], ecx
000D5824: 8B 16                mov edx, dword ptr [esi]
000D5826: 50                   push eax
000D5827: 8B 42 74             mov eax, dword ptr [edx + 0x74]
000D582A: 8B CE                mov ecx, esi
000D582C: FF D0                call eax
```

The final call site and return path at file offset/RVA `0x000D5FB2` are:

```text
8B 16 50 8B 42 74 8B CE FF D0 5E 5D 5B C2 04 00
```

```text
000D5FB2: 8B 16                mov edx, dword ptr [esi]
000D5FB4: 50                   push eax
000D5FB5: 8B 42 74             mov eax, dword ptr [edx + 0x74]
000D5FB8: 8B CE                mov ecx, esi
000D5FBA: FF D0                call eax
000D5FBC: 5E                   pop esi
000D5FBD: 5D                   pop ebp
000D5FBE: 5B                   pop ebx
000D5FBF: C2 04 00             ret 4
```

Observation: the target receives the client object in ECX and one explicit
channel argument on the stack, and uses channel-vtable byte offset `0x74`,
which is slot 29, to call the registration adapter with the message pointer
on the stack and the adapter in ECX. The target's native return is `ret 4`.
The write at `0x000D5821` is an observed message-object `+0x0C` write from the
current ECX value; no higher-level field name is assigned to it.

Interpretation: `ClientConnectionStartFn` is a one-stack-argument
`__thiscall` binding. The local adapter's fastcall declaration consumes ECX as
the adapter object and reserves the unused EDX position while leaving the
message argument on the stack.

### Packet boundaries

Packet start is at RVA and file offset `0x000A5EC0`. Its exact bytes and
complete body are:

```text
8B 44 24 04 8B 54 24 08 89 41 7C 89 91 50 4A 00 00 C2 08 00
```

```text
000A5EC0: 8B 44 24 04          mov eax, dword ptr [esp + 4]
000A5EC4: 8B 54 24 08          mov edx, dword ptr [esp + 8]
000A5EC8: 89 41 7C             mov dword ptr [ecx + 0x7C], eax
000A5ECB: 89 91 50 4A 00 00    mov dword ptr [ecx + 0x4A50], edx
000A5ED1: C2 08 00             ret 8
```

Observation: ECX is the client object, two stack arguments are copied to
client offsets `+0x7C` and `+0x4A50`, and the callee removes eight stack bytes.
The source calls this target as
`__thiscall(clientState, incomingSequence, outgoingAcknowledged)`.

Packet end is at RVA and file offset `0x000A5EE0`. The exact function bytes
through its native return are:

```text
56 8B F1 E8 D8 15 05 00 8B 86 C4 00 00 00 3B 86
CC 00 00 00 75 30 8B 8E 50 4A 00 00 8B C1 2B 86
4C 4A 00 00 89 8E 4C 4A 00 00 8B 0D D0 07 44 10
8B 11 50 8B 42 18 FF D0 8B 0D 98 13 3E 10 8B 11
8B 42 40 5E FF E0 5E C3
```

```text
000A5EE0: 56                   push esi
000A5EE1: 8B F1                mov esi, ecx
000A5EE3: E8 D8 15 05 00       call 0x100F74C0
000A5EE8: 8B 86 C4 00 00 00    mov eax, dword ptr [esi + 0xC4]
000A5EEE: 3B 86 CC 00 00 00    cmp eax, dword ptr [esi + 0xCC]
000A5EF4: 75 30                jne 0x100A5F26
000A5EF6: 8B 8E 50 4A 00 00    mov ecx, dword ptr [esi + 0x4A50]
000A5EFC: 8B C1                mov eax, ecx
000A5EFE: 2B 86 4C 4A 00 00    sub eax, dword ptr [esi + 0x4A4C]
000A5F04: 89 8E 4C 4A 00 00    mov dword ptr [esi + 0x4A4C], ecx
000A5F0A: 8B 0D D0 07 44 10    mov ecx, dword ptr [0x104407D0]
000A5F10: 8B 11                mov edx, dword ptr [ecx]
000A5F12: 50                   push eax
000A5F13: 8B 42 18             mov eax, dword ptr [edx + 0x18]
000A5F16: FF D0                call eax
000A5F18: 8B 0D 98 13 3E 10    mov ecx, dword ptr [0x103E1398]
000A5F1E: 8B 11                mov edx, dword ptr [ecx]
000A5F20: 8B 42 40             mov eax, dword ptr [edx + 0x40]
000A5F23: 5E                   pop esi
000A5F24: FF E0                jmp eax
000A5F26: 5E                   pop esi
000A5F27: C3                   ret
```

Observation: packet end has no explicit stack arguments and has a plain `ret`
on its direct return path. It reads client offsets `+0xC4`, `+0xCC`, `+0x4A50`,
and `+0x4A4C`, updating `+0x4A4C`; another path tail-jumps after the same
packet bookkeeping. These are engine-owned fields; the lifecycle source does
not write them directly.

### Client signon-state method

The client-state slot 15 target is RVA and file offset `0x001091A0`. Its exact
entry/early-return bytes are:

```text
81 EC 48 01 00 00 8B 94 24 50 01 00 00 55 8B AC
24 50 01 00 00 56 8B F1 8B 8C 24 5C 01 00 00 8B
46 68 51 52 55 8B CE 89 44 24 14 E8 00 85 FC FF
84 C0 75 12 E8 37 E5 FE FF 5E 32 C0 5D 81 C4 48
01 00 00 C2 0C 00
```

```text
001091A0: 81 EC 48 01 00 00    sub esp, 0x148
001091A6: 8B 94 24 50 01 00 00 mov edx, dword ptr [esp + 0x150]
001091AD: 55                   push ebp
001091AE: 8B AC 24 50 01 00 00 mov ebp, dword ptr [esp + 0x150]
001091B5: 56                   push esi
001091B6: 8B F1                mov esi, ecx
001091B8: 8B 8C 24 5C 01 00 00 mov ecx, dword ptr [esp + 0x15C]
001091BF: 8B 46 68             mov eax, dword ptr [esi + 0x68]
001091C2: 51                   push ecx
001091C3: 52                   push edx
001091C4: 55                   push ebp
001091C5: 8B CE                mov ecx, esi
001091C7: 89 44 24 14          mov dword ptr [esp + 0x14], eax
001091CB: E8 00 85 FC FF       call 0x100D16D0
001091D0: 84 C0                test al, al
001091D2: 75 12                jne 0x101091E6
001091D4: E8 37 E5 FE FF       call 0x100F7710
001091D9: 5E                   pop esi
001091DA: 32 C0                xor al, al
001091DC: 5D                   pop ebp
001091DD: 81 C4 48 01 00 00    add esp, 0x148
001091E3: C2 0C 00             ret 0x0C
```

Observation: ECX is the client-state object. The method has three explicit
stack arguments and the early return cleans 12 bytes. The source therefore
uses `ClientSetSignonStateFn(clientState, state, count, reserved)` and passes
zero for the reserved value.

The target obtains its channel from object offset `+0x10` and dispatches the
following adapter slots. Each slot value is decoded from the indirect load at
the listed call site; all call-site bytes are file-backed at the same RVA.

| Call-site RVA/file offset | Exact bytes | Decoded dispatch | Adapter slot |
| ---: | --- | --- | ---: |
| `0x00109250` | `8B 4E 10 8B 11 8B 82 90 00 00 00 83 C4 04 FF D0` | `mov ecx,[esi+0x10]; mov edx,[ecx]; mov eax,[edx+0x90]; add esp,4; call eax` | 36 |
| `0x00109266` | `8B 4E 10 8B 11 8B 82 80 00 00 00 53 51 D9 1C 24 FF D0` | `mov ecx,[esi+0x10]; mov edx,[ecx]; mov eax,[edx+0x80]; push ebx; push ecx; fstp dword ptr [esp]; call eax` | 32 |
| `0x00109278` | `8B 4E 10 8B 11 8B 82 00 01 00 00 53 68 00 77 01 00 6A 01 FF D0` | `mov ecx,[esi+0x10]; mov edx,[ecx]; mov eax,[edx+0x100]; push ebx; push 0x17700; push 1; call eax` | 64 |
| `0x001092A8` | `8B 4E 10 8B 01 8B 80 A4 00 00 00 83 C4 0C 53 53 8D 54 24 1C 52 FF D0` | `mov ecx,[esi+0x10]; mov eax,[ecx]; mov eax,[eax+0xA4]; add esp,0x0C; push ebx; push ebx; lea edx,[esp+0x1C]; push edx; call eax` | 41 |

Later branches use the same adapter slots with different surrounding
instructions. The exact later dispatch sequences are:

| Branch or dispatch RVA/file offset | Exact bytes through the indirect call | Decoded instructions and slot |
| ---: | --- | --- |
| `0x001093E7` | `8B 4E 10 A1 6C 04 45 10 8B 11 D9 40 2C 8B 92 80 00 00 00 53 51 D9 1C 24 FF D2` | `mov ecx,[esi+0x10]; mov eax,[0x1045046C]; mov edx,[ecx]; fld dword ptr [eax+0x2C]; mov edx,[edx+0x80]; push ebx; push ecx; fstp dword ptr [esp]; call edx` - slot 32 |
| `0x00109401` | `8B 4E 10 8B 01 8B 90 00 01 00 00 53 68 A0 0F 00 00 6A 01 FF D2` | `mov ecx,[esi+0x10]; mov eax,[ecx]; mov edx,[eax+0x100]; push ebx; push 0xFA0; push 1; call edx` - slot 64 |
| `0x0010955D` branch; `0x001095A2` dispatch | `8B 4E 10` at the branch start; `8B 11 8B 92 A4 00 00 00 53 8D 44 24 1C 50 FF D2` at the dispatch | `mov ecx,[esi+0x10]`; later `mov edx,[ecx]; mov edx,[edx+0xA4]; push ebx; lea eax,[esp+0x1C]; push eax; call edx` - slot 41 |

The byte offsets `0x90`, `0x80`, `0x100`, and `0xA4` are respectively
`36*4`, `32*4`, `64*4`, and `41*4`. These boundaries prove the adapter
object is in ECX and the explicit callback values are stack arguments; they
do not name the engine's channel class.

### Client raw offsets, local state, and lifecycle guards

Direct raw accesses made by this translation unit to the engine-owned client
object are:

| Object offset | Observation and consequence |
| ---: | --- |
| `+0x00` | Object vptr. It must equal the table at `base + 0x00329B44`. |
| `+0x10` | Channel pointer. Dispatch temporarily or persistently replaces this pointer with the local registration adapter. |

The packet target, rather than this source file, owns the packet fields at
`+0x7C`, `+0x4A4C`, `+0x4A50`, `+0xC4`, and `+0xCC` shown above. The source
passes packet arguments and never treats those fields as a local structure.

`SharedClientState` is a separate packed process-local record of size `0x64`;
its offsets are:

| Offset | Field |
| ---: | --- |
| `0x00` | `magic` |
| `0x04` | `version` |
| `0x08` | `state` |
| `0x0C` | `connection` |
| `0x10` | `reason` |
| `0x14` | `intentCount` |
| `0x18` | `takeoverCount` |
| `0x1C` | `eventCount` |
| `0x20` | `wrapperInitialized` |
| `0x24` | `endpoint[64]` |

The lifecycle states are `DISCONNECTED=0`, `PENDING=1`, `CONNECTED=2`,
`FAILED=3`, and `CLOSED=4`. The generation/packet guards
`s_sourceContextBound`, `s_registrationAdapterReady`,
`s_sourceChannelPersistent`, `s_challengePrimed`, `s_connectedPrimed`,
`s_localConnectedPrime`, `s_packetStarted`, and `s_compatibilityFatal` gate
engine calls. Generation and compatibility resets clear these guards at the
existing connection and teardown boundaries. The command/sequence mirrors
`s_currentServerUpdateSeq` and `s_currentClientCommandAck` are call-boundary
bookkeeping, not engine fields.

The conservative lifecycle conclusion is: context binding requires x86, the
fixed PE identity, the exact client table, slot 1, and a nonempty registration
capture; the challenge and connected signon calls are then made through slot
15. The channel adapter is temporary during dispatch and becomes persistent
for the connected generation because the connected Source frame path
dereferences the channel pointer. Shutdown detaches the adapter before closing
the GNS connection. Any binding mismatch or failed signon/packet operation
stops the compatibility path instead of selecting a legacy transport.

### Client local registration adapter

`RegistrationChannelAdapter` contains one vtable pointer. In a 32-bit process,
`s_registrationVtable` has 75 entries, is zeroed before use, and leaves every
unlisted entry NULL. The retained local entries are:

| Slot | Byte offset | Local callback |
| ---: | ---: | --- |
| 1 | `0x04` | `RegistrationAdapterGetAddress` |
| 6 | `0x18` | `RegistrationAdapterIsLoopback` |
| 7 | `0x1C` | `RegistrationAdapterIsTimingOut` |
| 10 | `0x28` | `RegistrationAdapterGetAvgLatency` |
| 11 | `0x2C` | `RegistrationAdapterGetAvgLoss` |
| 12 | `0x30` | `RegistrationAdapterGetAvgChoke` |
| 14 | `0x38` | `RegistrationAdapterGetAvgPackets` |
| 17 | `0x44` | `RegistrationAdapterGetSequenceNr` |
| 22 | `0x58` | `RegistrationAdapterGetTimeSinceLastReceived` |
| 25 | `0x64` | `RegistrationAdapterGetRemoteFramerate` |
| 26 | `0x68` | `RegistrationAdapterGetTimeoutSeconds` |
| 28 | `0x70` | `RegistrationAdapterSetDataRate` |
| 29 | `0x74` | `RegistrationAdapterRegisterMessage` |
| 32 | `0x80` | `RegistrationAdapterSetTimeout` |
| 35 | `0x8C` | `RegistrationAdapterReset` |
| 36 | `0x90` | `RegistrationAdapterClear` |
| 37 | `0x94` | `RegistrationAdapterShutdown` |
| 41 | `0xA4` | `RegistrationAdapterSendNetMsg` |
| 46 | `0xB8` | `RegistrationAdapterSetChoked` |
| 47 | `0xBC` | `RegistrationAdapterSendDatagram` |
| 48 | `0xC0` | `RegistrationAdapterTransmit` |
| 49 | `0xC4` | `RegistrationAdapterGetRemoteAddress` |
| 56 | `0xE0` | `RegistrationAdapterUpdateMessageStats` |
| 57 | `0xE4` | `RegistrationAdapterCanPacket` |
| 58 | `0xE8` | `RegistrationAdapterIsOverflowed` |
| 59 | `0xEC` | `RegistrationAdapterIsTimedOut` |
| 60 | `0xF0` | `RegistrationAdapterHasPendingReliableData` |
| 64 | `0x100` | `RegistrationAdapterSetMaxBufferSize` |
| 65 | `0x104` | `RegistrationAdapterIsNull` |
| 67 | `0x10C` | `RegistrationAdapterSetInterpolationAmount` |
| 68 | `0x110` | `RegistrationAdapterSetRemoteFramerate` |
| 74 | `0x128` | `RegistrationAdapterIsRemoteDisconnected` |

The fastcall shims receive the adapter in ECX, reserve the unused EDX slot,
and keep public virtual parameters on the stack. The binary call sites above
directly prove slots 29, 32, 36, 41, and 64 on the client signon path; the
`ConnectionStart` target directly proves slot 29. The other populated entries
are local table assignments, not engine RVAs, so no address for a local
callback is claimed as a file location.

The adapter's source behavior is deliberately conservative: registration
captures opaque messages; `SendNetMsg` forwards ordinary messages through the
bridge while consuming the redundant connected signon message locally;
`SendDatagram` returns the tracked command number; `Transmit` reports success;
address and timing queries return the stored address or bounded sentinel
values; state queries return fixed safe values; setters are no-ops; and
`Shutdown` detaches the persistent channel before using the normal close path.
Unexpected adapter self-pointers and invalid message/buffer inputs are
rejected or return safe values.

## `src/game/shared/asrd_gns_server_lifecycle.cpp`

### Server global and object topology

| Binding | RVA | File offset | Exact binary evidence and use |
| --- | ---: | ---: | --- |
| Server global pointer | `0x00598480` | None (virtual tail) | Runtime pointer is read only after PE validation. |
| Expected server vtable | `0x00339C7C` | `0x00339C7C` | Exact first 16 bytes: `10 8E 18 10 10 6C 00 10 C0 28 00 10 00 29 00 10`. The source compares the loaded vptr and does not call a server-vtable slot. |
| `GetFreeClient` | `0x00005B00` | `0x00005B00` | Validated direct target used to obtain a Source client slot. |
| Handler table | `0x0033701C` | `0x0033701C` | Constructor writes this table at `client+0x04`. |
| Primary client table | `0x003370DC` | `0x003370DC` | Constructor writes this table at `client+0x00`. |

Constructor bytes at RVA/file offset `0x0017E6B1` establish the object
topology:

```text
C7 06 DC 70 33 10 C7 45 00 1C 70 33 10 C7 46 08 D4 6F 33 10
C7 86 08 D7 01 00 CC 6F 33 10
```

```text
0017E6B1: C7 06 DC 70 33 10          mov dword ptr [esi], 0x103370DC
0017E6B7: C7 45 00 1C 70 33 10       mov dword ptr [ebp], 0x1033701C
0017E6BE: C7 46 08 D4 6F 33 10       mov dword ptr [esi + 8], 0x10336FD4
0017E6C5: C7 86 08 D7 01 00 CC 6F 33 10
                                      mov dword ptr [esi + 0x1D708], 0x10336FCC
```

Observation: at this constructor point ESI is the client object and EBP is
`client+4`. Therefore the primary vptr is at `client+0`, the handler/IClient
vptr is at `client+4`, and another engine-private pointer is at `client+8`.
Only the first two are used by this source.

The handler-table entries used by this source are exact four-byte pointers:

| Slot | Entry file offset | Raw bytes | VA | Target RVA | Source use |
| ---: | ---: | --- | ---: | ---: | --- |
| 1 | `0x00337020` | `D0 F7 0C 10` | `0x100CF7D0` | `0x000CF7D0` | `ConnectionStart` |
| 10 | `0x00337044` | `40 C1 17 10` | `0x1017C140` | `0x0017C140` | `ClientConnect` |
| 13 | `0x00337050` | `10 CC 17 10` | `0x1017CC10` | `0x0017CC10` | `ClientDisconnect` |
| 14 | `0x00337054` | `60 63 1C 10` | `0x101C6360` | `0x001C6360` | `GetPlayerSlot` |
| 31 | `0x00337098` | `10 8A 04 10` | `0x10048A10` | `0x00048A10` | `IsConnected` |
| 32 | `0x0033709C` | `20 8A 04 10` | `0x10048A20` | `0x00048A20` | `IsSpawned` |
| 33 | `0x003370A0` | `30 8A 04 10` | `0x10048A30` | `0x00048A30` | `IsActive` |

The primary table entry at slot 20 is at file offset `0x0033712C`, has raw
bytes `70 CC 17 10`, and points to RVA `0x0017CC70`.

### Server slot allocator: `GetFreeClient`

The target at RVA/file offset `0x00005B00` begins with:

```text
51 53 55 56 57 33 ED 8B F9 39 AF 20 01 00 00 89
6C 24 10 0F 8E 36 01 00 00 8D A4 24 00 00 00 00
```

```text
00005B00: 51                   push ecx
00005B01: 53                   push ebx
00005B02: 55                   push ebp
00005B03: 56                   push esi
00005B04: 57                   push edi
00005B05: 33 ED                xor ebp, ebp
00005B07: 8B F9                mov edi, ecx
00005B09: 39 AF 20 01 00 00    cmp dword ptr [edi + 0x120], ebp
00005B0F: 89 6C 24 10          mov dword ptr [esp + 0x10], ebp
00005B13: 0F 8E 36 01 00 00    jle 0x10005C4F
00005B20: 8B 87 14 01 00 00    mov eax, dword ptr [edi + 0x114]
00005B26: 8B 1C A8             mov ebx, dword ptr [eax + ebp*4]
00005B29: 8B 53 04             mov edx, dword ptr [ebx + 4]
00005B2C: 8B 82 88 00 00 00    mov eax, dword ptr [edx + 0x88]
00005B32: 8D 73 04             lea esi, [ebx + 4]
00005B35: 8B CE                mov ecx, esi
00005B37: FF D0                call eax
```

The non-null return path at `0x00005C24` is exactly
`5F 5E 5D 8B C3 5B 59 C2 04 00`; it restores registers, moves the selected
slot pointer from EBX to EAX, and returns with `ret 4`. The null path at
`0x00005C5D` is exactly `5F 5E 5D 33 C0 5B 59 C2 04 00`; it clears EAX and
also returns with `ret 4`.

Observation: ECX is the server object, the routine scans server-owned fields
including `server+0x120` and `server+0x114`, and returns a slot pointer or
NULL. The one stack value is cleaned by the callee. Interpretation: the
source `GetFreeClientFn` uses `__thiscall(server, netadr_t *)`; the source
passes a zeroed `NA_IP` address only as slot-allocation input and does not use
it as a GNS transport address.

### Server connection start and client connection

The `ConnectionStart` target at RVA/file offset `0x000CF7D0` begins with:

```text
51 53 55 56 57 8B E9 6A 1C 89 6C 24 14 E8 2E B2 FB FF
33 DB 83 C4 04 3B C3 74 1B 0F 57 C0 89 58 08 C7 00 EC 27 30 10
```

Its dispatch call at RVA/file offset `0x000CF81D` is exactly:

```text
8B 16 50 8B 42 74 8B CE FF D0
```

```text
000CF81D: 8B 16                mov edx, dword ptr [esi]
000CF81F: 50                   push eax
000CF820: 8B 42 74             mov eax, dword ptr [edx + 0x74]
000CF823: 8B CE                mov ecx, esi
000CF825: FF D0                call eax
```

The final native path at RVA/file offset `0x000CFBCD` is
`8B 42 74 8B CE FF D0 5F 5E 5D 5B 59 C2 04 00`, ending in `ret 4`.
Observation: the target uses ECX for the handler, one channel argument on
the stack, and calls adapter slot 29 with ECX set to the adapter and the
message pointer pushed. Interpretation: `ConnectionStartFn` is a one-stack-
argument `__thiscall` binding with callee cleanup.

The `ClientConnect` target at RVA/file offset `0x0017C140` begins with these
exact bytes:

```text
8B 44 24 14 8B 54 24 0C 53 56 57 50 8B 44 24 18
8B F9 8B 4C 24 20 51 8B 4C 24 18 52 50 51 8B CF
```

```text
0017C140: 8B 44 24 14          mov eax, dword ptr [esp + 0x14]
0017C144: 8B 54 24 0C          mov edx, dword ptr [esp + 0x0C]
0017C148: 53                   push ebx
0017C149: 56                   push esi
0017C14A: 57                   push edi
0017C14B: 50                   push eax
0017C14C: 8B 44 24 18          mov eax, dword ptr [esp + 0x18]
0017C150: 8B F9                mov edi, ecx
0017C152: 8B 4C 24 20          mov ecx, dword ptr [esp + 0x20]
0017C156: 51                   push ecx
0017C157: 8B 4C 24 18          mov ecx, dword ptr [esp + 0x18]
0017C15B: 52                   push edx
0017C15C: 50                   push eax
0017C15D: 51                   push ecx
0017C15E: 8B CF                mov ecx, edi
```

The native return bytes at RVA/file offset `0x0017C24D` are
`01 8B 50 1C 6A 00 56 FF D2 5F 5E 5B C2 14 00`; the return instruction is
`C2 14 00` (`ret 0x14`).

Observation: ECX is retained as the handler and the target consumes five
four-byte stack arguments. The entry reads the fifth stack argument at
`[ESP+0x14]` and the third at `[ESP+0x0C]`; the return removes 20 bytes.
Interpretation: the source call
`clientConnect(handler, "ASRD_GNS", 1, &adapter, false, NULL)` matches
`__thiscall(handler, name, userId, channel, fakePlayer, conVars)`. The final
NULL is a four-byte stack value.

### Server disconnect and client-state helpers

The `ClientDisconnect` target at RVA/file offset `0x0017CC10` begins with
these exact bytes:

```text
81 EC 00 04 00 00 56 8B B4 24 08 04 00 00 83 BE E4 00 00 00 00 74 32
8B 8C 24 0C 04 00 00 8D 84 24 10 04 00 00 50 51 8D 54 24 0C 68 00 04
00 00 52 E8 8A 85 0D 00 8D 44 24 14 50 68 54 7E 2F 10 56 E8 8A 1F F5 FF
83 C4 1C 5E 81 C4 00 04 00 00 C3
```

```text
0017CC10: 81 EC 00 04 00 00    sub esp, 0x400
0017CC16: 56                   push esi
0017CC17: 8B B4 24 08 04 00 00 mov esi, dword ptr [esp + 0x408]
0017CC1E: 83 BE E4 00 00 00 00 cmp dword ptr [esi + 0xE4], 0
0017CC25: 74 32                je 0x1017CC59
0017CC27: 8B 8C 24 0C 04 00 00 mov ecx, dword ptr [esp + 0x40C]
0017CC2E: 8D 84 24 10 04 00 00 lea eax, [esp + 0x410]
0017CC35: 50                   push eax
0017CC36: 51                   push ecx
0017CC37: 8D 54 24 0C          lea edx, [esp + 0x0C]
0017CC3B: 68 00 04 00 00       push 0x400
0017CC40: 52                   push edx
0017CC41: E8 8A 85 0D 00       call 0x102551D0
0017CC46: 8D 44 24 14          lea eax, [esp + 0x14]
0017CC4A: 50                   push eax
0017CC4B: 68 54 7E 2F 10       push 0x102F7E54
0017CC50: 56                   push esi
0017CC51: E8 8A 1F F5 FF       call 0x100CEBE0
0017CC56: 83 C4 1C             add esp, 0x1C
0017CC59: 5E                   pop esi
0017CC5A: 81 C4 00 04 00 00    add esp, 0x400
0017CC60: C3                   ret
```

Observation: after its local stack allocation, the target reads the first
stack value as the handler and the next stack value as the reason/variadic
area, reads handler offset `+0xE4`, and ends in plain `ret`. Interpretation:
the source's `ClientDisconnectFn` is a stack-only `__cdecl` call with the
handler as its first stack value and the reason as the next value; the caller
is responsible for stack cleanup. The public `IClient::Disconnect` declaration
corroborates the variadic reason contract, while the target bytes determine
the local call layout.

The helper targets are exact and complete:

```text
001C6360: 8B 41 2C    mov eax, dword ptr [ecx + 0x2C]
001C6363: C3          ret

00048A10: 33 C0       xor eax, eax
00048A12: 83 B9 E4 00 00 00 02  cmp dword ptr [ecx + 0xE4], 2
00048A19: 0F 9D C0    setge al
00048A1C: C3          ret

00048A20: 33 C0       xor eax, eax
00048A22: 83 B9 E4 00 00 00 03  cmp dword ptr [ecx + 0xE4], 3
00048A29: 0F 9D C0    setge al
00048A2C: C3          ret

00048A30: 33 C0       xor eax, eax
00048A32: 83 B9 E4 00 00 00 06  cmp dword ptr [ecx + 0xE4], 6
00048A39: 0F 94 C0    sete al
00048A3C: C3          ret
```

Their file offsets equal their RVAs: `0x001C6360`, `0x00048A10`,
`0x00048A20`, and `0x00048A30`. Observation: all use ECX as the handler
object, read handler offset `+0xE4`, return a Boolean in AL, and use plain
`ret`. The predicates are `GetPlayerSlot -> [handler+0x2C]`, connected when
state is at least 2, spawned when state is at least 3, and active when state
equals 6.

### Server signon-state method

The primary client table slot 20 points to RVA/file offset `0x0017CC70`. Its
exact entry bytes are:

```text
56 57 8B 7C 24 0C 83 FF 02 8B F1 75 70 8B 46 04
8B 90 A4 00 00 00 8D 4E 04 FF D2 84 C0 75 05 5F
5E C2 08 00
```

```text
0017CC70: 56                   push esi
0017CC71: 57                   push edi
0017CC72: 8B 7C 24 0C          mov edi, dword ptr [esp + 0x0C]
0017CC76: 83 FF 02             cmp edi, 2
0017CC79: 8B F1                mov esi, ecx
0017CC7B: 75 70                jne 0x1017CCED
0017CC7D: 8B 46 04             mov eax, dword ptr [esi + 4]
0017CC80: 8B 90 A4 00 00 00    mov edx, dword ptr [eax + 0xA4]
0017CC86: 8D 4E 04             lea ecx, [esi + 4]
0017CC89: FF D2                call edx
0017CC8B: 84 C0                test al, al
0017CC8D: 75 05                jne 0x1017CC94
0017CC8F: 5F                   pop edi
0017CC90: 5E                   pop esi
0017CC91: C2 08 00             ret 8
```

The state-2 continuation begins at file offset/RVA `0x0017CC94` with:

```text
8B 8E E4 00 00 00 D9 05 94 2E 30 10 8B 01 8B 90 80 00 00 00
6A 00 51 D9 1C 24 FF D2
```

```text
0017CC94: 8B 8E E4 00 00 00    mov ecx, dword ptr [esi + 0xE4]
0017CC9A: D9 05 94 2E 30 10    fld dword ptr [0x10302E94]
0017CCA0: 8B 01                mov eax, dword ptr [ecx]
0017CCA2: 8B 90 80 00 00 00    mov edx, dword ptr [eax + 0x80]
0017CCA8: 6A 00                push 0
0017CCAA: 51                   push ecx
0017CCAB: D9 1C 24             fstp dword ptr [esp]
0017CCAE: FF D2                call edx
```

Observation: this target receives the client object in ECX, state as the first
stack value, and spawn count as the second stack value; the native state-2
failure return is `ret 8`. The state-2 branch reads the handler pointer at
`client+4`, calls its vtable byte offset `+0xA4` (slot 41), then reads the
client/handler state field at `+0xE4` and dispatches handler byte offset
`+0x80` (slot 32). Only those decoded reads and calls are assigned meaning.
Interpretation: the source `SetSignonStateFn` uses
`__thiscall(client, state, spawnCount)` and invokes `(2, 0)` during server
context binding.

The raw signon state used by the source is at `client+0xE8`. Because the
handler subobject begins at `client+4`, this is the same physical field as
`handler+0xE4`, which is the field read by the three Boolean helpers and by
the target above. No additional flag meaning is inferred from the private
routine.

### Server local registration adapter

The server uses the same one-pointer, 75-entry x86 adapter shape. The table is
zeroed before installation; all unlisted entries remain NULL.

| Slot | Byte offset | Local callback |
| ---: | ---: | --- |
| 1 | `0x04` | `RegistrationAdapterGetAddress` |
| 9 | `0x24` | `RegistrationAdapterGetLatency` |
| 10 | `0x28` | `RegistrationAdapterGetAvgLatency` |
| 11 | `0x2C` | `RegistrationAdapterGetAvgLoss` |
| 17 | `0x44` | `RegistrationAdapterGetSequenceNr` |
| 22 | `0x58` | `RegistrationAdapterGetTimeSinceLastReceived` |
| 28 | `0x70` | `RegistrationAdapterSetDataRate` |
| 29 | `0x74` | `RegistrationAdapterRegisterMessage` |
| 32 | `0x80` | `RegistrationAdapterSetTimeout` |
| 36 | `0x90` | `RegistrationAdapterClear` |
| 37 | `0x94` | `RegistrationAdapterShutdown` |
| 41 | `0xA4` | `RegistrationAdapterSendNetMsg` |
| 42 | `0xA8` | `RegistrationAdapterSendData` |
| 47 | `0xBC` | `RegistrationAdapterSendDatagram` |
| 48 | `0xC0` | `RegistrationAdapterTransmit` |
| 51 | `0xCC` | `RegistrationAdapterGetDropNumber` |
| 57 | `0xE4` | `RegistrationAdapterCanPacket` |
| 58 | `0xE8` | `RegistrationAdapterIsOverflowed` |
| 59 | `0xEC` | `RegistrationAdapterIsTimedOut` |
| 60 | `0xF0` | `RegistrationAdapterHasPendingReliableData` |
| 61 | `0xF4` | `RegistrationAdapterSetFileTransmissionMode` |
| 62 | `0xF8` | `RegistrationAdapterSetCompressionMode` |
| 64 | `0x100` | `RegistrationAdapterSetMaxBufferSize` |
| 66 | `0x108` | `RegistrationAdapterGetNumBitsWritten` |
| 68 | `0x110` | `RegistrationAdapterSetRemoteFramerate` |
| 69 | `0x114` | `RegistrationAdapterSetMaxRoutablePayloadSize` |
| 74 | `0x128` | `RegistrationAdapterIsRemoteDisconnected` |

The adapter shims use `__fastcall(self, unused_edx, ...)`, with explicit
virtual parameters on the stack. `SendNetMsg` appends an `INetMessage` to the
pending server update; `SendData` appends the written `bf_write` bits after
validating overflow and size; `SendDatagram` appends any additional raw bits
and seals/flushes the update; `Transmit` seals/flushes; `GetNumBitsWritten`
and pending-data queries read bridge state; `GetDropNumber` returns the
processing-scoped command drop count; `Shutdown` closes/removes the mapped
connection after the Source context is torn down. Query defaults and
unexpected-self returns are bounded and fail closed.

The engine bytes directly prove the x86 dispatch mechanics and slot 29 on the
two `ConnectionStart` paths. The public channel declarations corroborate the
names and the complete slot ordering listed below; local callback addresses
are not engine RVAs.

### Server source context, raw fields, and flags

The bind sequence is guarded by the fixed PE identity, the runtime server
global pointer and vptr, the x86 check, and exact handler/primary table target
comparisons. It obtains a free slot, calls handler slot 1 to register
messages, calls handler slot 10 with five stack arguments and a NULL cvar
vector, calls primary slot 20 with `(state=2, spawnCount=0)`, and records the
client context only after these checks succeed. Terminal events invoke the
validated cdecl disconnect path; a failed disconnect validation leaves the
connection cleanup on the fail-closed fallback.

The source-local lifecycle flags are `s_initialized`,
`s_registrationAdapterReady`, `s_contextBound`, `s_signonStarted`,
`s_adapterShutdownLogged`, `s_gameplayContextInitialized`, and
`s_clientStateBindingLogged`, together with the bounded message-trace counters
for types 4 and 9. `RemoveConnection` clears context, signon, gameplay, slot,
state-observation, and trace state. These flags are not engine fields.

## Public interface corroboration

The following is corroboration for names, declared order, and public
parameter contracts. It does not replace the fixed-binary pointer, offset, or
target comparisons above.

`src/public/inetmsghandler.h` declares a virtual destructor at slot 0 and
`INetChannelHandler::ConnectionStart(INetChannel *)` at slot 1. Its remaining
handler methods occupy the following base slots. `src/public/iclient.h`
derives `IClient` from that handler and declares `Connect` at slot 10,
`Disconnect(const char *, ...)` at slot 13, and `GetPlayerSlot` at slot 14;
the declared `Connect` parameters are name, user ID, channel, fake-player
flag, and a nullable cvar-vector pointer. The observed engine target and
return instructions determine the exact build-specific call cleanup.

After the destructor slot, the declarations in
`src/public/inetchannelinfo.h` and `src/public/inetchannel.h` line up with the
adapter slot names used by both lifecycle files:

| Slot | Public declaration |
| ---: | --- |
| 1 | `GetAddress` |
| 6 | `IsLoopback` |
| 7 | `IsTimingOut` |
| 9 | `GetLatency` |
| 10 | `GetAvgLatency` |
| 11 | `GetAvgLoss` |
| 12 | `GetAvgChoke` |
| 14 | `GetAvgPackets` |
| 17 | `GetSequenceNr` |
| 22 | `GetTimeSinceLastReceived` |
| 25 | `GetRemoteFramerate` |
| 26 | `GetTimeoutSeconds` |
| 28 | `SetDataRate` |
| 29 | `RegisterMessage` |
| 32 | `SetTimeout` |
| 35 | `Reset` |
| 36 | `Clear` |
| 37 | `Shutdown` |
| 41 | `SendNetMsg` |
| 42 | `SendData` |
| 46 | `SetChoked` |
| 47 | `SendDatagram` |
| 48 | `Transmit` |
| 49 | `GetRemoteAddress` |
| 51 | `GetDropNumber` |
| 56 | `UpdateMessageStats` |
| 57 | `CanPacket` |
| 58 | `IsOverflowed` |
| 59 | `IsTimedOut` |
| 60 | `HasPendingReliableData` |
| 61 | `SetFileTransmissionMode` |
| 62 | `SetCompressionMode` |
| 64 | `SetMaxBufferSize` |
| 65 | `IsNull` |
| 66 | `GetNumBitsWritten` |
| 67 | `SetInterpolationAmount` |
| 68 | `SetRemoteFramerate` |
| 69 | `SetMaxRoutablePayloadSize` |
| 74 | `IsRemoteDisconnected` |

`src/public/inetmessage.h` declares `INetMessage::GetType`, which corroborates
the registry's type capture. It does not establish the private message-vtable
offsets `+0x1C` and `+0x04` observed in the registration target.

## Registry and source-interface boundary

### `src/game/shared/asrd_gns_message_registry.cpp/.h`

The registry is a process-local packed pointer record, not an engine-address
resolver. `SharedEntry` has `type` at `+0x00`, `messageAddress` at `+0x04`,
`channelAddress` at `+0x08`, and `handlerContextAddress` at `+0x0C`. The packed
`SharedRegistry` header has `magic` `+0x00`, `version` `+0x04`, `count`
`+0x08`, `captureCount` `+0x0C`, and `lookupCount` `+0x10`; its 512 entries
begin at `+0x14` and each occupy `0x10` bytes. Capture checks 32-bit pointer
width, calls the public `INetMessage::GetType`, and stores opaque process
addresses under a mutex. Lookup returns those opaque values by message type.

There is no direct engine.dll RVA, file offset, vtable target, or raw engine
object offset in this registry implementation. Runtime registration and the
client/server adapter callbacks are the binding boundaries that supply its
opaque pointers.

### Public wrapper ABI

`src/public/asrd_gns_wrapper.h` defines a C ABI with a `uint32_t`
`ASRD_GNS_Connection` token, integer connection-event values, and byte-buffer
send/receive functions. It exposes no native networking object and no fixed
engine.dll address. `src/game/gns_wrapper/asrd_gns_wrapper.cpp` implements the
token/event/transport bridge and likewise contains no direct engine.dll
binding. The public ABI therefore corroborates the lifecycle boundary but
does not corroborate any engine object layout.

## Files with no direct engine.dll evidence

The following reviewed files do not contain a fixed engine.dll RVA, expected
engine byte signature, engine vtable target, or raw engine-object offset. Their
GNS-related behavior is reached through the validated lifecycle APIs above:

| File | Boundary conclusion |
| --- | --- |
| `src/game/client/cdll_client_int.cpp` | Calls client frame, hook, bridge, and smoke APIs; no direct fixed binding. |
| `src/game/server/gameinterface.cpp` | Calls server init/frame/shutdown, hook, bridge, and smoke APIs; no direct fixed binding. |
| `src/game/shared/swarm/asw_gamerules.cpp` | Calls server lifecycle tracing and connection-count APIs; no direct fixed binding. |
| `src/game/shared/asrd_gns_message_bridge.cpp/.h` | Pure envelope, bitstream, queue, dispatch, and transport-boundary code; no direct engine.dll evidence. |
| `src/game/shared/asrd_gns_move_compat.cpp/.h` | CLC_Move bit parser and command-range classifier; no direct engine.dll evidence. |
| `src/game/shared/asrd_gns_smoke_probe.cpp/.h` | Wrapper smoke traffic and command-line probe; no direct engine.dll evidence. |
| `src/game/shared/asrd_gns_message_registry.cpp/.h` | Opaque registry boundary described above; no direct engine.dll evidence. |
| `src/game/shared/asrd_gns_runtime_hook.h` | Public declarations only; no direct engine.dll evidence. |
| `src/game/shared/asrd_gns_client_lifecycle.h` | Public lifecycle declarations and state enum only; no direct engine.dll evidence. |
| `src/game/shared/asrd_gns_server_lifecycle.h` | Public lifecycle declarations and context API only; no direct engine.dll evidence. |
| `src/public/asrd_gns_wrapper.h` | Public C ABI only; no direct engine.dll evidence. |

In particular, the bridge, parser, smoke, and public-ABI files must not be
given invented engine RVAs or vtable evidence. Their behavior is bounded by
the exported wrapper and lifecycle interfaces, while the fixed binary claims
remain confined to the three low-level implementation files documented above.

## Conservative conclusions

1. The hash, PE fields, section mapping, exact bytes, instruction boundaries,
   vtable entries, and raw offsets in this record identify one PE32 build only.
2. The runtime hook is safe to enable only when its image, target signatures,
   target ranges, executable sections, 32-bit process, and trampoline jump
   construction all pass. Connect takeover intentionally has no original-call
   fallback; registration preserves the original result before capture.
3. Client packet and signon bindings use distinct observed ABIs: packet start
   is ECX plus two stack values with `ret 8`; packet end is ECX with plain
   `ret`; client signon is ECX plus three stack values with `ret 0x0C`.
4. Server lifecycle bindings are also distinct: `ConnectionStart` is ECX plus
   one stack value with `ret 4`; `ClientConnect` is ECX plus five stack values
   with `ret 0x14`; disconnect is stack-only with plain `ret`; and server
   signon is ECX plus two stack values with `ret 8`.
5. Public headers corroborate declared names, ordering, and exposed parameter
   contracts. They do not prove private table addresses, object sizes, private
   state meanings, or local callback addresses.
6. The local adapter tables are source-owned ABI shims. The binary proves the
   engine dispatch slots at the listed call sites; unlisted entries remain
   NULL, and no local function-pointer address is represented as an engine
   file offset.
7. The server-global value, private object ownership, and higher-level effects
   not present in the decoded instructions remain runtime conditions. When a
   vtable, target, pointer width, or state prerequisite does not match, the
   lifecycle path rejects the binding or tears down the GNS generation.
