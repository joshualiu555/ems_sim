-- Entities

CREATE TABLE IF NOT EXISTS hospitals (
    id SERIAL PRIMARY KEY, 
    num_patients INTEGER DEFAULT 0,
    capacity INTEGER NOT NULL,
    lat DOUBLE PRECISION NOT NULL,
    lon DOUBLE PRECISION NOT NULL
);

CREATE TABLE IF NOT EXISTS ambulances (
    id SERIAL PRIMARY KEY,
    status TEXT NOT NULL, -- Available, Transporting
    type TEXT NOT NULL, -- BLS, ALS
    lat DOUBLE PRECISION NOT NULL,
    lon DOUBLE PRECISION NOT NULL
);

CREATE TABLE IF NOT EXISTS calls (
    id SERIAL PRIMARY KEY,
    call_time INTEGER NOT NULL,  
    priority TEXT NOT NULL, -- Alpha, Bravo, Charlie, Delta, Echo
    description TEXT,
    lat DOUBLE PRECISION NOT NULL,
    lon DOUBLE PRECISION NOT NULL
);

-- Relationships

CREATE TABLE IF NOT EXISTS dispatches (
    id SERIAL PRIMARY KEY, 
    call_id INTEGER NOT NULL REFERENCES calls(id),
    ambulance_id INTEGER NOT NULL REFERENCES ambulances(id),
    hospital_id INTEGER NOT NULL REFERENCES hospitals(id) 
);

-- Logging

CREATE UNLOGGED TABLE IF NOT EXISTS events (
    id SERIAL PRIMARY KEY,
    event_time INTEGER NOT NULL,
    event_type TEXT NOT NULL, -- CallReceived, AmbulanceArriveAtScene, TransportStart, AmbulanceArriveAtHospital, AmbulanceBackAtStation
    call_id INTEGER,
    ambulance_id INTEGER,
    hospital_id INTEGER
);
