#ifndef PWM_H_
#define PWM_H_

void SetDutyCycle(unsigned int leftDuty, unsigned int rightDuty, unsigned int Frequency, int dir);
void InitPWM();
void PWM_ISR();
void SetServoDutyCycle(float DutyCycle, unsigned int Frequency, int dir);
void InitServoPWM();

#endif /* PWM_H_ */
