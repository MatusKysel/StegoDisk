#StegoDisk#

Linux/OSX [![Build Status](https://travis-ci.org/MatusKysel/StegoDisk.svg?branch=master)](https://travis-ci.org/MatusKysel/StegoDisk)

Windows [![Build status](https://ci.appveyor.com/api/projects/status/0x4qk5yudw0o30m7?svg=true)](https://ci.appveyor.com/project/MatusKysel/stegodisk)


###Overview###
StegoDisk is cross platform steganographic file system ... TODO

###Download###
TODO

###Building###
This project is using CMake bulid system, so it will generate Make files for Unix systems as well as Solution files for Microsoft Visual Studio. 
```Bash
cmake -DCMAKE_BUILD_TYPE= { Debug | Release} .
```
####Running unit test####
For build verification do not forget to run prepared unit test. It is possible by this command
```Bash
make check
```
or by building check project in Visual studio solution.

###Usage###
Main interface is defined in stego_storage.h. This is simple example how to use this library

```C
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


