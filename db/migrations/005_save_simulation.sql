CREATE TABLE IF NOT EXISTS saved_simulations (
  id SERIAL PRIMARY KEY,
  layout TEXT NOT NULL      
);

CREATE TABLE IF NOT EXISTS saved_calls (
  id SERIAL PRIMARY KEY,
  saved_simulation_id INTEGER NOT NULL REFERENCES saved_simulations(id) ON DELETE CASCADE,
  original_call_id INTEGER NOT NULL,  
  call_time INTEGER NOT NULL,
  priority TEXT NOT NULL,
  x DOUBLE PRECISION NOT NULL,
  y DOUBLE PRECISION NOT NULL
);
