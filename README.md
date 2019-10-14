This is a collection of miscellaneous code bits that are experiments or just a misc codebase.

Some experiments include things like color quantization. This means that some of this code may be easily reusable and stable, while other bits may be purely experimental or meant a a demonstration.

Building is done with the [meson](https://mesonbuild.com) build system. Usage is fairly simple:

    make build
    cd build
    meson .. --buildtype=release
    ninja

Use `--buildtype=debug` instead of `--buildtype=release` for development / debugging builds.
