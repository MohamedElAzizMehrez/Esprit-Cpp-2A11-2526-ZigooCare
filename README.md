# ZigooCare

## Description

ZigooCare is a smart desktop application developed to support municipalities in sewer, waste collection, and recycling management.

The application allows users to manage sewers, municipalities, employees, collection teams, and recycling centers through a desktop interface built with Qt/C++. It also provides dashboards and statistics to visualize sewer conditions, blockage rates, and environmental risks.

ZigooCare integrates artificial intelligence features, including an Environmental AI predictor and a voice assistant called ZigooBot. The Environmental AI module predicts the sewer environment status based on maximum capacity, current capacity, water level, blockage rate, and municipality data. The voice assistant uses speech recognition and a local Ollama model to answer user questions based on the application context.

The main objective of ZigooCare is to improve urban infrastructure monitoring, support faster decision-making, and reduce environmental risks related to sewer overflow, flooding, and waste management.

---

## Technologies Used

### Frontend / Desktop Application

- C++17
- Qt Creator
- Qt 6.7.3
- Qt Widgets
- Qt Charts
- Qt SQL
- Qt Serial Port
- Qt TextToSpeech
- Qt Location / Positioning
- Qt Quick
- Qt Quick Widgets

### Database

- Oracle Database
- Oracle SQL Developer
- Oracle ODBC connection

### Artificial Intelligence

- Python 3.14.4
- scikit-learn
- pandas
- numpy
- joblib
- SpeechRecognition
- sounddevice
- Ollama `llama3.2:3b`

### Other Tools

- Git / GitHub
- Google Drive
- Google Colab

---

## Supported Environment

This project was developed and tested on:

```txt
OS: Windows 10/11
IDE: Qt Creator
Qt Version: 6.7.3
Compiler: MinGW 11.2.0 64-bit
C++ Standard: C++17
Database: Oracle Database
Python Version: Python 3.14.4
```


## Prerequisites

Before running the project, make sure the following tools are installed:

- Qt Creator with Qt 6.7.3
- MinGW 11.2.0 64-bit
- Oracle Database
- Oracle SQL Developer or any Oracle-compatible SQL tool
- Oracle ODBC driver
- Python 3.14.4
- Git
- Ollama, required only for the voice assistant

