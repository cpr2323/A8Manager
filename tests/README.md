# Regression probes

These are repository-hosted versions of the temporary parser/CV and stereo split
probes used while validating the PR. They reconstruct the same checks, with
additional diagnostic-type, recovery, sample-rate, and finite-sample assertions.
They compile the real application implementation, use the pinned JUCE/oolib
submodules, and need neither an Assimil8or nor external sample files. Application
logging is stubbed out. These are focused regression probes, not a full test suite
or GUI/threading stress tests.

From the project root:

```sh
git submodule update --init --recursive
cmake -S . -B cmake_build -DCMAKE_BUILD_TYPE=Debug -DA8MANAGER_BUILD_TESTS=ON
cmake --build cmake_build --config Debug --target A8ManagerRegressionTests
ctest --test-dir cmake_build -C Debug --output-on-failure
```

The `--config`/`-C` options also select Debug for multi-configuration generators
such as Visual Studio. Tests are disabled by default; normal application builds
are unchanged. The probes were verified on macOS; other platforms have not been
verified. They do not open an application window or audio device.

CTest registers two tests, each with a 60-second timeout. Use `ctest --test-dir
cmake_build -C Debug -V` for detailed output or add `-R ParserCvRegression` /
`-R StereoSplitRegression` to select one test. Failed checks return a nonzero exit
code, including in Release builds.

- **ParserCvRegression:** unknown parameters at global, preset, channel, and zone
  scopes produce diagnostics without losing the scope of subsequent known
  parameters. The same parser is reused 100 times, including a zone-to-channel
  transition. Checks `CV A` to `0A` normalization for Zones CV, `CV B` to `0B`
  normalization in CV/amount formatting, a missing CV/amount delimiter, and error
  reset/recovery on the next parse.
- **StereoSplitRegression:** generates 16-bit and 24-bit, 48 kHz stereo WAVs with
  different left/right signals and 8,193 samples per channel, then calls
  `AudioManager::splitStereoIntoTwoMono()`. Checks both mono outputs' channel
  counts, exact lengths, bit depths, sample rates, and every sample (including the
  tails). Comparisons allow one PCM quantization step plus a 10% margin for float
  re-encoding, not arbitrary waveform differences. Generated files use unique
  temporary names and are removed on normal completion or a failed assertion.

The stereo content checks catch the original byte-count bug: copying 16-/24-bit
file-sized byte counts from a float buffer leaves part of each output channel
uncopied. Merely checking output file existence and length would miss it.
