#include "KBot.h"

KBot::KBot(void) 
{
	IterativeRobot::SetPeriod(1.0/PERIODIC_SPEED);
	
	m_Driver = new XBoxController(1);
	m_Operator = new XBoxController(2);
	
	m_Dashboard = SmartDashboard::GetInstance();
	
	m_FrontRightDrive = new Victor(FRONT_RIGHT_VICTOR);
	m_FrontLeftDrive = new Victor(FRONT_LEFT_VICTOR);
	m_BackRightDrive = new Victor(BACK_RIGHT_VICTOR);
	m_BackLeftDrive = new Victor(BACK_LEFT_VICTOR);

	m_robotDrive = new RobotDrive(m_FrontLeftDrive, m_BackLeftDrive, m_FrontRightDrive, m_BackRightDrive);
		 
	m_Intake = new Victor(INTAKE_VICTOR);
	m_Conveyor = new Victor(CONVEYOR_VICTOR);
	
	m_BottomShooter = new CANJaguar(BOTTOM_SHOOTER_CANJAG, CANJaguar::kSpeed);
	m_BottomShooter->ConfigEncoderCodesPerRev(360);
	m_BottomShooter->SetPID(1.0, 0.0, 0.0);
	m_BottomShooter->EnableControl();
	
	m_TopShooter = new CANJaguar(TOP_SHOOTER_CANJAG, CANJaguar::kSpeed);
	m_TopShooter->ConfigEncoderCodesPerRev(360);
	m_TopShooter->SetPID(1.0, 0.0, 0.0);
	m_TopShooter->EnableControl();
	
	m_Turret = new CANJaguar(TURRET_CANJAG, CANJaguar::kPercentVbus);
	
	m_RetroBottom = new DigitalInput (RETRO_BOTTOM);
	m_RetroTop = new DigitalInput (RETRO_TOP);
	m_TurretEncoder = new Encoder(TURRET_ENCODER_CH1, TURRET_ENCODER_CH2);
	m_TurretEncoder->Start();
	
	m_pCompressorRelay = new Relay(1,COMPRESSOR_SPIKE,Relay::kForwardOnly);
	m_pCompressorLimit = new DigitalInput(1, COMPRESSOR_LIMIT);
	m_ShootPiston = new Solenoid(SHOOT_SOLENOID);

	m_Timer = new Timer;
	
	Reset();
	
	printf("Oooh! KBot::KBot Completed.\n");
}// end


KBot::~KBot(void)
{
	printf("Oooh! KBot::~KBot Completed.\n");
}

void KBot::Reset(void)
{
	// set motors to 0
	m_FrontRightDrive->Set(0.0f);
	m_FrontLeftDrive->Set(0.0f);
	m_BackRightDrive->Set(0.0f);
	m_BackLeftDrive->Set(0.0f);
	m_Intake->Set(0.0f);
	m_Conveyor->Set(0.0f);
	m_BottomShooter->Set(0.0f);
	m_Turret->Set(0.0f);
	
	// set piston to false
	m_ShootPiston->Set(false);
	
	// Read shooter deltas from text file
	std::ifstream* in = new std::ifstream("shooterDeltas.txt");
	(*in) >> m_DeltaShooter;
	in->close();
	delete in;
	printf("manual delta %f \n", m_DeltaShooter);
	
	// Variables
	m_Speed = 0;
	m_TurnRate = 0;
	
	// auto speed init to 0
	m_TopShooterSpeed = 0;
	m_BottomShooterSpeed = 0;
	
	m_ShooterFlag = false;
	
	// state counters
	m_StateCounter = 0;
	m_autoState = 0;
	
	// counters
	m_IntakeCounter = 0;
	m_FireCounter = FIRE_DUR;
	m_ShootCounter = SHOOTER_SPEED_UP;
	m_CameraTestCounter = 0;
	m_AutoCounter = 0;
	m_ConveyorDur = 0;
	m_turretCounter = 0;
	
}