---

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/USERNAME/Esprit-PI-2A11-2526-ZigooCare.git
cd Esprit-PI-2A11-2526-ZigooCare
```

### 2. Create the Local Environment File

The repository contains a `.env.example` file.

Create a local `.env` file from `.env.example`:

```bash
Copy-Item .env.example .env
```

Then open `.env` and replace the placeholder values with your own local configuration.

---

### 3. Install Python Dependencies

Open PowerShell in the project folder and run:

```bash
cd zigoocareAi
py -m pip install --upgrade pip
py -m pip install -r requirements.txt
cd ..
```

---

### 4. Download the Trained AI Model

The trained AI model is not included directly in the GitHub repository.

Required file:

```txt
zigoocareAi/models/sewer_environment_model.pkl
```

Download link:

[Download sewer_environment_model.pkl](https://drive.google.com/file/d/1Hsvm8yWZuXed8_Np5sh3N5NAG0dMLkHI/view?usp=sharing)

After downloading the model, place it inside:

```txt
zigoocareAi/models/
```

---

### 5. Download the Dataset

The full training dataset is not included directly in the GitHub repository.

Required dataset:

```txt
zigoocareAi/data/sewer_ai_dataset_augmented_excel.csv
```

Dataset link:

[Download sewer_ai_dataset_augmented_excel.csv](https://drive.google.com/file/d/1wvAhkVkIW1U6sTD3YeNi9LSA0AbC6_yY/view?usp=sharing)

After downloading the dataset, place it inside:

```txt
zigoocareAi/data/
```

A small sample dataset is provided in:

```txt
zigoocareAi/data/sample/
```

## Database Setup

This project uses an Oracle Database.

To run the application correctly, the database must be created before launching the Qt project.

### Database Requirements

- Oracle Database
- Oracle SQL Developer or any Oracle-compatible SQL tool
- Oracle ODBC driver
- SQL script provided in the project repository

### Database Installation Steps

1. Open Oracle SQL Developer.
2. Create or connect to an Oracle user/schema.
3. Run the SQL script provided in the main project folder:

```txt
zigoo_care.sql
```

4. Make sure the required tables are created, including:

```txt
EMPLOYEE
MUNICIPALITY
SEWER
COLLECTION_TEAM
RECYCLING_CENTER
INTERVENES
```

5. Create or configure your Oracle ODBC data source.
6. Open the local `.env` file.
7. Add your own Oracle configuration:

```env
ODBC_SOURCE_NAME=YOUR_ODBC_SOURCE_NAME
DB_USER=YOUR_ORACLE_USERNAME
DB_PASSWORD=YOUR_ORACLE_PASSWORD
```

## Qt Installation

This project was developed using Qt Creator and Qt 6.7.3.

To install Qt:

1. Download the Qt online installer from the official Qt website.
2. Run the installer.
3. Log in or create a Qt account.
4. Choose a custom installation.
5. Select Qt version `6.7.3`.
6. Select the required kit:

```txt
MinGW 11.2.0 64-bit
```

7. Install the required Qt modules:

```txt
Qt Charts
Qt SQL
Qt Serial Port
Qt TextToSpeech
Qt Location
Qt Positioning
Qt Quick
Qt Quick Widgets
```

8. Finish the installation.
9. Open the project file in Qt Creator:

```txt
WasteCollection.pro
```

10. Run the following steps in Qt Creator:

```txt
Run qmake
Build
Run
```

---

## Running the Application

To run the complete ZigooCare desktop application:

1. Make sure Oracle Database is installed and configured.
2. Make sure the SQL script has been executed:

```txt
zigoo_care.sql
```

3. Make sure the local `.env` file exists and contains your Oracle configuration.
4. Make sure the AI model file exists at:

```txt
zigoocareAi/models/sewer_environment_model.pkl
```

5. Open Qt Creator.
6. Open the project file:

```txt
WasteCollection.pro
```

7. Run:

```txt
Run qmake
Build
Run
```

8. The ZigooCare desktop application will start.

---

## AI Module

ZigooCare includes an AI module used for sewer environment prediction and for the ZigooBot voice assistant.

### AI Features

- Environmental AI predictor
- Voice assistant using speech recognition
- Local AI assistant using Ollama
- Google Colab demo for testing the AI prediction model online

### Python and GPU Requirements

```txt
Python Version: Python 3.14.4
CUDA: Not required
cuDNN: Not required
```

GPU acceleration is not required. The AI module runs on CPU and does not require CUDA or cuDNN.

---

## Environmental AI Prediction Model

The Environmental AI model predicts the sewer environment status based on:

- Maximum capacity
- Current capacity
- Water level
- Blockage rate
- Municipality ID

Model output:

- Predicted sewer status
- Confidence score

### Run Environmental AI Prediction Locally

Open PowerShell and run:

```bash
cd zigoocareAi
py predict_sewer_environment.py 1000 700 60 70 1
```

Arguments:

```txt
py predict_sewer_environment.py <max_capacity> <current_capacity> <water_level> <blockage_rate> <municipality_id>
```

Example:

```txt
max_capacity = 1000
current_capacity = 700
water_level = 60
blockage_rate = 70
municipality_id = 1
```

Expected output format:

```txt
Status;Confidence
```

Example output:

```txt
Poor;0.85
```

### AI Model Performance

```txt
Accuracy: 96.88%
F1-score: 0.97
```

### Online AI Demo

The Environmental AI prediction model can be tested online using Google Colab:

[Open ZigooCare AI Demo on Google Colab](https://colab.research.google.com/drive/1ZB9mTfs6sCUp3hOrm3CUO8M5r5GIuAt0?usp=sharing)

---

## Ollama Voice Assistant Setup

ZigooCare includes a voice assistant called ZigooBot.

The voice assistant allows the user to ask questions using the microphone. The question is converted into text, then sent to a local Ollama model together with the ZigooCare application context.

### Voice Assistant Requirements

To use the voice assistant, the following elements are required:

- Python dependencies installed
- Working microphone
- Internet access for speech recognition
- Ollama installed locally
- `llama3.2:3b` model downloaded locally

### Install Ollama

Ollama must be installed locally before using the voice assistant.

On Windows, Ollama can be installed from the official Ollama website or by running the following command in PowerShell:

```bash
irm https://ollama.com/install.ps1 | iex
```

After installation, close PowerShell and open it again, then verify the installation:

```bash
ollama --version
```

Ollama for Windows requires Windows 10 or later.

### Install the Ollama Model

Download the required local model:

```bash
ollama pull llama3.2:3b
```

To test the model:

```bash
ollama run llama3.2:3b
```

### How to Use ZigooBot

1. Launch the ZigooCare desktop application.
2. Click on the ZigooBot voice assistant button.
3. Ask a question using the microphone.
4. The assistant generates an answer based on the current ZigooCare data.

Example questions:

```txt
What is the most critical sewer?
Give me a summary of the sewer situation.
Is there a flood risk?
Which sewer needs urgent intervention?
```

### Important Note

Ollama installation and model download may take around 10 to 25 minutes depending on the internet connection.

The voice assistant must be tested locally because it depends on the microphone, the Qt desktop interface, and the local Ollama installation.

---


## Demo

### Application Demo

A demonstration of the ZigooCare desktop application is available in the `demo/` folder.


### Online AI Demo

The Environmental AI prediction model can be tested online using Google Colab:

[Open ZigooCare AI Demo on Google Colab](https://colab.research.google.com/drive/1ZB9mTfs6sCUp3hOrm3CUO8M5r5GIuAt0?usp=sharing)

---
## License

All rights reserved.

This project is provided only for academic review, demonstration, and evaluation purposes within the context of ESPRIT School of Engineering.

No use, copy, modification, distribution, publication, commercialization, or derivative work is allowed without prior written authorization from the EcoCutie Team.

See the [LICENSE](LICENSE) file for details.

## Authors

ZigooCare — 2A11 — 2025/2026  
Tutor: Soumaya Argoubi

Team members:
- [Youssef Bouchehioua](https://github.com/Joe06-b)
- [Mohamed Aziz Darnaoui](https://github.com/medazizdarnaoui)
- [Maissa Hammami](https://github.com/Maissahm)
- [Zayneb Hergli](https://github.com/Zayneb0)
- [Mohamed El Aziz Mehrez](https://github.com/MohamedElAzizMehrez)