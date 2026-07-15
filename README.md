# LuminoPal

LuminoPal has two parts:

1. ESP32 firmware using ESP-IDF
2. Django web backend

Repository:

https://github.com/karanrobo/LuminoPal.git

---

# Platform Support

- Windows: Supported for full development
- macOS: Supported for full development
- iOS (iPhone/iPad): Not supported for ESP-IDF development toolchain
- iOS devices can still access the Django web app in a browser if network allows

---

# Project Structure

```
esp32_lamp/               # ESP-IDF firmware
server/luminopal_web/     # Django backend
```

---

# Prerequisites

Install these first:

1. Git  
   https://git-scm.com/downloads

2. Python 3.10+ (3.11 recommended)  
   https://www.python.org/downloads/

3. ESP-IDF  
   https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/

4. VS Code (optional)  
   https://code.visualstudio.com/

Optional USB Drivers

- CP210x  
  https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers

- CH340  
  https://sparks.gogo.co.nz/ch340.html

---

# 1. Clone the Repository

```bash
git clone https://github.com/karanrobo/LuminoPal.git
cd LuminoPal
```

---

# 2. Install ESP-IDF

### Windows Guide

https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/windows-setup.html

### macOS Guide

https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/macos-setup.html

Verify the installation:

```bash
idf.py --version
```

---

# 3. Build and Flash Firmware

## Windows (ESP-IDF PowerShell)

```bash
cd C:\path\to\LuminoPal\esp32_lamp

idf.py set-target esp32
idf.py build
idf.py -p COM5 flash monitor
```

## macOS

```bash
cd /path/to/LuminoPal/esp32_lamp

idf.py set-target esp32
idf.py build
idf.py -p /dev/cu.usbserial-xxxx flash monitor
```

Replace `COM5` or `/dev/cu.usbserial-xxxx` with your board's serial port.

Exit the monitor using:

```
Ctrl + ]
```

---

# 4. Django Setup

Navigate to the Django project:

```bash
cd server/luminopal_web
```

## Windows (PowerShell)

```powershell
python -m venv .venv

.\.venv\Scripts\Activate.ps1

pip install -r requirements.txt

python manage.py migrate

python manage.py runserver
```

## macOS / Linux

```bash
python3 -m venv .venv

source .venv/bin/activate

pip install -r requirements.txt

python manage.py migrate

python manage.py runserver
```

Open your browser:

```
http://127.0.0.1:8000/
```

---

# 5. Update requirements.txt

Whenever new Python packages are installed:

```bash
pip freeze > requirements.txt
```

Commit the updated `requirements.txt`.

---

# 6. Team Collaboration Workflow

## Before Starting Work

```bash
git checkout main
git pull
```

## Create a Feature Branch

```bash
git checkout -b feature/short-name
```

## Commit Changes

```bash
git add .

git commit -m "Clear commit message"

git push -u origin feature/short-name
```

Then open a Pull Request into `main`.

---

# 7. Avoid Environment Conflicts

Use two separate terminals.

**Terminal A**

- ESP-IDF
- Working inside `esp32_lamp`

**Terminal B**

- Django
- Working inside `server/luminopal_web`

Do not mix both environments in the same terminal session.

---

# 8. Troubleshooting

## `idf.py` not found

- Windows: Use the ESP-IDF PowerShell
- macOS: Run the ESP-IDF export/init step from the official installation guide

---

## Board Port Not Found

- Reconnect USB cable
- Try another USB port
- Install the correct USB drivers
- Check your operating system's device list

---

## PowerShell Blocks Virtual Environment

Run:

```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

Then activate the virtual environment again.

---

## Django Fails to Run

Ensure the virtual environment is active, then run:

```bash
pip install -r requirements.txt

python manage.py migrate
```

---

# 9. Important Git Rules

Never commit:

- `.venv`
- `venv`
- `virtualEnv`
- `build/`
- `db.sqlite3`
- `.env`
- secrets or API keys

---

# 10. Reference Links

ESP-IDF Documentation

https://docs.espressif.com/projects/esp-idf/en/latest/esp32/

ESP-IDF Examples

https://github.com/espressif/esp-idf/tree/master/examples

Django Documentation

https://docs.djangoproject.com/

GitHub Flow

https://docs.github.com/en/get-started/using-github/github-flow

---

# 11. Contributor Checklist

Before pushing:

- [ ] Firmware builds successfully
- [ ] Django server runs locally
- [ ] `requirements.txt` updated if dependencies changed
- [ ] No build, virtual environment, or database artifacts committed
- [ ] Commit message is descriptive