void KBot::RobotInit(void)
{
	GetWatchdog().SetExpiration(2.0);
	printf("Oooh! KBot::RobotInit Completed.\n");
}

void KBot::DisabledInit(void)
{
	printf("Oooh! KBot::DisabledInit Completed.\n");
}

void KBot::AutonomousInit(void)
{
	Reset();

	m_Timer->Reset();	
	m_Timer->Start();
	
	m_autoState = 0;
		
	printf("Oooh! KBot::AutonomousInit Completed.\n");
}

void KBot::TeleopInit(void) 
{	
	Reset();
	
	//std::ifstream* in = new std::ifstream("shooterValues.txt");
	//(*in) >> m_TopShooterSpeed;
	//(*in) >> m_BottomShooterSpeed;
	//in->close();
	//delete in;
	//printf("top %f bottom %f \n", m_autoTopShooterSpeed, m_autoBottomShooterSpeed);
	
	printf("Oooh! KBot::TeleopInit Completed. \n");

}

void KBot::DisabledPeriodic(void)
{
	GetWatchdog().Feed();
}

void KBot::AutonomousPeriodic(void)
{
	GetWatchdog().Feed();
	
	double timer = m_Timer->Get();
	std::cerr << "time: " << timer << " state: " << m_autoState << std::endl;
	Compressor();
	
	switch(m_autoState)
	{
		// init wait for timer
		case 0:
			AutoShoot(Shot_Nothing);
			m_Dashboard->PutInt("shooting", 0);
			
			if(timer > 0.5) // was 0.5;   MIN=0.5  MAX=8.5
				m_autoState = 1;
			break;
		
		// fire first ball
		case 1:
			AutoShoot(Shot_Auto);
			m_Dashboard->PutInt("shooting", 1);
			
			// ball has been shot
			if(!m_RetroTop->Get()) 
			{
				m_autoState = 2;
				m_autoCounter = AUTONOMOUS_DELAY;
			}
			
			break;
		
		// wait	
		case 2:
			m_autoCounter--;
			if(m_autoCounter <= 0)
				m_autoState = 3;
			break;
		
		// load next ball
		case 3:
			m_Conveyor->Set(CONVEYOR_SPEED);
			m_ShootPiston->Set(false);
			AutoShoot(Shot_Nothing);
			m_Dashboard->PutInt("shooting", 0);
			
			// ball is loaded
			if(m_RetroTop->Get())
				m_autoState = 4;
			break;
		
		case 4:	
			// time to fire next ball
			m_Conveyor->Set(0);
			if(timer > 8) // was 8  ;  MAX = 12  MIN=initial delay + 1.5
				m_autoState = 5;	
			break;
			
		// shoot second ball
		case 5:
			AutoShoot(Shot_Auto);
			m_Dashboard->PutInt("shooting", 1);
			
			// ball has been shot
			if(!m_RetroTop->Get()) 
			{
				m_autoState = 6;
				m_autoCounter = AUTONOMOUS_DELAY;
			}
			break;
		
		// wait	
		case 6:
			m_autoCounter--;
			if(m_autoCounter <= 0)
				m_autoState = 7;
			break;
		
		// done	
		case 7:
			m_TopShooter->Set(0.0);
			m_BottomShooter->Set(0.0);
			m_ShootPiston->Set(false);
			AutoShoot(Shot_Nothing);
			m_Dashboard->PutInt("shooting", 0);
			
			break;
			
	}// end switch
}

