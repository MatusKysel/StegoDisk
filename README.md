# StegoDisk

Linux/macOS/Windows [![CI](https://github.com/MatusKysel/StegoDisk/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/MatusKysel/StegoDisk/actions/workflows/ci.yml)

[![codecov](https://codecov.io/gh/MatusKysel/StegoDisk/branch/master/graph/badge.svg)](https://app.codecov.io/github/MatusKysel/StegoDisk)

### Overview
StegoDisk is cross platform steganographic library with a support of BMP, JPEG and PNG files. This library is using steganographic techniques for embedding data into carrier files. This libarary aslo comes with new interface for the Python programming language.

### Download
[Source code](https://github.com/MatusKysel/StegoDisk/releases/latest)

### Building
The build requires CMake 3.25 or newer and a compiler with C++17 support. CI uses CMake 4.4.3 on Linux, macOS, and Windows. Initialize the bundled dependencies before configuring:
```Bash
git submodule update --init --recursive
cmake -S . -B out -DCMAKE_BUILD_TYPE=Release
cmake --build out --config Release --parallel 4
```
No command-line policy override is needed for the bundled JPEG library. Use `-DCMAKE_BUILD_TYPE=Debug` for a Debug build with Makefiles or Ninja. With a multi-configuration generator such as Visual Studio or Ninja Multi-Config, select the configuration using `--config Debug` when building.
#### Running unit test
Build and run the project test suite with:
```Bash
cmake --build out --config Release --target check
```
Alternatively, build the `check` project in a Visual Studio solution.

For an out-of-source build in `out`, run the project tests after building with:
```Bash
ctest --test-dir out --build-config Release -L stegodisk --output-on-failure --no-tests=error
```
This includes regression tests for reconfiguration, capacity estimates, per-file encoders, and reopening storage.

#### Code coverage
The `Coverage` workflow runs the project tests in an instrumented GCC Debug build and uploads a Cobertura XML report to [Codecov](https://app.codecov.io/github/MatusKysel/StegoDisk). It uses GitHub OIDC authentication; no `CODECOV_TOKEN` secret is required. Public fork pull requests use the Codecov action's tokenless upload support.

Coverage includes the library sources compiled by the default build, including Keccak. Test code and the bundled `lib/` dependencies are excluded; optional FUSE code is not built by this workflow. Coverage percentage checks are informational while a baseline is established; test failures, failed uploads, and empty or near-empty reports fail the workflow. XML and browsable HTML reports are also available in the run's `coverage-reports` artifact. The badge shows the latest coverage uploaded for `master`, so it populates after the first successful run on that branch.

To generate the same reports locally on Linux with GCC 13, CMake, Ninja, TBB, and `gcovr==8.6` installed:

```sh
CC=gcc-13 CXX=g++-13 cmake -S . -B out-coverage -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DOPTIMIZE_FOR_NATIVE=OFF \
  -DCMAKE_CXX_FLAGS="--coverage -fprofile-update=atomic -fprofile-abs-path" \
  -DCMAKE_EXE_LINKER_FLAGS=--coverage -DCMAKE_SHARED_LINKER_FLAGS=--coverage
cmake --build out-coverage --parallel 4
ctest --test-dir out-coverage --build-config Debug -L stegodisk --output-on-failure --parallel 4 --timeout 600 --no-tests=error
gcovr --config gcovr.cfg --gcov-executable gcov-13 \
  --xml-pretty --xml out-coverage/coverage.xml \
  --html-details out-coverage/coverage.html --print-summary --fail-under-line 1 out-coverage
```

#### AddressSanitizer and UndefinedBehaviorSanitizer
The `Sanitizers` workflow runs the full project test suite with Clang 18 AddressSanitizer and UndefinedBehaviorSanitizer on Linux. It checks memory accesses, leaks, and undefined behavior; a detected error immediately fails the job. Failure logs are retained in the `sanitizer-test-logs` artifact.

To reproduce locally with Clang 18, its sanitizer runtimes, LLVM symbolizer, CMake, Ninja, and TBB installed:

```sh
CC=clang-18 CXX=clang++-18 cmake -S . -B out-sanitizers -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DOPTIMIZE_FOR_NATIVE=OFF \
  -DCMAKE_C_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all" \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all" \
  -DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined \
  -DCMAKE_SHARED_LINKER_FLAGS=-fsanitize=address,undefined
cmake --build out-sanitizers --parallel 4
ASAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer-18 \
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1:external_symbolizer_path=/usr/bin/llvm-symbolizer-18 \
ctest --test-dir out-sanitizers --build-config Debug -L stegodisk --output-on-failure --parallel 4 --timeout 600 --no-tests=error
```

#### Static analysis
The `Clang-Tidy` and `Cppcheck` workflows check the Linux Debug library sources and project headers. Clang-tidy uses the correctness checks in `.clang-tidy`; cppcheck enables its error and warning checks. Each workflow contains the commands for generating its compilation database and running the analyzer locally. Tests, bundled dependency translation units, and optional FUSE code are outside these analysis configurations.

Source findings are initially **advisory** while the existing backlog is reviewed. A green analysis job does not mean there are no findings. Tool, configuration, and parsing failures fail the job. Findings appear in the job summary and full diagnostics are retained in the `clang-tidy-reports` and `cppcheck-reports` artifacts.

#### Code formatting
The `Clang-format` workflow checks pull requests against `.clang-format`. That configuration was tuned to match the code already in the tree rather than to impose a new style, so reformatting is not required of existing files.

Only the lines a pull request actually changes are checked, because the tree predates the configuration and reformatting it wholesale would bury real changes in noise.

The workflow pins **clang-format 22.1.8**, matching `CLANG_VERSION` in `ci.yml`. Note this is deliberately not the Clang 18 used by the sanitizer and static analysis workflows above: formatting output can change between major releases, so the version is pinned independently of the compiler. Releases 22 and 23 were verified to format this tree identically, so either reproduces CI.

To check the lines you touched, with the same version CI uses:

```Bash
python3 -m pip install clang-format==22.1.8
python3 .github/scripts/check-format.py $(git merge-base HEAD origin/master) src
```

### Usage
Main interface is defined in stego_storage.h. This is simple example how to use this library

```C++
#include "stego_storage.h"

int main(int argc, char *argv[]) {

  std::unique_ptr<stego_disk::StegoStorage>
      stego_storage(new stego_disk::StegoStorage());

  stego_storage->Configure();

  stego_storage->Open(dir, password);

  stego_storage->Load();

  size = stego_storage->GetSize();

  stego_storage->Write(input, 0, input.size());

  stego_storage->Read(output, 0, input.size());

  stego_storage->Save();

  return 0;
}
```
### Configuration
##### JSON configuration
The configuration itself through external files is an excellent security option for a file system like this, because in the case when this configuration file is a secret it can serve as a secret key for the whole file system. This is true only if the file system has a large number of possible configurations parameters, so in the current form of this library, where there are only a few configuration parameters, it is not secure to use only the configuration file as the secret key. The Configuration example:

```json
{
   "encoder":"hamming",
   "glob_perm":"mix_feistel",
   "local_perm":"mix_feistel",
   "file_types":[
      {
         "file_type":"jpg",
         "permutation":"mix_feistel",
         "encoder":"hamming"
      }
   ],
   "exclude_types":[
      "bmp",
      "png"
   ]
}
```

In this configuration, all parameters are filled in, but in the event when some of them stayed unfilled steganographic file system will replace them by the default parameters. The first part of the configuration file global parameters are set, such as an encoder and a permutation, you can also specify these parameters per individual file type, and in the last part filters for file type exclusion are set.

File-specific encoder overrides are now applied when loading and saving. Older versions ignored these encoder overrides and used the global encoder for every file. To reopen an existing store written by those versions, set each file-specific encoder to the global encoder that was used when the store was saved; keep the permutation settings unchanged.

##### Enum configuration
As the standard way to configure systems is configuration using enumerated types, which are defined for the individual parameters.
This method is more intuitive for programmers and most likely it will be the most used form of configuration for this steganographic file system. An example of the configuration by this method:
```C++
// Function defintion
void Configure( const EncoderFactory::EncoderType encoder,
                const PermutationFactory::PermutationType global_perm,
                const PermutationFactory::PermutationType local_perm ) const ;

// Function usage
stego_storage->Configure( StegoStorage::EncoderFactory::LSB,
                          StegoStorage::PermutationFactory::AFFINE,
                          StegoStorage::PermutationFactory::FEISTEL_MIX );
```
