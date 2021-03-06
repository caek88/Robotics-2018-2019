/*--------------------------------------------*/
/*                    5249S                   */
/*              Robotic ManiVEXo              */
/*                    Main                    */
/*                Version 0.3.0               */
/*--------------------------------------------*/
/*      Place files in following order:       */
/*                    Main                    */
/*                   Methods                  */
/*                Ball Launcher               */
/*                Auton Methods               */
/*                Driver Auton                */
/*--------------------------------------------*/
#include "robot-config.h"//Includes config file
#include <cmath> //Includes math operations Ex: pow, sin, sqrt
    
static int mode = -1;//Mode for the robot to operate in
static bool colorRed = true;//True if on red alliance, false if on blue
void auton(int);//Declares Auton and driver functions, initialized in Driver Auton file
void driver();
static bool warning[10][2];//Array of warning statuses for warning the driver of problems with the robot
double getAccelTiltAngle(){//Method for getting the tilt angle of the ball launcher using the accelerometer
    double calibrationParam[4][3] = {{-328141E-10,361739E-8,736303E-10},{398013E-10,-469610E-10,-372480E-8},{-350956E-8,-130104E-10,-459309E-10},{7.20023,-7.29033,7.57130}};
    //12 parameter calibration matrix
    int X = accelLauncherX.value(vex::analogUnits::range12bit);//Gets analog values from all three axes
    int Y = accelLauncherY.value(vex::analogUnits::range12bit);
    int Z = accelLauncherZ.value(vex::analogUnits::range12bit);
    double measuredValues[4] = {(double)X,(double)Y,(double)Z,1};//Sets values in matrix
    double trueValues[3];//Declares true accelerometer values matrix
    //Multiplies the parameter matrix by the analog values
    for (int i = 0;i < 3; i++){
        double dotSum = 0;
        for (int j = 0; j < 4; j++){
            dotSum += measuredValues[j]*calibrationParam[j][i];
        }
        trueValues[i] = dotSum;
    }
    if (pow(trueValues[1],2)+pow(trueValues[2],2) == 0){//Prevents a divide by zero error in the next calculation
        return 90;
    }
    return (180/3.141592)*atan((trueValues[0])/(sqrt(pow(trueValues[1],2)+pow(trueValues[2],2))));//Calculates tilt angle
}
void runDiagnostics(){//Method for displaying any problems with the robot
    char warningText[10][6] = {"BatL ","BatH ","MdlH ","MdrH ","MllH ","MlrH","","","",""};//array of warning texts
    for (int i = 0; i < 10; i++){ //store the previous state of each error to check for a change
        warning[i][1] = warning[i][0];
    }
    warning[0][0] = robotMain.Battery.capacity() < 25;//Battery capacity < 25%
    warning[1][0] = robotMain.Battery.temperature() > 80; //Battery temperature > 80%
    warning[2][0] = mtrDriveLeft.temperature(vex::percentUnits::pct) > 45; //Left Drive Motor Temp >45%
    warning[3][0] = mtrDriveRight.temperature(vex::percentUnits::pct) >45; //Right Drive Motor Temp >45%
    warning[4][0] = mtrLiftLeft.temperature(vex::percentUnits::pct) > 45; //Left Lift Motor Temp >45%
    warning[5][0] = mtrLiftRight.temperature(vex::percentUnits::pct) > 45; //Right Lift Motor Temp >45%
    warning[6][0] = mtrClaw.temperature(vex::percentUnits::pct) > 45; //Claw Motor Temp >45%
    warning[7][0] = mtrLauncherAngle.temperature(vex::percentUnits::pct) >45; //Launcher Angle Motor Temp >45%
    warning[8][0] = mtrLauncherFire.temperature(vex::percentUnits::pct) > 45; //Launcher Fire Motor Temp >45%
    warning[9][0] = false;
    
    //Checks if a value has changed and needs to be updated to comply with controller screen's slow update rate
    bool update = false;
    for (int i = 0; i < 10; i++){
        if ((warning[i][0] && !warning[i][1]) || (warning[i][1] && !warning[i][0])){ //Update the display if any of the warnings have changed
            update = true; 
            break;
        }
    }
    if (update) {//Display all warnings
        ctrPrimary.Screen.clearLine(2);
        ctrPrimary.Screen.setCursor(2,0);
        int cursor = 0;
        for (int i = 0; i < 10; i++){
            if (warning[i][0]){
                ctrPrimary.Screen.print("%s ", warningText[i]);//Display all warning text in succession
                cursor += 6;
                ctrPrimary.Screen.setCursor(2,cursor);
            }
        }
    }
}
void clearDiagnostics(){//Clears the diagnostic warnings
    for (int i = 0; i < 10;i++){
        warning[i][1] = false;
    }
}
class GyroSettings {//Class used to set gyros to specific values, as they can't be changed in the current API
    private:
        int gyroBias = 0;
        int reverse = 1;
    public:
        void setValues(int trueValue, int currentValue, bool rev){//Sets proper values
            reverse = rev?-1:1;
            gyroBias = currentValue - reverse * trueValue;
        }
        int value(int currentValue){//returns true value with wanted bias value
            return reverse * (currentValue - gyroBias);
        }
};
void wait(int time){//waits a number of milliseconds
    vex::task::sleep(time);
}
GyroSettings gyroNavSet;
void calibrateGyros(){//Calibrates gyros
    ctrPrimary.Screen.clearScreen();//Display calibration message on the controller
    ctrPrimary.Screen.setCursor(1,0);
    ctrPrimary.Screen.print("Gyros Calibrating");
    ctrPrimary.Screen.newLine();
    ctrPrimary.Screen.print("Do Not Touch Robot!");
    ctrPrimary.Screen.newLine();
    ctrPrimary.Screen.print("(B) Bypass");
    gyroNav.startCalibration();
    int timer = 0;
    while(gyroNav.isCalibrating() || timer < 3000){//waits for both gyros to finish or three seconds have passed
        if (ctrPrimary.ButtonB.pressing()){
            break;//allows bypass
        }
        wait(20);
        timer += 20;
    }
    while(ctrPrimary.ButtonB.pressing()){wait(20);}
    gyroNavSet.setValues(0, gyroNav.value(vex::rotationUnits::deg), false);
    ctrPrimary.Screen.clearScreen();
}
void stopAllMotors(){//stops all motors on the robot
    mtrDriveLeft.stop(vex::brakeType::coast);
    mtrDriveRight.stop(vex::brakeType::coast);
    mtrLiftLeft.stop(vex::brakeType::coast);
    mtrLiftRight.stop(vex::brakeType::coast);
    mtrClaw.stop(vex::brakeType::coast);
    mtrLauncherAngle.stop(vex::brakeType::coast);
    mtrLauncherFire.stop(vex::brakeType::coast);
	  mtrBallLift.stop(vex::brakeType::coast);
    clearDiagnostics();
}
bool isField(){//Method for checking if either field control device is connected
    return compControl.isCompetitionSwitch() || compControl.isFieldControl();
}
class DisplaySelection {//Class created to hold and change the values needed to move the display up and down
        private: 
            int maxLines = 3;//Number of controller display lines
            int topLine = 0;//Choice on the top line of the controller
            int position = 0;//Position of the arrow
            unsigned int max = 0;//Max number of choices
            bool selectionMade = false;
    
