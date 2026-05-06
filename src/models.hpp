#include <string>

struct Location
{
  double lat;
  double lon;
};

// Call

enum class CallPriority
{
  Alpha,
  Bravo,
  Charlie,
  Delta,
  Echo
};

struct Call
{
  std::string id;
  CallPriority priority;
  Location location;
};

// Ambulance

enum class AmbulanceStatus
{
  Available,
  Transporting,
  OutOfService
};

enum class AmbulanceType
{
  ALS,
  BLS
};

struct Ambulance
{
  std::string id;
  AmbulanceStatus ambulance_status;
  AmbulanceType ambulance_type;
  Location location;
};

// Hospital

struct Hospital
{
  std::string id;
  int capacity;
  Location location;
};

// Dispatch

struct Dispatch
{
  std::string call_id;
  std::string ambulance_id;
  std::string hospital_id;
};
