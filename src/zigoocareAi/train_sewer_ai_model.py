import pandas as pd
import joblib
from pathlib import Path

from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score, classification_report, confusion_matrix


BASE_DIR = Path(__file__).resolve().parent
DATASET_FILE = BASE_DIR / "data" / "sewer_ai_dataset_augmented_excel.csv"
MODEL_FILE = BASE_DIR / "models" / "sewer_environment_model.pkl"

if not Path(DATASET_FILE).exists():
    raise FileNotFoundError(f"Fichier introuvable : {DATASET_FILE}")

df = pd.read_csv(DATASET_FILE, sep=None, engine="python")


df.columns = df.columns.str.strip()

print("Colonnes trouvées :")
print(df.columns.tolist())

print("\nAperçu du dataset :")
print(df.head())


features = [
    "MAX_CAPACITY",
    "CURRENT_CAPACITY",
    "WATER_LEVEL",
    "BLOCKAGE_RATE",
    "MUNICIPALITY_ID"
]

target = "ENVIRONMENT_STATUS"

for col in features:
    if col not in df.columns:
        raise ValueError(f"Colonne manquante : {col}")

if target not in df.columns:
    raise ValueError(f"Colonne cible manquante : {target}")


X = df[features]
y = df[target]



X_train, X_test, y_train, y_test = train_test_split(
    X,
    y,
    test_size=0.20,
    random_state=42,
    stratify=y
)




model = RandomForestClassifier(
    n_estimators=200,
    random_state=42,
    class_weight="balanced"
)

model.fit(X_train, y_train)



y_pred = model.predict(X_test)

print("\nAccuracy :")
print(accuracy_score(y_test, y_pred))

print("\nRapport de classification :")
print(classification_report(y_test, y_pred))

print("\nMatrice de confusion :")
print(confusion_matrix(y_test, y_pred))



joblib.dump(model, MODEL_FILE)

print(f"\nModèle sauvegardé avec succès : {MODEL_FILE}")