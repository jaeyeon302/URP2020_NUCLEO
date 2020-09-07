#include <mbed.h>
#include "def_pins.h"
#include "step_listener.h"
#include "stall_load_detector.h"
#include "Flash_handler.h"

static BufferedSerial pc(USBTX, USBRX);

//Timer t should be defined in main.cpp
//AccelStepper call Timer t as extern variable
Timer t;
AnalogIn currentPin(current_sense);

AccelStepper stepper1(AccelStepper::DRIVER,internal_step,internal_dir);

InterruptIn step(external_step);
InterruptIn dir(external_dir);
InterruptIn ms1(external_ms1);
InterruptIn ms2(external_ms2);
InterruptIn ms3(external_ms3);

DigitalOut stepIn(internal_step);
DigitalOut dirIn(internal_dir);
DigitalOut ms1In(internal_ms1);
DigitalOut ms2In(internal_ms2);
DigitalOut ms3In(internal_ms1);

DigitalOut stall_(stall);
DigitalOut force_dir_(force_dir);
AnalogOut force_mag_(force_mag);


int main() {

  StepListener driver( &step, &dir, &ms1, &ms2, &ms3, &stepIn, &dirIn, &ms1In, &ms2In, &ms3In); 
  
  t.start();// must start timer in main
  pc.set_baud(9600);
  pc.set_format(
      /* bits */ 8,
      /* parity */ BufferedSerial::None,
      /* stop bit */ 1
  );
  
  int goal = 2400;
  stepper1.moveTo(goal);
  stepper1.setAcceleration(2000);
  stepper1.setMinPulseWidth(20);
  stepper1.setMaxSpeed(800);
  stepper1.setSpeed(400);

  Flash_handler Flash_handler;//Flash_memory handler
  Ammeter ammeter(&currentPin);
  StallLoadDetector detector(&ammeter, &stepper1);
  
  /*
  //if(ms3 ==1){
    detector.measureMotorMeanCharacteristics();
    Flash_handler.Flash_erase();
    Flash_handler.Flash_write(detector.currentValues,NUM_OF_CURRENT_SAMPLE *sizeof(double));
  //}
  */
  
  Flash_handler.Flash_read(detector.currentValues,NUM_OF_CURRENT_SAMPLE *sizeof(double));
  //driver.readyToListen();

  //Get_Linear_Regression2(stepper1);
  // put your setup code here, to run once:
  //double speed=200;
  int speed;
  unsigned int last_time = std::chrono::duration_cast<chrono::milliseconds>(t.elapsed_time()).count();
  while(1){

  if(stepper1.speed()<0) speed =-stepper1.speed();
  else speed= stepper1.speed();

  double mag = detector.AnalogOutForce(speed, &force_mag_, &force_dir_);
  //printf("%d",(int)(mag*1000));
  if(mag > 0 && std::chrono::duration_cast<chrono::milliseconds>(t.elapsed_time()).count()- last_time >30){
    //printf("yes");
    goal = -goal;
    stepper1.moveTo(goal);
    last_time = std::chrono::duration_cast<chrono::milliseconds>(t.elapsed_time()).count();
  }
  stepper1.run();
  //printf("%ld\n",(long)(detector.getLoadCurrent(&driver)));
  //printf("%d\n",(int)(detector.calculateCurrentFromSpeed(&driver))); 
  //printf("%d\n",(int)(detector.gettotalCurrent(&driver)));
  //printf("%d\n",(int)(driver.getCurrentSpeed()));  
  //printf("%d\n",(int)(driver.returnSpeed()));
  // put your main code here, to run repeatedly:
  }
    return 0;
}