#pragma once
#include <fstream>
#include <iomanip>
#include <cmath>
#include <string>
#include <iostream>
#include <vector>
using namespace std;

// Takes a note and returns its frequency
// Based on https://gist.github.com/stuartmemo/3766449

float GetFrequency(string note)
{
    std::vector<std::string> notes = { "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#" };
    float octave = (note.back() - '0');
    note.pop_back();

    float keyNumber = std::distance(notes.begin(), std::find(notes.begin(), notes.end(), note));

    if (keyNumber < 3)
    {
        keyNumber += octave * 12 + 1;
    }
    else 
    {
        keyNumber += (octave - 1) * 12 + 1;
    }

    float freq = 440 * pow(2, (keyNumber - 49) / 12);

    return freq;
}

namespace little_endian_io
{
    template <typename Word>
    std::ostream& write_word(std::ostream& outs, Word value, unsigned size = sizeof(Word))
    {
        for (; size; --size, value >>= 8)
            outs.put(static_cast <char> (value & 0xFF));
        return outs;
    }
}
using namespace little_endian_io;

bool CreateWavFile(string note)
{
    ofstream f("./Sounds/" + note + ".wav", ios::binary);

    // Write the file headers
    f << "RIFF----WAVEfmt ";     // (chunk size to be filled in later)
    write_word(f, 16, 4);  // no extension data
    write_word(f, 1, 2);  // PCM - integer samples
    write_word(f, 2, 2);  // two channels (stereo file)
    write_word(f, 44100, 4);  // samples per second (Hz)
    write_word(f, 176400, 4);  // (Sample Rate * BitsPerSample * Channels) / 8
    write_word(f, 4, 2);  // data block size (size of two integer samples, one for each channel, in bytes)
    write_word(f, 16, 2);  // number of bits per sample (use a multiple of 8)

    // Write the data chunk header
    size_t data_chunk_pos = f.tellp();
    f << "data----";  // (chunk size to be filled in later)

    // Write the audio samples
    constexpr double two_pi = 6.283185307179586476925286766559;
    constexpr double max_amplitude = 32760;  // "volume"

    double hz = 44100;    // samples per second

    // Work out from note input which note to make

    double frequency = GetFrequency(note);

    double seconds = 1;      // time

    int N = hz * seconds;  // total number of samples
    for (int n = 0; n < N; n++)
    {
        double amplitude = (double)n / N * max_amplitude;
        double value = sin((two_pi * n * frequency) / hz);
        write_word(f, (int)(amplitude * value), 2);
        write_word(f, (int)((max_amplitude - amplitude) * value), 2);
    }

    // (We'll need the final file size to fix the chunk sizes above)
    size_t file_length = f.tellp();

    // Fix the data chunk header to contain the data size
    f.seekp(data_chunk_pos + 4);
    write_word(f, file_length - data_chunk_pos + 8);

    // Fix the file header to contain the proper RIFF chunk size, which is (file size - 8) bytes
    f.seekp(0 + 4);
    write_word(f, file_length - 8, 4);

    return true;
}