void KBot::TeleopPeriodic(void)
{	
	GetWatchdog().Feed();
	Compressor();
	
	m_Driver->RefreshController();
	m_Operator->RefreshController();
	
	//Driver
	DriveRoutine(m_Driver->GetLeftY(),-m_Driver->GetRightX(), m_Driver->GetTriggers());//forward backward speed, turn rate
	
	// Operator
	Intake(Deadband(m_Operator->GetLeftY()));
	Turret(m_Operator->GetTriggers());
	
	///////////////////////////////////////////////
	// Firing
	//
	// Auto Score
	if(m_Operator->RightBumper->Get())
	{
		m_Dashboard->PutInt("shooting", 1);
		AutoShoot(Shot_Auto);
	}
	// Manual 12 ft shot
	else if(m_Operator->XButton->Get())
	{
		m_Dashboard->PutInt("shooting", 1);
		AutoShoot(Shot_12ft);
	}
	// 2pt Fender Shot
	else if(m_Operator->BButton->Get())
	{
		m_Dashboard->PutInt("shooting", 1);
		AutoShoot(Shot_2pt_Fender);
	}
	// 3pt Fender Shot
	else if(m_Operator->YButton->Get())
	{
		m_Dashboard->PutInt("shooting", 1);
		AutoShoot(Shot_3pt_Fender);
	}
	// manual 2pt fender shot (failsafe)
	else if(m_Operator->LeftBumper->Get())
	{
		m_Dashboard->PutInt("shooting", 1);
		m_BottomShooter->Set(MAX_SHOOTER_POWER);
		m_TopShooter->Set(MAX_SHOOTER_POWER);
		
		Conveyor(m_Operator->GetRightY());
		ShootPiston(m_Operator->AButton->Get());
	}
	// Don't Shoot
	else
	{
		m_Dashboard->PutString("shooter_status", "IDLE");
		m_Dashboard->PutInt("shooting", 0);
		AutoShoot(Shot_Nothing);
		
		Conveyor(Deadband(m_Operator->GetRightY()));
		ShootPiston(m_Operator->AButton->Get());
	}
	 
	SetShooterDelta(m_Operator->StartButton->OnPress(), m_Operator->BackButton->OnPress()); 
	
	// send data to dash
	m_Dashboard->PutDouble("manual_shooter_delta",m_DeltaShooter);
	m_Dashboard->PutDouble("bottom_shooter",-m_BottomShooter->GetOutputVoltage());
	m_Dashboard->PutDouble("top_shooter",-m_TopShooter->GetOutputVoltage());
	
}// end TeleopPeriodic

float KBot::Deadband(float x)
{
	if (x>-OPERATOR_DEADBAND && x<OPERATOR_DEADBAND)
	{
		return 0.0f;
	}
	return x;
}

void KBot::SetShooterSpeedFromY(int y)
{
	// Calculate shooter speeds:
	
	const float A = -0.25; //-0.2449;
	const float B = -55.0; //-54.881;
	m_TopShooterSpeed = A * y + B + BACKSPIN + m_DeltaShooter;
	m_BottomShooterSpeed =  A * y + B - BACKSPIN + m_DeltaShooter;

	printf("topSpeed: %f bottomSpeed %f \n",m_TopShooterSpeed, m_BottomShooterSpeed);
}

void KBot::Compressor(void)
{
	// control the compressor based on pressure switch reading
	if (m_pCompressorLimit->Get() == 1)
	{
		m_pCompressorRelay->Set(Relay::kOff);
	}
	else
	{
		m_pCompressorRelay->Set(Relay::kOn);				
	}
}

void KBot::ShootPiston(const bool &btn)
{
	// Shooter Piston
	if (btn)
	{
		m_ShootPiston->Set(true);
	}
	else if(!btn)
	{
		m_ShootPiston->Set(false);
	}
	return; 
}

void writeShooterDeltas(float m_DeltaShooter)
{
	// Write shooter deltas to text file
	std::ofstream* out = new std::ofstream("shooterDeltas.txt");
	(*out) << m_DeltaShooter << std::endl;
	out->close();
	delete out;
}

void KBot::SetShooterDelta(const bool incSpeed, const bool decSpeed)
{
	if (incSpeed)
	{
		m_DeltaShooter += 1;
		writeShooterDeltas(m_DeltaShooter);
	}
	
	if (decSpeed)
	{
		m_DeltaShooter -= 1;
		writeShooterDeltas(m_DeltaShooter);
	}
}

