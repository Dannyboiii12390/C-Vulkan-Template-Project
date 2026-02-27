from collections import Counter
import re

with open("report.txt", "r", encoding="utf-8") as f:
    text = f.read()

# Match lines starting with "Severity X"
matches = re.findall(r"Severity\s+(\d+)", text)

counts = Counter(matches)

# Build sets of unique tasks per severity (use message + filename to identify a task)
unique_tasks = {}
for line in text.splitlines():
    parts = line.split('\t')
    if len(parts) < 2:
        continue
    # severity may be in the first tab section
    sev_match = re.search(r"Severity\s+(\d+)", parts[0])
    if not sev_match:
        sev_match = re.search(r"Severity\s+(\d+)", line)
        if not sev_match:
            continue
    sev = sev_match.group(1)
    # message is usually in the second tab section; strip leading [Line N]
    msg = re.sub(r"^\[Line\s*\d+\]\s*", "", parts[1]).strip()
    filename = parts[2].strip() if len(parts) > 2 else ''
    unique_tasks.setdefault(sev, set()).add((msg, filename))

total = sum(counts.values())

print("Counts by severity:")
for sev, cnt in sorted(counts.items(), key=lambda x: int(x[0])):
    print(f"Severity {sev}: {cnt}")

print("\nTotal findings:", total)