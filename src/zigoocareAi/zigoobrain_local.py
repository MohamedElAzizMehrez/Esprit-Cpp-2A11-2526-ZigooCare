import sys
import json
import urllib.request
import urllib.error

# Force UTF-8 output for Qt
try:
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")
except:
    pass

try:
    data = json.load(sys.stdin)

    question = data.get("question", "").strip()
    context = data.get("context", "").strip()

    if not question:
        print("ERROR: Empty question", file=sys.stderr)
        sys.exit(1)

    prompt = f"""
You are ZigooBrain AI, assistant of ZigooCare.

Language rule:
If the question contains ANSWER_LANGUAGE: English, answer only in English.
If the question contains ANSWER_LANGUAGE: French, answer only in French.

Rules:
Use only the context.
Answer directly.
Maximum 4 short lines.
Do not ask for confirmation.

Context:
{context}

Question:
{question}

Answer:
"""

    payload = {
        "model": "llama3.2:3b",
        "prompt": prompt,
        "stream": False,
        "keep_alive": "10m",
        "options": {
            "num_predict": 80,
            "temperature": 0.1,
            "num_ctx": 2048
        }
    }

    request = urllib.request.Request(
        "http://localhost:11434/api/generate",
        data=json.dumps(payload).encode("utf-8"),
        headers={"Content-Type": "application/json; charset=utf-8"},
        method="POST"
    )

    with urllib.request.urlopen(request, timeout=120) as response:
        result = json.loads(response.read().decode("utf-8"))

    answer = result.get("response", "").strip()

    if not answer:
        print("ERROR: Empty response from Ollama", file=sys.stderr)
        sys.exit(1)

    print(answer)

except urllib.error.URLError:
    print("ERROR: Ollama is not running. Please open Ollama or run: ollama serve", file=sys.stderr)
    sys.exit(1)

except Exception as e:
    print("ERROR: " + str(e), file=sys.stderr)
    sys.exit(1)