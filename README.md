# StegoDisk

Linux/OSX [![Build Status](https://travis-ci.org/MatusKysel/StegoDisk.svg?branch=master)](https://travis-ci.org/MatusKysel/StegoDisk)

Windows [![Build status](https://ci.appveyor.com/api/projects/status/0x4qk5yudw0o30m7?svg=true)](https://ci.appveyor.com/project/MatusKysel/stegodisk)


### Overview
StegoDisk is cross platform steganographic library with a support of BMP, JPEG and PNG files. This library is using steganographic techniques for embedding data into carrier files. This libarary aslo comes with new interface for the Python programming language.

### Download
[Source code](https://github.com/MatusKysel/StegoDisk/releases/latest)

### Building
This project is using CMake bulid system, so it will generate Make files for Unix systems (gcc 4.8/clang 3.4 or greater) as well as Solution files for Microsoft Visual Studio (Visual Studio 2013 or greater).
```Bash
cmake -DCMAKE_BUILD_TYPE= { Debug | Release} .
```
#### Running unit test
For build verification do not forget to run prepared unit test. It is possible by this command
```Bash
make check
```
or by building check project in Visual studio solution.

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
