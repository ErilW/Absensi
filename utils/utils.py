import datetime
def is_unix_epoch_timestamp(timestamp):
    # Determine if timestamp is in milliseconds or seconds
    if timestamp > 1e10:  # This is a heuristic: if the number is greater than 10 billion, it's likely in milliseconds
        # Convert milliseconds to seconds
        timestamp_s = timestamp / 1000
    elif timestamp > 1e7:  # This is a heuristic: if the number is greater than 10 million, it's likely in seconds
        timestamp_s = timestamp
    else: # If the number is too small, it's likely not a valid timestamp
        return False
    
    try:
        # Convert to datetime
        date_time = datetime.datetime.fromtimestamp(timestamp_s)
        
        # Check if the date is within a reasonable range
        current_time = datetime.datetime.now()
        if date_time < datetime.datetime(1970, 1, 1) or date_time > current_time:
            return False
        
        return True
    except Exception as e:
        # If any exception occurs during conversion, it's likely not a valid timestamp
        return False
    
def process_epoch(epoch):
    return datetime.datetime.fromtimestamp(epoch/1000)