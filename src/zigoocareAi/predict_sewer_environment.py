import sys
import pandas as pd
import joblib
from pathlib import Path

BASE_DIR = Path(__file__).resolve().parent
MODEL_FILE = BASE_DIR / "models" / "sewer_environment_model.pkl"

model = joblib.load(MODEL_FILE)
if len(sys.argv) != 6:
    print("Erreur: vous devez entrer 5 valeurs.")
    print("Usage:")
    print("python predict_sewer_environment.py MAX_CAPACITY CURRENT_CAPACITY WATER_LEVEL BLOCKAGE_RATE MUNICIPALITY_ID")
    sys.exit(1)

max_capacity = float(sys.argv[1])
current_capacity = float(sys.argv[2])
water_level = float(sys.argv[3])
blockage_rate = float(sys.argv[4])
municipality_id = float(sys.argv[5])


data = pd.DataFrame([{
    "MAX_CAPACITY": max_capacity,
    "CURRENT_CAPACITY": current_capacity,
    "WATER_LEVEL": water_level,
    "BLOCKAGE_RATE": blockage_rate,
    "MUNICIPALITY_ID": municipality_id
}])


prediction = model.predict(data)[0]


probabilities = model.predict_proba(data)[0]
classes = model.classes_

confidence = 0

for i in range(len(classes)):
    if classes[i] == prediction:
        confidence = probabilities[i]


print(f"{prediction};{confidence:.2f}")