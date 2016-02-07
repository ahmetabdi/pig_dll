#include <Windows.h>

extern int Acceleration;
void SpeedHack(bool Enabled);
int ExitSpeedHack();

class Speed {
public:
	//CONSTRUCTOR
	Speed() {
		//INITIALIZE
	}
public:
	void setSpeedMultiplier(int speed) {
		Acceleration = speed;
	}
	void enableSpeedHack() {
		SpeedHack(true);
	}
	void disableSpeedHack() {
		SpeedHack(false);
	}
};