from pymongo import MongoClient
import pandas as pd
import numpy as np
from sklearn import preprocessing

client = MongoClient('localhost', 27017)
db = client.flask_database
sensor_data_collection = db.mideval

data = list(sensor_data_collection.find({}, {"_id": 0}))
df = pd.DataFrame(data)

def classify_water_safety(tds, turbidity, ph):
    is_tds_safe = 0 <= tds <= 1000
    is_turbidity_safe = 0 <= turbidity <= 5
    is_ph_safe = 6.5 <= ph <= 8.5
    if is_tds_safe and is_turbidity_safe and is_ph_safe:
        return 1
    else:
        return 0
    
df['is_safe'] = df.apply(lambda row: classify_water_safety(row['TDS'], row['TURBIDITY'], row['PH']), axis=1)

def predict_water_safety(new_data):
    result = classify_water_safety(new_data['TDS'], new_data['TURBIDITY'], new_data['PH'])

    reasons = []
    if new_data['TDS'] > 1000:
        reasons.append("TDS is too high")
    if new_data['TURBIDITY'] > 5:
        reasons.append("Turbidity is too high")
    if new_data['PH'] < 6.5 or new_data['PH'] > 8.5:
        reasons.append("pH is out of range")

    return {'is_safe':result, 'reasons': reasons if not result else ["All parameters are within the safe range"]}

if __name__ == "__main__":
    test_sample = {
        'TDS': 500,
        'TURBIDITY': 2.5,
        'PH': 7.0
    }

    prediction = predict_water_safety(test_sample)
    print("\nWater Quality Analysis:")
    print("----------------------")
    print(f"Test Sample: {test_sample}")
    print(f"Is Water Safe? {'Yes' if prediction['is_safe'] else 'No'}")
    print(f"Reason: {', '.join(prediction['reasons'])}")
    
    print("\nDataset Statistics:")
    print(f"Total Samples: {len(df)}")
    print(f"Safe Samples: {df['is_safe'].sum()}")
    print(f"Unsafe Samples: {len(df) - df['is_safe'].sum()}")
    print(f"Percentage Safe: {(df['is_safe'].sum() / len(df) * 100):.2f}%")

