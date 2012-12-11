#ifndef KBOT_H
#define KBOT_H

// Standard includes
#include "math.h"
#include <fstream>

// FRC includes
#include "WPILib.h"
#include "IterativeRobot.h"

// Our includes
#include "XBoxController.h"
#include "mappings2012.h"
#include "constants2012.h"

class KBot : public IterativeRobot {

public:
	KBot();
	~KBot();
	
	void RobotInit(void);
	void DisabledInit(void);
	void AutonomousInit(void);
	void TeleopInit(void);
	
	void DisabledPeriodic(void);
	void AutonomousPeriodic(void);
	void TeleopPeriodic(void);
	
	void DisabledContinuous(void){};
	void AutonomousContinuous(void){};
	void TeleopContinuous(void){};
	
private:
	
	// Members
	XBoxController *m_Driver;
	XBoxController *m_Operator;
	
	SmartDashboard *m_Dashboard;
	
	Victor *m_FrontRightDrive;
	Victor *m_BackRightDrive;
	Victor *m_FrontLeftDrive;
	Victor *m_BackLeftDrive;
	Victor *m_Intake;
	Victor *m_Conveyor;
	
	CANJaguar *m_BottomShooter;
	CANJaguar *m_TopShooter;
	CANJaguar *m_Turret;
	
	RobotDrive *m_robotDrive;
	
	DigitalInput *m_RetroBottom;
	DigitalInput *m_RetroTop;
	Encoder *m_TurretEncoder;
	
	Relay	*m_pCompressorRelay;
	DigitalInput *m_pCompressorLimit;
	Solenoid *m_ShootPiston;
	
	Timer *m_Timer;
	
	// Private Methods
	void Reset(void);
	
	void DriveRoutine(float left_y,float right_x, float extraTurn = 0);
	float SineSquare (float input);
	
	float Deadband(float x);

	
	void Intake(float intakespeed);
	void Conveyor(float conveyorspeed);
	void AutoConveyor();
	
	void Turret(float axis);
	
	void SetShooterSpeedFromY(int y);
	void SetShooterDelta(const bool incSpeed, const bool decSpeed);
	
	enum ShotTypes
	{
		Shot_Nothing,
		Shot_Auto,
		Shot_12ft,
		Shot_2pt_Fender,
		Shot_3pt_Fender
	};
	
	void AutoAim(int &state, ShotTypes ShotType);//bool xcorrection, bool constantSpeed, bool fenderShot);
	void AutoShoot(ShotTypes ShotType);//bool btn, bool xCorrection=true, bool calcDistance=true, bool fenderShot=false);
	
	void Compressor();
	void ShootPiston(const bool &btn);
	
	// Member Variables
	float m_TurnRate;
	float m_Speed;
	float m_TopShooterSpeed;
	float m_BottomShooterSpeed;
	float m_DeltaShooter;
	
	bool m_ShooterFlag;
	
	// state counters
	int m_StateCounter;
	int m_autoState;
	
	// counters
	int m_ShootCounter;
	int m_CameraTestCounter;
	int m_IntakeCounter;
	int m_FireCounter;
	int m_AutoCounter;
	int m_ConveyorDur;
	int m_autoCounter;
	int m_turretCounter;

};
#endif
