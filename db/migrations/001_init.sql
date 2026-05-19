CREATE TABLE IF NOT EXISTS simulations (
	id SERIAL PRIMARY KEY
);

-- Entities

CREATE TABLE IF NOT EXISTS hospitals (
	id SERIAL PRIMARY KEY, 
	simulation_id INTEGER NOT NULL REFERENCES simulations(id) ON DELETE CASCADE,
	num_patients INTEGER DEFAULT 0,
	capacity INTEGER NOT NULL,
	x DOUBLE PRECISION NOT NULL,
	y DOUBLE PRECISION NOT NULL
);

CREATE TABLE IF NOT EXISTS ambulances (
	id SERIAL PRIMARY KEY,
	simulation_id INTEGER NOT NULL REFERENCES simulations(id) ON DELETE CASCADE,
	status TEXT NOT NULL, -- Available, Transporting
	type TEXT NOT NULL, -- BLS, ALS
	station_x DOUBLE PRECISION NOT NULL,
	station_y DOUBLE PRECISION NOT NULL,
	current_x DOUBLE PRECISION NOT NULL,
	current_y DOUBLE PRECISION NOT NULL
);

CREATE TABLE IF NOT EXISTS calls (
	id SERIAL PRIMARY KEY,
	simulation_id INTEGER NOT NULL REFERENCES simulations(id) ON DELETE CASCADE,
	call_time INTEGER NOT NULL,  
	priority TEXT NOT NULL, -- Alpha, Bravo, Charlie, Delta, Echo
	description TEXT,
	x DOUBLE PRECISION NOT NULL,
	y DOUBLE PRECISION NOT NULL
);

-- Relationships

CREATE TABLE IF NOT EXISTS dispatches (
	id SERIAL PRIMARY KEY, 
	simulation_id INTEGER NOT NULL REFERENCES simulations(id) ON DELETE CASCADE,
	call_id INTEGER NOT NULL REFERENCES calls(id),
	ambulance_id INTEGER NOT NULL REFERENCES ambulances(id),
	hospital_id INTEGER NOT NULL REFERENCES hospitals(id) 
);

-- Logging

CREATE UNLOGGED TABLE IF NOT EXISTS events (
	id SERIAL PRIMARY KEY,
	simulation_id INTEGER NOT NULL REFERENCES simulations(id) ON DELETE CASCADE,
	event_time INTEGER NOT NULL,
	event_type TEXT NOT NULL, -- CallReceived, AmbulanceArriveAtScene, TransportStart, AmbulanceArriveAtHospital, AmbulanceBackAtStation, PatientDischarged
	call_id INT REFERENCES calls(id) ON DELETE CASCADE,
	ambulance_id INT REFERENCES ambulances(id) ON DELETE SET NULL,
	hospital_id INT REFERENCES hospitals(id) ON DELETE SET NULL
);
