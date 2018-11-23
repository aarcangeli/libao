#include <ao/ao.h>
#include <math.h>

// C major
const int notes[] = {
        0, 2, 4, 5, 7, 9, 11, 12,
        12, 11, 9, 7, 5, 4, 2, 0
};
const double volume = 0.3;
const double bpm = 220;
const double smooth = 0.1;

int main() {
    const int notesCount = sizeof(notes) / sizeof(int);

    const int sampleRate = 44100;
    const int bufferTime = 2;
    const int bufferSize = bufferTime * sampleRate;

    const double smoothStep = smooth * sampleRate;

    const int samplePerBeat = (int) (sampleRate / (bpm / 60.));
    const int lastSample = notesCount * samplePerBeat;

    printf("Initializing\n");
    ao_initialize();

    int driver = ao_default_driver_id();
    printf("  + driver id: %i\n", driver);

    ao_sample_format sformat;
    sformat.bits = 16;
    sformat.channels = 1;
    sformat.rate = sampleRate;
    sformat.byte_format = AO_FMT_NATIVE;
    sformat.matrix = 0;

    ao_device *adevice = ao_open_live(driver, &sformat, NULL);

    int i = 0;
    double cycle = 0;
    short buffer[bufferSize];

    while (i < lastSample) {
        uint32_t j = 0;
        for (; j < bufferSize && i < lastSample; ++j) {
            int nota = i / samplePerBeat;

            double freq = 440 * pow(2, notes[nota] / 12.);
            cycle += sin(1 / 44100. * freq);

            double sample = sin(cycle * 2 * M_PI);

            // smooth step
            if (i < smoothStep) {
                sample *= sin(i / smoothStep * M_PI_2);
            }
            if (i > lastSample - smoothStep) {
                sample *= sin((lastSample - i) / smoothStep * M_PI_2);
            }

            buffer[j] = (short) ((sample * 0x7fff) * volume);
            i++;
        }

        printf("Writed %i samples to device\n", j * 2);
        if (!ao_play(adevice, (char *) buffer, j * 2)) break;
    }

    printf("Flushing device\n");
    ao_close(adevice);

    printf("Closing\n");
    ao_shutdown();
    return 0;
}
