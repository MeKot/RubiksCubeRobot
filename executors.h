#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "BrickPi.h"
#include "tick.h"

//Perfect flip is 1000 ms at 60
#define ARM_MOTOR PORT_A
#define BASE_MOTOR PORT_D
#define ARM_SPEED_LOCK        115
#define ARM_SPEED_RESET       70
#define ARM_SPEED_UNLOCK      105
#define ARM_TIMEOUT_LOCK      170 
#define ARM_TIMEOUT_ROTATE    95
#define ARM_TIMEOUT_RESET     90
#define ARM_TIMEOUT_UNLOCK    190 
#define BASE_TIMEOUT          220
#define BASE_SPEED_FREE       73
#define BASE_SPEED_FORCED     127
#define BASE_RESET_TIMEOUT    5
#define BASE_RESET_SPEED      80
#define REST_TIME             3 

static void lockS(void) { 
	BrickPi.Timeout = ARM_TIMEOUT_LOCK;
	BrickPiSetTimeout();
	BrickPi.MotorSpeed[ARM_MOTOR]  = ARM_SPEED_LOCK;
	BrickPi.MotorSpeed[BASE_MOTOR] = 0;
	while (BrickPiUpdateValues() < 0) {
		printf("Failed to update motors in lock");
	}
	sleep(REST_TIME);
}

//places the arm on to the right position
static void rotateArm(void) {
	BrickPi.Timeout = ARM_TIMEOUT_ROTATE;
	BrickPiSetTimeout();
	BrickPi.MotorSpeed[ARM_MOTOR]  = ARM_SPEED_LOCK;
	BrickPi.MotorSpeed[BASE_MOTOR] = 0;
	while (BrickPiUpdateValues() < 0) {
		printf("Failed to update motors in rotateArm");
	}
	sleep(REST_TIME);
}

static void resetArm(int speed) {
	BrickPi.Timeout = ARM_TIMEOUT_RESET;
	BrickPiSetTimeout();
	BrickPi.MotorSpeed[ARM_MOTOR]  = -speed;
	BrickPi.MotorSpeed[BASE_MOTOR] = 0;
	while (BrickPiUpdateValues() < 0) {
		printf("Failed to update motors in resetArm");
	}
	sleep(REST_TIME);
}

void lock(void) {
	lockS();
	sleep(1);
	resetArm(ARM_SPEED_RESET - 20);
}

void unlock(void) {
	BrickPi.Timeout = ARM_TIMEOUT_UNLOCK;
	BrickPiSetTimeout();
	BrickPi.MotorSpeed[ARM_MOTOR]  = -ARM_SPEED_UNLOCK;
	BrickPi.MotorSpeed[BASE_MOTOR] = 0;
	while (BrickPiUpdateValues() < 0) {
		printf("Failed to update motors in unlock");
	}
	sleep(REST_TIME);
}

void flip(void) {
	lockS();
	lockS();
	rotateArm();
	resetArm(ARM_SPEED_RESET);
}

static void baseReset(int speed) {
	BrickPi.Timeout = BASE_RESET_TIMEOUT;
	BrickPiSetTimeout();
	BrickPi.MotorSpeed[BASE_MOTOR] = speed;
	BrickPi.MotorSpeed[ARM_MOTOR]  = 0;
	while (BrickPiUpdateValues() < 0) {
		printf("Failed to update values in base reset");
	}
	sleep(REST_TIME);
}

static void rotator(int speed) {
	BrickPi.Timeout = BASE_TIMEOUT;
	BrickPiSetTimeout();
	BrickPi.MotorSpeed[BASE_MOTOR] = speed;
	BrickPi.MotorSpeed[ARM_MOTOR]  = 0;
	int enc = BrickPi.Encoder[BASE_MOTOR];
	while (BrickPiUpdateValues() < 0) {
		printf("Failed to update motors in rotator");
	}
	sleep(REST_TIME);
}

void horizontalClockwise(void) {
	rotator(BASE_SPEED_FORCED);
	rotator(BASE_SPEED_FORCED);
	rotator(BASE_SPEED_FORCED);
	baseReset(-BASE_RESET_SPEED);
}

void horizontalClockwiseFree(void) {
	rotator(BASE_SPEED_FREE);
	//rotator(BASE_SPEED_FREE);
	rotator(BASE_SPEED_FREE);
}

void horizontalAnticlockwise(void) {
	rotator(-BASE_SPEED_FORCED);
	rotator(-BASE_SPEED_FORCED);
	rotator(-BASE_SPEED_FORCED);
	baseReset(BASE_RESET_SPEED);
}

void horizontalAnticlockwiseFree(void) {
	rotator(-BASE_SPEED_FREE);
	//rotator(-BASE_SPEED_FREE);
	rotator(-BASE_SPEED_FREE);
}

void horizontalDoubleFree(void) {
	rotator(BASE_SPEED_FREE);
	rotator(BASE_SPEED_FREE);
	rotator(BASE_SPEED_FREE);
	rotator(BASE_SPEED_FREE);
}

void horizontalDouble(void) {
	rotator(BASE_SPEED_FORCED);
	rotator(BASE_SPEED_FORCED);
	rotator(BASE_SPEED_FORCED);
	rotator(BASE_SPEED_FORCED);
	rotator(BASE_SPEED_FORCED);
	sleep(1);
	baseReset(-BASE_RESET_SPEED);
}


