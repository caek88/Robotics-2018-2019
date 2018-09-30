/*--------------------------------------------*/
/*                    5249S                   */
/*             Communist ManiVEXo             */
/*                    Main                    */
/*                Version 0.1.0               */
/*--------------------------------------------*/
#include "robot-config.h"
#include <cmath>
    
static int mode = -1;
static bool colorRed = true;
void autonomous(int);
void driverControl();
class GyroSettings {//Class used to set gyros to specific values, as they can't be changed in the program
    private:
        int gyroBias = 0;
        int reverse = 1;
    public:
        void setValues(int trueValue, int currentValue, bool rev){
            reverse = rev?-1:1;
            gyroBias = currentValue - reverse * trueValue;
        }
        int value(int currentValue){
            return reverse * (currentValue - gyroBias);
        }
};
void wait(int time){
    vex::task::sleep(time);
}
GyroSettings gyroLauncherSet;
GyroSettings gyroNavSet;
void calibrateGyros(){
    ctrPrimary.Screen.clearScreen();
    ctrPrimary.Screen.setCursor(0,0);
    ctrPrimary.Screen.print("Gyros Calibrating");
    ctrPrimary.Screen.newLine();
    ctrPrimary.Screen.print("Do Not Touch Robot");
    ctrPrimary.Screen.newLine();
    ctrPrimary.Screen.print("(B) Bypass");
    gyroNav.startCalibration();
    gyroLauncher.startCalibration();
    while(gyroNav.isCalibrating() || gyroLauncher.isCalibrating()){
        if (ctrPrimary.ButtonB.pressing()){
            break;
        }
        wait(20);
    }
    while(ctrPrimary.ButtonB.pressing()){wait(20);}
    gyroNavSet.setValues(0, gyroNav.value(vex::rotationUnits::deg), false);
    gyroLauncherSet.setValues(0, gyroLauncher.value(vex::rotationUnits::deg), false);
    
    
}
void stopAllMotors(){

}
class DisplaySelection {//Class created to hold and change the values needed to move the display up and down
    private:
        int maxLines = 3;
        int current = 0;
        int topLine = 0;
        bool selectionMade = false;
        
        unsigned int max = 0;
        int getPosition(){
            return current - topLine;
        }

        void moveDown(){
            if (current != max - 1){
                if (current == topLine + maxLines - 1){
                    topLine ++;
                }
                current ++;
            }
        }
        void moveUp(){
            if (current != 0){
                if (current == topLine){
                    topLine --;
                }
                current --;
            }
        }
    public:
        char text[8][32];
        DisplaySelection(unsigned int maxOptions){
            max = maxOptions;
        }
        
