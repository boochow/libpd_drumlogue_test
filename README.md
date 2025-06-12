# libpd_drumlogue_test_synth

Run a pure-data patch as a synth unit on drumlogue

## How to build

clone this repository on `logue-sdk/platform/drumlogue`, then:
```
git submodule update --init --recursive
make install
```

## Pure Data patch

The pure-data patch is included as a string named `patch_str` in `synth.h`.
