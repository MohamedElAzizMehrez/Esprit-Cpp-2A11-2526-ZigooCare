import speech_recognition as sr
import sounddevice as sd
from scipy.io.wavfile import write
import tempfile
import sys
import os
import re

try:
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")
except:
    pass

DURATION = 5
SAMPLE_RATE = 16000

def score_french(text):
    text = text.lower()
    score = 0

    french_words = [
        "quel", "quelle", "quels", "quelles", "est-ce", "est ce",
        "comment", "pourquoi", "combien", "égout", "egout",
        "critique", "dangereux", "municipalité", "municipalite",
        "situation", "pluie", "risque", "zigoocare", "explique",
        "donne", "doit", "faire", "avant", "actuel", "actuelle"
    ]

    for word in french_words:
        if word in text:
            score += 2

    if any(c in text for c in "éèêàçùîô"):
        score += 3

    return score


def score_english(text):
    text = text.lower()
    score = 0

    english_words = [
        "what", "which", "where", "when", "why", "how",
        "is", "are", "the", "sewer", "sewers", "critical",
        "dangerous", "risk", "rain", "municipality",
        "situation", "current", "explain", "should",
        "do", "before", "flood", "zigoocare", "status"
    ]

    for word in english_words:
        if re.search(r"\b" + re.escape(word) + r"\b", text):
            score += 2

    return score


try:
    audio_data = sd.rec(
        int(DURATION * SAMPLE_RATE),
        samplerate=SAMPLE_RATE,
        channels=1,
        dtype="int16"
    )

    sd.wait()

    with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as temp_audio:
        audio_path = temp_audio.name

    write(audio_path, SAMPLE_RATE, audio_data)

    recognizer = sr.Recognizer()

    with sr.AudioFile(audio_path) as source:
        audio = recognizer.record(source)

    try:
        os.remove(audio_path)
    except:
        pass

    text_fr = ""
    text_en = ""

    try:
        text_fr = recognizer.recognize_google(audio, language="fr-FR").strip()
    except:
        pass

    try:
        text_en = recognizer.recognize_google(audio, language="en-US").strip()
    except:
        pass

    if not text_fr and not text_en:
        print("ERROR: I could not understand the question", file=sys.stderr)
        sys.exit(1)

    fr_score = score_french(text_fr)
    en_score = score_english(text_en)

    # Choose the most probable language
    if text_en and en_score >= fr_score:
        print("EN:" + text_en)
    elif text_fr:
        print("FR:" + text_fr)
    else:
        print("EN:" + text_en)

except Exception as e:
    print("ERROR: " + str(e), file=sys.stderr)
    sys.exit(1)