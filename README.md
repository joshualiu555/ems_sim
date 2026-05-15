# ems_sim

## Features
- Allow for manual input of hospitals, ambulances, roads, calls AND random generation
- Allow ambulance to take another call from anywhere by keeping track of live locations in database - Make sure ambulance defaults by returning to station, though
- Don't automatically fail a call if there is no available ambulance - Give them time and only fail if a certain amount of time has elapsed (death) depending on the call priority

## Refactoring
- Update ambulance, hospital, call statuses in database for each event
- Delete unnecessary files
- Add const
- Add tests for database
- Add tests for network
