# afl

A C++ library for interfacing with several Nintendo file formats.

Supported formats: 
* Yaz0
* SARC
* SZS (Yaz0-compressed SARC)
* BYML

In progress:
* BFFNT
* BFRES
* BNTX

## Building

```
mkdir build && cd build
cmake ..
make
```

## Credits

* [float16_t](https://github.com/fengwang/float16_t) for 16-bit floats
* [tinygltf](https://github.com/syoyo/tinygltf) for glTF format support
* https://nintendo-formats.com by [kinnay](https://github.com/kinnay)
* https://epd.zeldamods.org
* http://amnoid.de/gc/yaz0.txt