void KBot::Turret(float axis)
{
	if (fabs(axis) > DEADBAND )
	{
		if (axis > 0)
		{
			axis = axis * axis;
		}
		else
		{
			axis = -axis * axis;
		}
		m_Turret->Set(axis*TURRET_SPEED);
	}
	else
	{
		m_Turret->Set(0.0);
	}
	return;
}

void KBot::AutoConveyor(void)
{
	if(m_RetroBottom->Get())
	{
		m_IntakeCounter = AUTO_CONVEYOR_DUR;
	}

	if(m_IntakeCounter > 0)
	{
		m_Conveyor->Set(CONVEYOR_SPEED);
		m_IntakeCounter--;
	}
	else 
	{
		m_Conveyor->Set(0.0);
	}
	return;
}

void KBot::Intake(float intakespeed)
{
	/*
	if (fabs(intakespeed) < DEADBAND) 
	{
		intakespeed = 0;
	}
	intakespeed = intakespeed * INTAKE_SPEED;
	
	m_Intake->Set(-intakespeed);
	*/
	
	if (intakespeed > DEADBAND) 
	{
		m_Intake->Set(-INTAKE_SPEED);
	}
	else if( intakespeed < -DEADBAND )
	{
		m_Intake->Set(INTAKE_SPEED);
	}
	else 
	{
		m_Intake->Set(0);
	}
	
	return;
}

void KBot::Conveyor(float conveyorSpeed)
{	
	if(fabs(conveyorSpeed) < DEADBAND) 
	{
		AutoConveyor();
	}
	else 
	{
		m_Conveyor->Set(-conveyorSpeed);
	}
	
	return;
}

float KBot::SineSquare (float input)
{
	if (input > 0)
	{
		input = input * input;
	}
	else if (input > 0)
	{
		input = -input * input;
	}
	return input;
}

void KBot::DriveRoutine(float left_y,float right_x, float extraTurn) 
{
	//check for left_y deadband
	if( fabs(left_y) < DEADBAND)
	{
		left_y = 0;
	}
	
	// check for right_x deadband
	if( fabs(right_x) < DEADBAND) 
	{
		right_x = 0;
	}
	
	// no turing on a dime
	if( fabs(right_x) > 0 && fabs(left_y) == 0)
	{
		right_x = 0;
	}
	
	left_y = SineSquare(left_y);
	right_x = SineSquare(right_x);
	
	// smoothing input
	m_TurnRate = (1 - SMOOTHING) * m_TurnRate + SMOOTHING * right_x;
	m_Speed = (1 - SMOOTHING) * m_Speed + SMOOTHING * left_y; //smoothing

	m_TurnRate += extraTurn;
	
	m_robotDrive->ArcadeDrive(-m_Speed, m_TurnRate*0.8);
	
	return;
}

