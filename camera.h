/*
* Rochester Institute of Technology
* Department of Computer Engineering
* CMPE 460  Interfacing Digital Electronics
* Fall 2018
* Authors: Michael Baumgarten & Matthew Toro
*
* Filename: camera.h
*/
#ifndef CAMERA_H_
#define CAMERA_H_

#include <stdint.h>

// Default System clock value
// period = 1/20485760  = 4.8814395e-8
#define SYSTEM_CLOCK (20485760u)
#define INTEGRATION_TIME (0.0075f)
#define CAM_CLK 	( 1 << 9)
#define CAM_SI		( 1 << 23)

/* 
 * Initializes all of the peripherals needed to drive the camera
 */
void initCamera(void);

/*
 * Fetches the latest data from the camera
 */
void getLine(uint16_t* data);


#endif