            int getCurrent(){//returns the option the arrow is on
                return topLine + position;
            }
            void moveDown(){//Moves display down
                if (getCurrent() != max - 1){//If the arrow is not at the last choice, move everything down
                    if (position == maxLines - 1){//Move the options down if the arrow is at the bottom
                        topLine ++;
                    } else {//Move the arrow down otherwise
                        position ++;
                    }
                } else {//If the arrow is at the last choice, return to the top
                    topLine = 0;
                    position = 0;
                }
            }
            void moveUp(){//Moves Display up
                if (getCurrent() != 0){//If the arrow is at not at the first selection, move everything up
                    if (position == 0){//move the options up if the arrow is at the top
                        topLine --;
                    } else {//Otherwise move the arrow up
                        position --;
                    }
                } else {//If the arrow is at the first choice, go to the bottom
                    position = maxLines - 1;
                    topLine = max - maxLines;
                }
            }
        public:
            char text[8][32];//storage for text options
            DisplaySelection(unsigned int maxOptions){//Constructor
                if (maxOptions < maxLines){//Sets the maxlines to the option number in case there are less options that usable lines
                    maxLines = maxOptions;
                }
                max = maxOptions;//Set the max number of options
            }
            int select(){//returns the chosen selection
                while(true){//repeat update until a selection is chosen
                    if(ctrPrimary.ButtonA.pressing()){//Return the current number if a selection has been made
                        while(ctrPrimary.ButtonA.pressing() || ctrPrimary.ButtonUp.pressing() || ctrPrimary.ButtonDown.pressing()){wait(20);}
                        return getCurrent();
                    }
                    if(ctrPrimary.ButtonUp.pressing()){//Move up if up button is pressed
                        moveUp();
                    } 
                    if(ctrPrimary.ButtonDown.pressing()){//Move down if down button is pressed
                        moveDown();
                    }
                    ctrPrimary.Screen.clearScreen();//clears the screen
                    for (int i=0; i < maxLines; i++){//Displays lines of text based on instance variables
                        ctrPrimary.Screen.setCursor(i+1,3);//
                        ctrPrimary.Screen.print("%s", text[i + topLine]);
                    }
                    ctrPrimary.Screen.setCursor(position+1,0);
                    ctrPrimary.Screen.print("->");//Print the arrow at the position
                    while(ctrPrimary.ButtonA.pressing() || ctrPrimary.ButtonUp.pressing() || ctrPrimary.ButtonDown.pressing()){wait(20);}//wait for all buttons to be released
                    while(!(ctrPrimary.ButtonA.pressing() || ctrPrimary.ButtonUp.pressing() || ctrPrimary.ButtonDown.pressing())){//Waits for a button to be pressed to prevent controller lag
                        if (isField()){//If the robot is connected to the field, display message to remove the cable
                            ctrPrimary.Screen.clearScreen();
                            ctrPrimary.Screen.setCursor(1,0);
                            ctrPrimary.Screen.print("Remove Field Cable");
                            while (isField()){//Wait for field cable to be removed
                                wait(20);
                            }
                            break;//Break the loop to redisplay the options
                        }
                        wait(20);
                    }
                }
            }
};
bool confirmAuton(){//Confirms it is allowed to run auton
    if (mode == 0 || mode == 1){//If in field control or skills mode, the competition control will be checked
        if (compControl.isAutonomous() && compControl.isEnabled() && isField()){//return true if auton is on and the robot is enabled
            return true;
        }
        return false;//otherwise return false
    }
    if (mode == 2 && !isField() && !ctrPrimary.ButtonB.pressing()){//if in auton testing mode, always allow
        return true;
    }
    return false;//return false otherwise
}
bool confirmDriver(){//Confirms it is allowed to run driver control
    if (mode == 0 || mode == 1){//If in field control or skills mode, the competition control will be checked
        if (compControl.isDriverControl() && compControl.isEnabled() && isField()){//return true if driver is on and the robot is enabled
            return true;
        }
        return false;//otherwise return false
    }
    if (mode == 3 && !isField() && !ctrPrimary.ButtonB.pressing()){//if in driver mode, always allow
        return true;
    }
    return false;//return false otherwise
}
int selectAutonomous(){//method for selecting autons
    DisplaySelection selectAuton = DisplaySelection(4);//create display selection object
    strcpy(selectAuton.text[0], "Exit");//place names of autons in array
    strcpy(selectAuton.text[1], "Auton1");
    strcpy(selectAuton.text[2], "Auton2");
    strcpy(selectAuton.text[3], "Auton3");
    strcpy(selectAuton.text[4], "");
    return selectAuton.select();
}
void colorSelect(){//method for selecting field color
    DisplaySelection selectColor = DisplaySelection(2);//create display object
    strcpy(selectColor.text[0], "Red");//set array values to colors
    strcpy(selectColor.text[1], "Blue");
    strcpy(selectColor.text[2], "");
    strcpy(selectColor.text[3], "");
    strcpy(selectColor.text[4], "");
    colorRed = (selectColor.select() == 0);
}
int main() {
    robotMain.Screen.setFont(vex::fontType::mono40);
    ctrPrimary.Screen.clearScreen();
    ctrPrimary.Screen.setCursor(1,0);
    vex::digital_out led = vex::digital_out(robotMain.ThreeWirePort.H);
    led.set(false);
    while(true){
        DisplaySelection selectMode = DisplaySelection(5); //Create Display object
        strcpy(selectMode.text[0], "Field Control");//set values in array to options
        strcpy(selectMode.text[1], "Skills Control");
        strcpy(selectMode.text[2], "Auton Testing");
        strcpy(selectMode.text[3], "Driver Control");
        strcpy(selectMode.text[4], "Exit");//for closing the program
        mode = selectMode.select();
        if(mode == 0){//Field control was selected
            calibrateGyros();//Calibrate gyro sensors
            colorSelect();//select team color
            int autonMode = selectAutonomous();//select auton to run
            while(true){//loop for competition
                if (!isField()){//Waits for the user to connect to the field after selections are made
                    ctrPrimary.Screen.clearScreen();
                    ctrPrimary.Screen.setCursor(1,0);
                    ctrPrimary.Screen.print("Connect to Field");
                    ctrPrimary.Screen.newLine();
                    ctrPrimary.Screen.print("(B) Close");
                    while(!ctrPrimary.ButtonB.pressing() && !isField()){wait(20);}
                    if(ctrPrimary.ButtonB.pressing()){
                        break;
                    }
                }
                while(!compControl.isEnabled()){//Wait while the robot is disabled
                    ctrPrimary.Screen.setCursor(1,0);
                    ctrPrimary.Screen.clearLine();
                    ctrPrimary.Screen.print("FC-Disabled");
                    while(!compControl.isEnabled()){wait(20);}
                }
                
                if(compControl.isEnabled() && compControl.isAutonomous()){//runs auton when enabled and autonomous
                        auton(autonMode);
                        while(compControl.isEnabled() && compControl.isAutonomous() && isField()){wait(20);}//Waits for auton to end (50 Hertz)
                }
                if(compControl.isEnabled() && compControl.isDriverControl()){//runs driver control when enabled and driver control
                    driver();
                    while(compControl.isEnabled() && compControl.isDriverControl() && isField()){wait(20);}//Waits for driver control to end (50 Hertz)
                }
                stopAllMotors();
            }
            stopAllMotors();
        }
        if(mode == 1){//Skills mode- same as field control, except no color selection
	    calibrateGyros();
            colorRed = true;
            int autonMode = selectAutonomous();
            ctrPrimary.Screen.clearScreen();
            while(true){
                if (!isField()){
                    ctrPrimary.Screen.clearScreen();
                    ctrPrimary.Screen.setCursor(1,0);
                    ctrPrimary.Screen.print("Connect to Field");
                    ctrPrimary.Screen.newLine();
                    ctrPrimary.Screen.print("(B) Close");
                    while(!ctrPrimary.ButtonB.pressing() && !isField()){wait(20);}
                    if(ctrPrimary.ButtonB.pressing()){
                        break;
                    }
                }
                while(!compControl.isEnabled()){//Waits while robot is disabled
                    ctrPrimary.Screen.setCursor(1,0);
                    ctrPrimary.Screen.clearLine();
                    ctrPrimary.Screen.print("SK-Disabled");
                    while(!compControl.isEnabled()){wait(20);}
                }
                if(compControl.isEnabled() && compControl.isAutonomous()){//runs auton when enabled and autonomous
                        auton(autonMode);
                        while(compControl.isEnabled() && compControl.isAutonomous() && isField()){wait(20);}//Waits for auton to end (50 Hertz)
                }
                if(compControl.isEnabled() && compControl.isDriverControl()){//runs driver control when driver control
                    driver();
                    while(compControl.isEnabled() && compControl.isDriverControl() && isField()){wait(20);}//Waits for driver control to end (50 Hertz)
                }
                stopAllMotors();
            }
            stopAllMotors();
        }
        if(mode == 2){//Auton testing mode
            while (true){
                while(true){
                    ctrPrimary.Screen.clearScreen();
                    ctrPrimary.Screen.setCursor(1,0);
                    ctrPrimary.Screen.print("Setup Robot");
                    ctrPrimary.Screen.newLine();
                    ctrPrimary.Screen.print("(A) Done");//Waits until robot is placed properly for auton
                    while (!ctrPrimary.ButtonA.pressing()){wait(20);}
                    if (ctrPrimary.ButtonA.pressing()){
                        break;
                    }
                    wait(50); //Update at 20 hertz
                }
                while(ctrPrimary.ButtonA.pressing()){wait(20);}
                calibrateGyros();
                int selection = selectAutonomous();
                if(selection == 0){//exits if auton 1 is selected
                    break;
                }
                colorSelect();//select color
                auton(selection);//run selected auton
                stopAllMotors();
                while(ctrPrimary.ButtonB.pressing()){wait(20);}//wait for exit button to be released
            }
        }
        if(mode == 3){//Runs driver control
            calibrateGyros();
            driver();
            stopAllMotors();
            while(ctrPrimary.ButtonB.pressing()){wait(20);}//wait for exit button to be released
        }
        if(mode == 4){//Exits program
            return 0;
        }
    }
}
