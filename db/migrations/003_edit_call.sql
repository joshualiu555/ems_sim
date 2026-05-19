ALTER TABLE calls
  DROP COLUMN description,   
  ADD COLUMN status TEXT NOT NULL DEFAULT 'Pending'
