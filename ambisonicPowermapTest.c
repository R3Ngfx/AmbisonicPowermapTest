#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <saf.h>
#include <powermap.h>
#include "powermap/_common.h"
#include "powermap/powermap.h"
#include "tinywav/tinywav.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char** argv) {
	// Check arguments
	if (argc < 4) {
		printf("Invalid arguments\nUse ./ambisonicTest input mode framerate\nMode:\n  0: PWD\n  1: MVDR\n  2: CroPaC\n");
		return 1;
	}
	// Load audio file
	TinyWav tw;
	tinywav_open_read(&tw, argv[1], TW_SPLIT);
	const int sampleRate = tw.h.SampleRate;
	const int channels = tw.numChannels;
	const int order = sqrt(channels)-1;
	const int blockSize = sampleRate/atoi(argv[3]);
	// Initialize powermap
	void* hPm;
	powermap_create(&hPm);
	powermap_init(hPm, sampleRate);
	powermap_setMasterOrder(hPm, order);
	powermap_setPowermapMode(hPm, atoi(argv[2]));
	powermap_setAnaOrderAllBands(hPm, order);
	powermap_setChOrder(hPm, CH_ACN);
	powermap_setNormType(hPm, NORM_SN3D);
	powermap_setDispFOV(hPm, HFOV_360);
	powermap_setAspectRatio(hPm, ASPECT_RATIO_2_1);
	powermap_setPowermapEQAllBands(hPm, 1);
	powermap_setNumSources(hPm, (int)((float)powermap_getNSHrequired(hPm)/2.0f));
	powermap_setSourcePreset(hPm, MIC_PRESET_IDEAL);
	powermap_setCovAvgCoeff(hPm, 0);
	powermap_setPowermapAvgCoeff(hPm, 0.666);
	powermap_refreshSettings(hPm);
	powermap_initCodec(hPm);
	// Process audio file
	int frame = 0;
	while (1) {
		// Get samples
		float samples[channels * blockSize];
		float* data[channels];
		for (int i = 0; i < channels; i++) {
			data[i] = samples + i*blockSize;
		}
		int samplesRead = tinywav_read_f(&tw, data, blockSize);
		if (samplesRead <= 0) break;
		// Process samples
		powermap_analysis(hPm, data, channels, blockSize, 1);
		float* dirsDeg, *pmap;
		int nDirs, width, hfov, aspectRatio;
		while (!powermap_getPmap(hPm, &dirsDeg, &pmap, &nDirs, &width, &hfov, &aspectRatio)) {
			printf("Frame %i not ready yet [%i%%]\n", frame, (int)(100*powermap_getProgressBar0_1(hPm)));
			powermap_requestPmapUpdate(hPm);
			powermap_analysis(hPm, data, channels, blockSize, 1);
			usleep(10000);
		}
		// Save frame
		int height = (int)((float)width/(float)aspectRatio+0.5f);
		unsigned char image[width][height];
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				image[i][j] = (char)(255*pmap[(height-j-1)*width+(width-i-1)]);
			}
		}
		char filename[20];
		sprintf(filename, "../out/%04d.png", frame);
		stbi_write_png(filename, width, height, 1, image, sizeof(unsigned char)*width);
		printf("Frame %i processed\n", frame);
		frame++;
	}
	printf("Ok!\n");
	powermap_destroy(&hPm);
	tinywav_close_read(&tw);
	return 0;
}
