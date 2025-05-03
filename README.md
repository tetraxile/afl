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
* [cgltf](https://github.com/jkuhlmann/cgltf) for glTF format support
