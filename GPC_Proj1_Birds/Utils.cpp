
#include <chrono>

static long long GetTime() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

static void AnimateClouds(float& yCloud1, float& yCloud2) {
	static long long lastUpdateMs = GetTime();
	const int updateInterval = 25;

	static bool increasing = true;
	static int currentStep = 0;
	const int nrSteps = 25;
	const float step = 0.1;

	if (lastUpdateMs + updateInterval <= GetTime())
		lastUpdateMs = GetTime();
	else
		return;

	if (increasing) {

		currentStep += 1;

		if (currentStep == nrSteps)
			increasing = false;

		yCloud1 += step;
		yCloud2 += step;
	}
	else {
		currentStep -= 1;

		if (currentStep == -nrSteps)
			increasing = true;

		yCloud1 -= step;
		yCloud2 -= step;
	}
}