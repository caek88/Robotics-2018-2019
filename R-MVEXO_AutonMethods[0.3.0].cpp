/*--------------------------------------------*/
/*                    5249S                   */
/*              Robotic ManiVEXo              */
/*                Auton Methods               */
/*                Version 0.3.0               */
/*--------------------------------------------*/
RobotControl robot;
//Create pid objects for the following methods
Pid driveSpeedPID;
Pid driveYawPID;
Pid launchAnglePID;
Pid visionHorizontalPID;
BallLauncher targetSystem;
bool driveToPoint(float endpoint, float yaw){//Drives to a specific distance in a certain direction
    driveSpeedPID.kP = 1;//Set gains for both pids
    driveSpeedPID.kI = 4.35;
    driveSpeedPID.kD = 0.153;
    driveYawPID.kP = 0;
    driveYawPID.kI = 0;
    driveYawPID.kD = 0;
    const int maxSpeed = 100;//Set motor power limit
    driveSpeedPID.setPoint = endpoint;//Set the setpoints of the pids
    driveYawPID.setPoint = yaw;
    int speed = (int)driveSpeedPID.pidCalc(mtrDriveLeft.rotation(vex::rotationUnits::deg));//Get correction values from the pids
    int turn = (int)driveYawPID.pidCalc(gyroNav.value(vex::analogUnits::range12bit));
    //If the corrections exceed the speed limit, set the speed to the limit and reset the integral
    if (speed > maxSpeed){
        speed = maxSpeed;
        driveSpeedPID.resetIntegral();
    }
    if (speed < -maxSpeed){
        speed = -maxSpeed;
        driveSpeedPID.resetIntegral();
    }
    if (turn > maxSpeed){
        turn = maxSpeed;
        driveYawPID.resetIntegral();
    }
    if (turn < -maxSpeed){
        turn = -maxSpeed;
        driveYawPID.resetIntegral();
    }
    robot.driveH(speed, turn);//Drive the chassis
    return fabs((double)endpoint) < fabs((double)mtrDriveLeft.rotation(vex::rotationUnits::deg));//Return whether the robot has reached the end
}
bool pointTurn(float yaw){//Method for turning the robot
    const int maxSpeed = 100;//Set gains and max speed
    driveYawPID.kP = 0.45;
    driveYawPID.kI = 0.7;
    driveYawPID.kD = 0.73;
    driveYawPID.setPoint = yaw;//Set the setpoint to the wanted turn
    int turnSpeed = (int)driveYawPID.pidCalc(gyroNav.value(vex::analogUnits::range12bit));//Calculate the correction
    //Limit the speed to the max speed in either direction and reset the integral
    if (turnSpeed > maxSpeed){
        turnSpeed = maxSpeed;
        driveYawPID.resetIntegral();
    }
    if (turnSpeed < -maxSpeed){
        turnSpeed = -maxSpeed;
        driveYawPID.resetIntegral();
    }
    robot.driveH(0, turnSpeed);
    return fabs((double)yaw) < fabs((double)gyroNav.value(vex::analogUnits::range12bit));//return whether the robot has reached the target
    
}
double setLauncherToAngle(double angle){//Sets the launcher to a specific angle based on the accelerometer
    const int maxSpeed = 30;//Set gains and max speed
    launchAnglePID.kP = 3;
    launchAnglePID.kI = 0;
    launchAnglePID.kD = 1.7;
    launchAnglePID.setPoint = angle;//Set the setpoint
    int speed = (int)launchAnglePID.pidCalc((float)getAccelTiltAngle());//Calculate correction
    //Limit the speed to the max speed in either direction
    if (speed > maxSpeed){
        speed = maxSpeed;
        launchAnglePID.resetIntegral();
    }
    if (speed < -maxSpeed){
        speed = -maxSpeed;
        launchAnglePID.resetIntegral();
    }
    robot.launchAnglePower(speed);//Power the launcher angle
    return fabs(angle - getAccelTiltAngle());//Return the current error
}
void horizontalAlignFlag(int position){//Aligns the launcher horizontally
    const int maxSpeed = 30;//Set gains and max speed
    visionHorizontalPID.kP = 2;
    visionHorizontalPID.kI = 0;
    visionHorizontalPID.kD = 1.7;
    visionHorizontalPID.setPoint = 140;//Set setpoint
    int speed = -(int)visionHorizontalPID.pidCalc((float)position);//Calculate correction
    //Limit speed in both directions
    if (speed > maxSpeed){
        speed = maxSpeed;
        visionHorizontalPID.resetIntegral();
    }
    if (speed < -maxSpeed){
        speed = -maxSpeed;
        visionHorizontalPID.resetIntegral();
    }
    robot.driveH(0, speed);//Power chassis
}
