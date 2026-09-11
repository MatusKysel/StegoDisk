# StegoDisk

Linux/macOS/Windows [![CI](https://github.com/MatusKysel/StegoDisk/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/MatusKysel/StegoDisk/actions/workflows/ci.yml)

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
