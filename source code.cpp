#include<sys_config.h>
#include<wireless.h>
#include <stdio.h>
#include "utilities.h"
#include "io.hpp"
#include <math.h>
#include <ctype.h>
#include <cstring>
#include<gpio.hpp>

void calibrateAccel();
bool checkMotion();

int tolerance=10; // Sensitivity of the Alarm
//bool calibrated=false; // When accelerometer is calibrated - changes to true
//bool moveDetected=false; // When motion is detected - changes to true
bool calibrated_temp =false;
bool moveDetected_temp =false;

int xMin; //Minimum x Value
int xMax; //Maximum x Value
int xVal; //Current x Value

int yMin; //Minimum y Value
int yMax; //Maximum y Value
int yVal; //Current y Value

int zMin; //Minimum z Value
int zMax; //Maximum z Value
int zVal; //Current z Value

int main(void)

{

	//alarmcode
	int temperature_f = TS.getFarenheit();
	LD.setNumber(temperature_f);
	typedef enum { calibrated, moveDetected,set_alarm_on, set_alarm_off } motion_var ;
					motion_var current_state;

					GPIO myPin(P2_5);   // Control P1.19
					 myPin.setAsOutput(); // Use the pin as output pin
					char  addr ;
					char hops;
					bool alarm_off ;

					mesh_packet_t pkt;
					int count =0; //glob var
					while(1){
						delay_ms(100);
						bool sw1 =SW.getSwitch(1);
							if(sw1){
								current_state =calibrated ;
							}
						bool sw2 =false;
						delay_ms(100);


									// background task takes care of the mest network

														/* Try to get a packet destined for us with 100ms timeout*/

														    if (wireless_get_rx_pkt(&pkt, 100)) {
														    	mesh_deform_pkt(&pkt, 1, &sw2, sizeof(sw2) );
														    	printf("data %i\n\n",pkt.data[0]);
														    	printf("switch 2 value received as %i\n", sw2);

														    }
						if (sw2==1){
							count =count +1;

						}
						if(count>=1){
							current_state = set_alarm_off;
						}
						switch(current_state)
						{
						case calibrated:
							printf("i m in calibrated");

							calibrateAccel();
							LE.on(1);
							delay_ms(1000);
							printValues();
							calibrated_temp = true;
							current_state = moveDetected;
							LE.off(1);
							break;

						case moveDetected :
							printf("i m in move detecttedd");

							if(calibrated_temp){
								if(checkMotion()){
									moveDetected_temp = true;
									current_state = set_alarm_on;

								}
							}//ends moveDetected - if statements bracket
							break;

						case set_alarm_on :
							printf("i m in alarm on");

							LE.on(2);
							LE.on(4);
							delay_ms(100);
							LE.off(2);
							LE.off(4);
							delay_ms(100);
							 myPin.setHigh();     // Pin will now be at 3.3v
							delay_ms(100);
							break;

						case set_alarm_off:
							mesh_packet_t pkt1;
							addr = 106;
							hops = 10;
							alarm_off =true;
							printf("alarm off.... you got it...hacker");
							LE.on(3);
							delay_ms(100);
							LE.off(3);
							delay_ms(100);
							myPin.setLow();
							mesh_form_pkt(&pkt1, addr, mesh_pkt_ack, hops,
														                                  1,                     /* 2 Pairs below */
														                                  &alarm_off, sizeof(alarm_off)   /* Pair 1 */
														                                  );  /* Pair 2 */
							 mesh_send_formed_pkt(&pkt1);

							break;

						default:
							printf("check your code fool");
						}

					}




					return 0;


				}

// Function used to calibrate the Accelerometer
void calibrateAccel(){
 // reset alarm
  moveDetected_temp =false;

 //initialise x,y,z variables
  xVal = AS.getX();
  xMin = xVal;
  xMax = xVal;

  yVal = AS.getY();
  yMin = yVal;
  yMax = yVal;

  zVal = AS.getZ();
  zMin = zVal;
  zMax = zVal;

 // Calibration sequence initialisation sound - 3 seconds before calibration begins
 //buzz(3,1000);

 //calibrate the Accelerometer (should take about 0.5 seconds)
 for (int i=0; i<50; i++){
 // Calibrate X Values
 xVal = AS.getX();
 if(xVal>xMax){
 xMax=xVal;
 }else if (xVal < xMin){
 xMin=xVal;
 }

 // Calibrate Y Values
 yVal = AS.getY();
 if(yVal>yMax){
 yMax=yVal;
 }else if (yVal < yMin){
 yMin=yVal;
 }

 // Calibrate Z Values
 zVal = AS.getZ();
 if(zVal>zMax){
 zMax=zVal;
 }else if (zVal < zMin){
 zMin=zVal;
 }

 //Delay 10msec between readings
 delay_ms(100);
 }

 //End of calibration sequence sound. ARMED.
 //buzz(3,40);
// printValues(); //Only useful when connected to computer- using serial monitor.
 calibrated_temp =true;

}



//Function used to detect motion. Tolerance variable adjusts the sensitivity of movement detected.
bool checkMotion(){
 bool tempB=false;

 xVal = AS.getX();
 yVal = AS.getY();
 zVal = AS.getZ();

 if(xVal >(xMax+tolerance)||xVal < (xMin-tolerance)){
 tempB=true;
 printf("X Failed = ");
 printf("%i\n\n",xVal);
 }

 if(yVal >(yMax+tolerance)||yVal < (yMin-tolerance)){
 tempB=true;
 printf("Y Failed = ");
  printf("%i\n\n",yVal);
 }

 if(zVal >(zMax+tolerance)||zVal < (zMin-tolerance)){
 tempB=true;
 printf("Z Failed = ");
  printf("%i\n\n",zVal);
 }

 return tempB;
}
