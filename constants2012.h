#ifndef CONSTANTS2012_H
#define CONSTANTS2012_H

// Constants
#define DEADBAND 0.12f
#define SMOOTHING 0.3f

#define OPERATOR_DEADBAND 0.05f

#define PERIODIC_SPEED 50 // Speed of periodic loops in Hz

#define CONVEYOR_SPEED 0.75f
#define TURRET_SPEED 0.2f
#define INTAKE_SPEED 0.3f

#define BACKSPIN 17.0f

// counters
#define FIRE_DUR 50
#define AUTO_CONVEYOR_DUR 55
#define TURRET_WAIT 25
#define SHOOTER_SPEED_UP 75
#define AUTONOMOUS_DELAY 25

//manual shooter speeds for 12 feet
#define MANUAL_KEY_TOP -52
#define MANUAL_KEY_BOTTOM -86

#define TOP_FENDER_TOP_SPEED 0
#define TOP_FENDER_BOTTOM_SPEED -105

#define MID_FENDER_TOP_SPEED 0
#define MID_FENDER_BOTTOM_SPEED -90

#define MAX_SHOOTER_POWER -100


// Imaging params
#define IMAGE_WIDTH 320
#define IMAGE_HEIGHT 240
#define AIM_TOL 1
#define IMAGE_OFFSET 21
#define IMAGE_Y_THRESHOLD 100

#endif
