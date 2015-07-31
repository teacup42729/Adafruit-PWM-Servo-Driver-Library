#include <PWMDriver.h>
#include <Wire.h>

// Set to true to print some debug messages, or false to disable them.
#define ENABLE_DEBUG_OUTPUT false

PWMDriver::PWMDriver(uint8_t addr)
{
  _i2caddr = addr;
}

void PWMDriver::begin(void)
{
 Wire.begin();
 reset();
}


void PWMDriver::reset(void)
{
 write8(PCA9685_MODE1, 0x0);
}

void PWMDriver::setPWMFreq(float freq)
{
  //Serial.print("Attempting to set freq ");
  //Serial.println(freq);
  freq *= 0.9;  // Correct for overshoot in the frequency setting (see issue #11).
  float prescaleval = 25000000;
  prescaleval /= 4096;
  prescaleval /= freq;
  prescaleval -= 1;
  if (ENABLE_DEBUG_OUTPUT)
  {
    Serial.print("Estimated pre-scale: "); Serial.println(prescaleval);
  }
  uint8_t prescale = floor(prescaleval + 0.5);
  if (ENABLE_DEBUG_OUTPUT)
  {
    Serial.print("Final pre-scale: "); Serial.println(prescale);
  }
  
  uint8_t oldmode = read8(PCA9685_MODE1);
  uint8_t newmode = (oldmode&0x7F) | 0x10; // sleep
  write8(PCA9685_MODE1, newmode); // go to sleep
  write8(PCA9685_PRESCALE, prescale); // set the prescaler
  write8(PCA9685_MODE1, oldmode);
  delay(5);
  write8(PCA9685_MODE1, oldmode | 0xa1);  //  This sets the MODE1 register to turn on auto increment.
                                          // This is why the beginTransmission below was not working.
  //  Serial.print("Mode now 0x"); Serial.println(read8(PCA9685_MODE1), HEX);
}

void PWMDriver::setPWM(uint8_t num, uint16_t on, uint16_t off)
{
  //Serial.print("Setting PWM "); Serial.print(num); Serial.print(": "); Serial.print(on); Serial.print("->"); Serial.println(off);

  Wire.beginTransmission(_i2caddr);
  Wire.write(LED0_ON_L + 4 * num);
  Wire.write(on);
  Wire.write(on >> 8);
  Wire.write(off);
  Wire.write(off >> 8);
  Wire.endTransmission();
}

// Sets pin without having to deal with on/off tick placement and properly handles
// a zero value as completely off.  Optional invert parameter supports inverting
// the pulse for sinking to ground.  Val should be a value from 0 to 4095 inclusive.
void PWMDriver::setPin(uint8_t num, uint16_t val, bool invert)
{
  // Clamp value between 0 and 4095 inclusive.
  val = min(val, 4095);
  if (invert)
  {
    if (val == 0)
    {
      // Special value for signal fully on.
      setPWM(num, 4096, 0);
    }
    else if (val == 4095)
    {
      // Special value for signal fully off.
      setPWM(num, 0, 4096);
    }
    else
    {
      setPWM(num, 0, 4095 - val);
    }
  }
  else
  {
    if (val == 4095)
    {
      // Special value for signal fully on.
      setPWM(num, 4096, 0);
    }
    else if (val == 0)
    {
      // Special value for signal fully off.
      setPWM(num, 0, 4096);
    }
    else
    {
      setPWM(num, 0, val);
    }
  }
}

uint8_t PWMDriver::read8(uint8_t addr)
{
  Wire.beginTransmission(_i2caddr);
  Wire.write(addr);
  Wire.endTransmission();

  Wire.requestFrom((uint8_t)_i2caddr, (uint8_t)1);
  return Wire.read();
}

void PWMDriver::write8(uint8_t addr, uint8_t d)
{
  Wire.beginTransmission(_i2caddr);
  Wire.write(addr);
  Wire.write(d);
  Wire.endTransmission();
}
