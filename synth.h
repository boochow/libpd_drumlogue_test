#pragma once
/*
 *  File: synth.h
 *
 *  Simple Synth using libpd
 *
 */

#include <atomic>
#include <cstddef>
#include <cstdint>
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <fcntl.h>

#include <arm_neon.h>

#include "unit.h"

#include "z_libpd.h"

#ifndef MFD_CLOEXEC
#define MFD_CLOEXEC 0x0001
#endif

#ifndef SYS_memfd_create
#if defined(__x86_64__)
#define SYS_memfd_create 319
#elif defined(__aarch64__)
#define SYS_memfd_create 279
#else
#error "SYS_memfd_create not defined for this architecture"
#endif
#endif

const char *patch_str = "#N canvas 1178 268 647 665 12;\
#X obj 25 58 mtof;\
#X obj 25 148 *~;\
#X obj 25 185 dac~;\
#X obj 25 101 osc~;\
#X obj 95 111 line~;\
#X obj 67 51 select 0;\
#X obj 120 80 / 127;\
#X msg 67 80 0 250;\
#X obj 25 18 notein;\
#X connect 0 0 3 0;\
#X connect 1 0 2 0;\
#X connect 1 0 2 1;\
#X connect 3 0 1 0;\
#X connect 4 0 1 1;\
#X connect 5 0 7 0;\
#X connect 5 1 6 0;\
#X connect 6 0 4 0;\
#X connect 7 0 4 0;\
#X connect 8 0 0 0;\
#X connect 8 1 5 0;\
";

static int memfd_create(const char *name, unsigned int flags) {
    return syscall(SYS_memfd_create, name, flags);
}

// initialize libpd and load a patch
static int setup_pd(char *patch_path) {

    if (libpd_init() != 0) {
        return 1;
    }

    libpd_init_audio(0, 2, 48000);

    // parameters should be (file, dir) but this seems to work
    void *patch = libpd_openfile(patch_path, ".");
    if (!patch) {
        return 2;
    }

    // Turn DSP on
    libpd_start_message(1);
    libpd_add_float(1.0f);
    libpd_finish_message("pd", "dsp");

    return 0;
}

class Synth {
public:
    Synth(void) {}
    ~Synth(void) {}

    inline int8_t Init(const unit_runtime_desc_t * desc) {

        if (desc->samplerate != 48000)
            return k_unit_err_samplerate;

        if (desc->output_channels != 2)  // should be stereo output
            return k_unit_err_geometry;

        // create a file then write patch_str into it
        int fd = memfd_create("embedded_patch", MFD_CLOEXEC);
        if (fd < 0) {
            return k_unit_err_undef;
        }

        size_t len = strlen(patch_str);
        if (write(fd, patch_str, len) != len) {
            close(fd);
            return k_unit_err_undef;
        }

        // make a temporary path
        char path[64];
        snprintf(path, sizeof(path), "/proc/self/fd/%d", fd);

        // init libpd and load a patch from the path
        setup_pd(path);

        close(fd); // unlink is not needed; memfd is released when it is closed

        blocksize_ = libpd_blocksize(); // usually 64 (frames / tick)
        return k_unit_err_none;
    }

    inline void Teardown() {
    }

    inline void Reset() {
    }

    inline void Resume() {
    }

    inline void Suspend() {
    }

    fast_inline void Render(float * out, size_t frames) {
        static float input_buffer[1024];

        memset(input_buffer, 0, sizeof(input_buffer));
        while(frames > 0) {
            libpd_process_float(1, input_buffer, out);
            frames -= blocksize_;
        }
    }

    inline void setParameter(uint8_t index, int32_t value) {
        (void)value;
        switch (index) {
        case 0:
            note_ = MIN(127, MAX(0, value));
            break;
        default:
            break;
        }
    }

    inline int32_t getParameterValue(uint8_t index) const {
        switch (index) {
        default:
            break;
        }
        return 0;
    }

    inline const char * getParameterStrValue(uint8_t index, int32_t value) const {
        (void)value;
        switch (index) {
        default:
            break;
        }
        return nullptr;
    }

    inline const uint8_t * getParameterBmpValue(uint8_t index,
                                              int32_t value) const {
        (void)value;
        switch (index) {

        default:
            break;
        }
        return nullptr;
    }

    inline void NoteOn(uint8_t note, uint8_t velocity) {
        libpd_noteon(0, note, velocity);
    }

    inline void NoteOff(uint8_t note) {
        libpd_noteon(0, note, 0);
    }

    inline void GateOn(uint8_t velocity) {
        libpd_noteon(0, note_, 127);
    }

    inline void GateOff() {
        libpd_noteon(0, note_, 0);
    }

    inline void AllNoteOff() {}

    inline void PitchBend(uint16_t bend) {
        libpd_pitchbend(0, bend);
    }

    inline void ChannelPressure(uint8_t pressure) {
        libpd_aftertouch(0, pressure);
    }

    inline void Aftertouch(uint8_t note, uint8_t aftertouch) {
        libpd_polyaftertouch(0, note, aftertouch);
    }

    inline void LoadPreset(uint8_t idx) { (void)idx; }

    inline uint8_t getPresetIndex() const { return 0; }

    static inline const char * getPresetName(uint8_t idx) {
        (void)idx;
        return nullptr;
    }

private:
    std::atomic_uint_fast32_t flags_;
    int blocksize_;
    int note_;
};
