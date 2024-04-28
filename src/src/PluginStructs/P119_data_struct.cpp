#include "../PluginStructs/P119_data_struct.h"

#ifdef USES_P119

// **************************************************************************/
// Constructor
// **************************************************************************/
P119_data_struct::P119_data_struct(uint8_t i2c_addr, bool rawData, uint8_t aSize)
  : _i2cAddress(i2c_addr), _rawData(rawData), _aSize(aSize) {
  if (_aSize == 0) { _aSize = 1; }
  _XA.resize(_aSize, 0);
  _YA.resize(_aSize, 0);
  _ZA.resize(_aSize, 0);
  _aUsed = 0;
  _aMax  = 0;
}

// **************************************************************************/
// Destructor
// **************************************************************************/
P119_data_struct::~P119_data_struct() {
  if (initialized()) {
    delete itg3205;
    itg3205 = nullptr;
  }
}

// **************************************************************************/
// Initialize sensor and read data from ITG3205
// **************************************************************************/
bool P119_data_struct::read_sensor() {
  # ifdef PLUGIN_119_DEBUG
  String log;
  # endif // if PLUGIN_119_DEBUG

  if (!initialized()) {
    init_sensor();

    # ifdef PLUGIN_119_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLogMove(LOG_LEVEL_DEBUG,
                 strformat(F("ITG3205: i2caddress: 0x%02x, initialized: %d, ID=0x%02x"),
                           _i2cAddress, initialized(), itg3205->readWhoAmI()));
    }
    # endif // if PLUGIN_119_DEBUG
  }

  if (initialized()) {
    if (_rawData) {
      itg3205->readGyroRaw();
    } else {
      itg3205->readGyro();
    }
    _XA[_aUsed] = itg3205->g.x;
    _YA[_aUsed] = itg3205->g.y;
    _ZA[_aUsed] = itg3205->g.z;

    _aUsed++;

    if ((_aMax < _aUsed) && (_aUsed < _aSize)) {
      _aMax = _aUsed;
    }

    if (_aUsed == _aSize) {
      _aUsed = 0;
    }

    # ifdef PLUGIN_119_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLogMove(LOG_LEVEL_DEBUG, strformat(F("ITG3205: %s, X: %d, Y: %d, Z: %d"),
                                            String(_rawData ? F("raw ") : F("")).c_str(),
                                            itg3205->g.x, itg3205->g.y, itg3205->g.z));
    }
    # endif // if PLUGIN_119_DEBUG
    return true;
  }
  return false;
}

// **************************************************************************/
// Average the measurements and return the results
// **************************************************************************/
bool P119_data_struct::read_data(int& X, int& Y, int& Z) {
  X = 0;
  Y = 0;
  Z = 0;

  if (initialized()) {
    for (uint8_t n = 0; n <= _aMax; ++n) {
      X += _XA[n];
      Y += _YA[n];
      Z += _ZA[n];
    }

    X /= _aMax; // Average available measurements
    Y /= _aMax;
    Z /= _aMax;

    # ifdef PLUGIN_119_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLogMove(LOG_LEVEL_DEBUG, strformat(F("ITG3205: averages, X: %d, Y: %d, Z: %d"), X, Y, Z));
    }
    # endif // if PLUGIN_119_DEBUG
  }
  return initialized();
}

// **************************************************************************/
// Initialize ITG3205
// **************************************************************************/
bool P119_data_struct::init_sensor() {
  itg3205 = new (std::nothrow) ITG3205(_i2cAddress);

  if (initialized()) {
    addLog(LOG_LEVEL_INFO, F("ITG3205: Initializing Gyro..."));
    itg3205->initGyro();
    addLog(LOG_LEVEL_INFO, F("ITG3205: Calibrating Gyro..."));
    itg3205->calibrate();
    addLog(LOG_LEVEL_INFO, F("ITG3205: Calibration done."));
  } else {
    addLog(LOG_LEVEL_ERROR, F("ITG3205: Initialization of Gyro failed."));
    return false;
  }

  # ifdef PLUGIN_119_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLogMove(LOG_LEVEL_DEBUG, strformat(F("ITG3205: Address: 0x%02x"), _i2cAddress));
  }
  # endif // ifdef PLUGIN_119_DEBUG

  return true;
}

#endif // ifdef USES_P119