        int update(bool select, bool up, bool down){
            if(select){
                return current;
            }
            if(up){
                moveUp();
            } 
            if(down){
                moveDown();
            }
            ctrPrimary.Screen.clearScreen();
            for (int i=0; i == maxLines; i++){
                ctrPrimary.Screen.setCursor(i,2);
                ctrPrimary.Screen.print("%s", text[i + topLine]);
            }
            ctrPrimary.Screen.setCursor(getPosition(),0);
            ctrPrimary.Screen.print("->");
            return -1;
        }
};
class PromptClose {//Handles whether the user wants to exit at a particular screen
    private:
        bool prompt = false;//bool for whether the second confirm close screen should be shown
    public:
        int update(bool A, bool B){//A and B are the values for selecting the different options
            ctrPrimary.Screen.clearLine(1);
            ctrPrimary.Screen.clearLine(2);
            if((!prompt && B) || (prompt && !A && !B)){//Shows second screen if prompt is true or b is chosen on first screen
                prompt = true;
                ctrPrimary.Screen.setCursor(1,0);
                ctrPrimary.Screen.print("(A) Close?");
                ctrPrimary.Screen.newLine();
                ctrPrimary.Screen.print("(B) Back");
                return 0;
            }
            if((prompt && B) || (!prompt && !B)){//Shows first screen if prompt is false or if b is chosen on the second screen
                prompt = false;
                ctrPrimary.Screen.setCursor(1,0);
                ctrPrimary.Screen.print("(B) Close");
                return 0;
            }
            return 1;//returns 0 if there is no close chosen, returns 1 if close is chosen
        }
};
bool confirmAuton(){//Confirms it is allowed to run auton
    if (mode == 0 || mode == 1){//If in field control or skills mode, the competition control will be checked
        if (compControl.isAutonomous() && compControl.isEnabled()){//return true if auton is on and the robot is enabled
            return true;
        }
        return false;//otherwise return false
    }
    if (mode == 2){//if in auton testing mode, always allow
        return true;
    }
    return false;//return false otherwise
}
bool confirmDriver(){//Confirms it is allowed to run driver control
    if (mode == 0 || mode == 3){//If in field control or skills mode, the competition control will be checked
        if (compControl.isDriverControl() && compControl.isEnabled()){//return true if driver is on and the robot is enabled
            return true;
        }
        return false;//otherwise return false
    }
    if (mode == 2){//if in driver mode, always allow
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
    while(true){//repeat update until a selection is chosen
        bool select = ctrPrimary.ButtonA.pressing();
        bool up = ctrPrimary.ButtonUp.pressing();
        bool down = ctrPrimary.ButtonDown.pressing();
        int status = selectAuton.update(select, up, down);//call update function
        while(ctrPrimary.ButtonA.pressing() || ctrPrimary.ButtonUp.pressing() || ctrPrimary.ButtonDown.pressing()){wait(20);}//wait for all buttons to be released
        if (status != -1){//repeat loop until selection is made (update return something other than -1)
            return status;//return auton number
            break;
        }
        wait(20); //Update at 50 hertz
    }
}
void colorSelect(){//method for selecting field color
    DisplaySelection selectColor = DisplaySelection(2);//create display object
    strcpy(selectColor.text[0], "Red");//set array values to colors
    strcpy(selectColor.text[1], "Blue");
    while(true){//update until a selection is chosen
        bool select = ctrPrimary.ButtonA.pressing();
        bool up = ctrPrimary.ButtonUp.pressing();
        bool down = ctrPrimary.ButtonDown.pressing();
        int status = selectColor.update(select, up, down);//call update method
        while(ctrPrimary.ButtonA.pressing() || ctrPrimary.ButtonUp.pressing() || ctrPrimary.ButtonDown.pressing()){wait(20);}//wait for all buttons to be released
        if (status != -1){//repeat loop until selection is made (update return something other than -1)
            colorRed = (status == 0);//set colorRed to true if the status is zero, otherwise blue
            break;
        }
        wait(20);//update at 50 hertz
    }
}
int main(){//main control function
    while(true){//repeat until close is selected
        //Prompt mode selection
        DisplaySelection selectMode = DisplaySelection(5); //Create Display object
        strcpy(selectMode.text[0], "Field Control");//set values in array to options
        strcpy(selectMode.text[1], "Skills Control");
        strcpy(selectMode.text[2], "Auton Testing");
        strcpy(selectMode.text[3], "Driver Control");
        strcpy(selectMode.text[4], "Exit");//for closing the program
        while(true){//update screen until selection is chosen
            bool select = ctrPrimary.ButtonA.pressing();
            bool up = ctrPrimary.ButtonUp.pressing();
            bool down = ctrPrimary.ButtonDown.pressing();
            int status = selectMode.update(select, up, down);//call update method
            while(ctrPrimary.ButtonA.pressing() || ctrPrimary.ButtonUp.pressing() || ctrPrimary.ButtonDown.pressing()){wait(20);}//wait for all buttons to be released
            if (status != -1){//repeat loop until selection is made (update return something other than -1)
                mode = status;
                break;
            }
            wait(20); //Update at 50 hertz
        }
        if(mode == 0){//Field control was selected
            calibrateGyros();//Calibrate gyro sensors
            colorSelect();//select team color
            int autonMode = selectAutonomous();//select auton to run

            while(true){//loop for competition
                bool statusClose = false;
                PromptClose promptExit = PromptClose();
                while(!compControl.isEnabled()){//While disabled, user has option to close field control 
                    ctrPrimary.Screen.setCursor(0,0);
                    ctrPrimary.Screen.clearLine();
                    ctrPrimary.Screen.print("FC-Disabled");
                    bool a = ctrPrimary.ButtonA.pressing();
                    bool b = ctrPrimary.ButtonB.pressing();
                    statusClose = (promptExit.update(a, b) == 1);
                    while((ctrPrimary.ButtonA.pressing() || ctrPrimary.ButtonB.pressing()) && !compControl.isEnabled()){wait(20);}
                    if (statusClose){
                        break;
                    }
                    wait(20);
                }
                if (statusClose){
                    break;
                }
                if(compControl.isEnabled() && compControl.isAutonomous()){
                        auton(autonMode);
                        while(compControl.isEnabled() && compControl.isAutonomous()){wait(20);}//Waits for auton to end (50 Hertz)
                }
                if(compControl.isEnabled() && compControl.isDriverControl()){
                    driver();
                    while(compControl.isEnabled() && compControl.isDriverControl()){wait(20);}//Waits for driver control to end (50 Hertz)
                }
                stopAllMotors();
            }
            stopAllMotors();
        }
        if(mode == 1){
            colorRed = true;
            int autonMode = selectAutonomous();

            while(true){
                bool statusClose = false;
                PromptClose promptExit = PromptClose();
                while(!compControl.isEnabled()){
                    ctrPrimary.Screen.setCursor(0,0);
                    ctrPrimary.Screen.clearLine();
                    ctrPrimary.Screen.print("SK-Disabled");
                    bool a = ctrPrimary.ButtonA.pressing();
                    bool b = ctrPrimary.ButtonB.pressing();
                    statusClose = (promptExit.update(a, b) == 1);
                    while((ctrPrimary.ButtonA.pressing() || ctrPrimary.ButtonB.pressing()) && !compControl.isEnabled()){wait(20);}
                    if (statusClose){
                        break;
                    }
                    wait(20);
                }
                if (statusClose){
                    break;
                }
                if(compControl.isEnabled() && compControl.isAutonomous()){
                        auton(autonMode);
                        while(compControl.isEnabled() && compControl.isAutonomous()){wait(20);}//Waits for auton to end (50 Hertz)
                }
                if(compControl.isEnabled() && compControl.isDriverControl()){
                    driver();
                    while(compControl.isEnabled() && compControl.isDriverControl()){wait(20);}//Waits for driver control to end (50 Hertz)
                }
                stopAllMotors();
            }
            stopAllMotors();
        }
        if(mode == 2){
            while (true){
                while(true){
                    ctrPrimary.Screen.clearScreen();
                    ctrPrimary.Screen.setCursor(0,0);
                    ctrPrimary.Screen.print("Setup Robot");
                    ctrPrimary.Screen.newLine();
                    ctrPrimary.Screen.print("(A) Done");
                    if (ctrPrimary.ButtonA.pressing()){
                        break;
                    }
                    wait(20);
                }
                while(ctrPrimary.ButtonA.pressing()){wait(20);}
                calibrateGyros();
                int selection = selectAutonomous();
                if(selection == 0){
                    break;
                }
                colorSelect();
                auton(selection);
            }
        }
        if(mode == 3){
            calibrateGyros();
            colorSelect();
            driver();
        }
        if(mode == 4){
            return 0;
        }
    }
}
