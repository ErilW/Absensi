from fastapi import FastAPI, Request 
from starlette.templating import Jinja2Templates
from datetime import datetime
from firebase_controller import FirebaseSingleton
from utils.utils import is_unix_epoch_timestamp, process_epoch

    
app = FastAPI()
templates = Jinja2Templates(directory="templates")
firebase = FirebaseSingleton()

names = firebase.get_db_reference("name").get()
nims = firebase.get_db_reference("nim").get()
roles = firebase.get_db_reference("role").get()


html_data = {
    "nama": [],
    "nim": [],
    "role": [],
    "date": [],
    "time": []
}

def change_data(event):
    """Handle changes in Firebase data."""
    if event.data is None:
        return

    data = event.data 
    
    
    if isinstance(data, int):
        print(event.path)
        print(event.data)
        for key in names.keys():
            if key in event.path:
                if not is_unix_epoch_timestamp(event.data):
                    print(f"Invalid timestamp: {event.data}")
                    return
                
                
                html_data["nama"].insert(0, names[key])
                html_data["nim"].insert(0, nims[key])
                html_data["role"].insert(0, roles[key])
                processed_date = process_epoch(event.data)
                
                html_data["date"].insert(0, processed_date.strftime("%Y-%m-%d"))
                html_data["time"].insert(0, processed_date.strftime("%H:%M:%S"))
        
        return
    
    elif not isinstance(data, dict):
        print("Failed to change data, data is not a dictionary")
        return
    else:
        pass
    
    
    key_to_pop = []
    badge_list = []
    date_list = []
    time_stamp = []

    for badge, value in data.items():
        for val in value.values():
            if not is_unix_epoch_timestamp(val):
                print(f"Invalid timestamp: {val}")
                key_to_pop.append(badge)  # Collect badges with invalid timestamps
            else:
                badge_list.append(badge)
                processed_date = process_epoch(val)
                date_list.append(processed_date.strftime("%Y-%m-%d"))
                time_stamp.append(processed_date.strftime("%H:%M:%S"))

    # Remove invalid badge entries
    for key in key_to_pop:
        data.pop(key)
    
    # Combine and sort data
    combined_list = [
        (badge, f"{date} {time}")
        for badge, date, time in zip(badge_list, date_list, time_stamp)
    ]

    combined_list.sort(key=lambda x: datetime.strptime(x[1], "%Y-%m-%d %H:%M:%S"), reverse=True)

    # Extract sorted lists
    badge_list_sorted = [badge for badge, _ in combined_list]
    date_list_sorted = [datetime.strptime(dt, "%Y-%m-%d %H:%M:%S").strftime("%Y-%m-%d") for _, dt in combined_list]
    time_stamp_sorted = [datetime.strptime(dt, "%Y-%m-%d %H:%M:%S").strftime("%H:%M:%S") for _, dt in combined_list]

    # Update html_data
    # mengisi data nama sesuai len badge dan di urutkan dengan if names.keys == badge_list_sorted
    for badge in badge_list_sorted:
        if badge in names.keys():
            html_data["nama"].append(names[badge])
            html_data["nim"].append(nims[badge])
            html_data["role"].append(roles[badge])
        
    html_data["date"] = date_list_sorted
    html_data["time"] = time_stamp_sorted

# Add listener for Firebase changes
firebase.add_listener("waktu", change_data)

@app.get("/")
async def put_data(request: Request):
    return templates.TemplateResponse("index.html", {"request": request, "datas": html_data})