void KBot::AutoAim(int &state, ShotTypes ShotType) // bool xcorrection, bool constantSpeed, bool fenderShot)
{	
	// AutoAim State Machine
	switch(state)
	{
		// Aim Turret	
		case 0:
			
			// use auto aim x correction
			if (ShotType == Shot_Auto)
			{
				int x_target = m_Dashboard->GetInt("x_target");
				
				// is there a target being tracked?
				if(x_target != 0)
				{
					// are we aimed?
					float diff = abs(x_target - (IMAGE_WIDTH / 2 - IMAGE_OFFSET));
					if ( diff <= AIM_TOL )
					{
						m_Turret->Set(0.0);
						m_turretCounter = TURRET_WAIT;
						// advance the state
						state = 1;
					}
					// off to the left, turn turret right
					else if(x_target < (IMAGE_WIDTH / 2 - IMAGE_OFFSET))
					{				
						m_Turret->Set(+TURRET_SPEED*(diff>20?1.0:0.5));
						// dont advance the state
						state = 0;
					}
					// off to the right, turn turret left			
					else if(x_target > (IMAGE_WIDTH / 2 - IMAGE_OFFSET))
					{
						m_Turret->Set(-TURRET_SPEED*(diff>20?1.0:0.5));
						// dont advance the state
						state = 0;
					}
				}
				// no target found don't change state
				else
				{
					state = 0;
				}
			}
			// don't use auto aiming x correction
			else 
			{
				state = 2;
			}
			
			break;
		
		// turret finished moving, wait a sec
		case 1:
			m_turretCounter--;
			if(m_turretCounter <= 0)
				//advance the state
				state = 2;
			else
				// dont advance the state
				state = 1;
			break;
		
		// get the width and set speeds
		case 2:
			if (ShotType == Shot_2pt_Fender)
			{
				m_TopShooterSpeed = MID_FENDER_TOP_SPEED;
				m_BottomShooterSpeed = MID_FENDER_BOTTOM_SPEED;
			}
			else if (ShotType == Shot_3pt_Fender)
			{
				m_TopShooterSpeed = TOP_FENDER_TOP_SPEED;
				m_BottomShooterSpeed = TOP_FENDER_BOTTOM_SPEED;
			}
			else if (ShotType == Shot_12ft)
			{
				m_TopShooterSpeed = MANUAL_KEY_TOP+m_DeltaShooter;
				m_BottomShooterSpeed = MANUAL_KEY_BOTTOM+m_DeltaShooter;
			}
			else
			{
				int y_target = m_Dashboard->GetInt("y_target");
				SetShooterSpeedFromY(y_target);
			}
			state = -1;
			break;
			
	}// end switch
	
	return;
}// end function

void KBot::AutoShoot(ShotTypes ShotType)
{
	if(ShotType != Shot_Nothing)
	{	
		if(m_RetroTop->Get())
		{	
			// ball is "loaded" stop conveyor, set flag
			m_Conveyor->Set(0);
			m_ShooterFlag = true;
		}
		else if(!m_ShooterFlag)
		{
			printf("running conveyor\n");
			m_Dashboard->PutString("shooter_status", "LOADING");
			m_Conveyor->Set(CONVEYOR_SPEED);
		}	
	}
	else
	{
		m_StateCounter = 0;
		m_FireCounter = FIRE_DUR;
		m_ShootCounter = SHOOTER_SPEED_UP; // reset autoaim
		m_ShooterFlag = false;
		m_BottomShooter->Set(0);
		m_TopShooter->Set(0);
	}
	
	// ball loaded, now Fire!
	if(m_ShooterFlag)
	{
		if(m_StateCounter == -1)
		{
			// shooter speeds have been set by AutoAim routine	
			m_BottomShooter->Set(m_BottomShooterSpeed);
			m_TopShooter->Set(m_TopShooterSpeed);
			
			m_Dashboard->PutString("shooter_status", "SHOOTING");
			printf("spinning up\n"); // leave this print
			
			--m_ShootCounter;
			if (m_ShootCounter <= 0) // 1/2 second for motor to speed up
			{
				// fire
				if(m_FireCounter >= 0)
				{
					printf("firing\n"); //leave this print
					//m_Dashboard->PutString("shooter_status", "FIRING");
					m_ShootPiston->Set(true);
					m_FireCounter--;
				}
				else
				{
					m_ShootPiston->Set(false);
					m_BottomShooter->Set(0);
					m_TopShooter->Set(0);
				}
			}
		}
		else
		{
			AutoAim(m_StateCounter, ShotType);
			m_Dashboard->PutString("shooter_status", "AIMING");
			m_FireCounter = FIRE_DUR;
			m_ShootCounter = SHOOTER_SPEED_UP;
		}
	}
	
	return;
}

START_ROBOT_CLASS(KBot);
