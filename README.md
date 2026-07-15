# LuminoPal

LuminoPal has two parts:
1. ESP32 firmware using ESP-IDF
2. Django web backend

Repository:
https://github.com/karanrobo/LuminoPal.git

## Platform Support

- Windows: Supported for full development
- macOS: Supported for full development
- iOS (iPhone/iPad): Not supported for ESP-IDF development toolchain
- iOS devices can still access the Django web app in browser if network allows

## Project Structure

- esp32_lamp -> ESP-IDF firmware
- server/luminopal_web -> Django app

## Prerequisites

Install these first:
1. Git: https://git-scm.com/downloads
2. Python 3.10+ (3.11 recommended): https://www.python.org/downloads/
3. ESP-IDF docs: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/
4. VS Code (optional): https://code.visualstudio.com/

Optional USB drivers:
- CP210x: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
- CH340: https://sparks.gogo.co.nz/ch340.html

## 1) Clone Repo

Run:
git clone https://github.com/karanrobo/LuminoPal.git
cd LuminoPal

## 2) ESP-IDF Install

Windows guide:
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/windows-setup.html

macOS guide:
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/macos-setup.html

Verify:
idf.py --version

## 3) Build and Flash Firmware

Windows (ESP-IDF PowerShell):
cd C:\path\to\LuminoPal\esp32_lamp
idf.py set-target esp32
idf.py build
idf.py -p COM5 flash monitor

macOS:
cd /path/to/LuminoPal/esp32_lamp
idf.py set-target esp32
idf.py build
idf.py -p /dev/cu.usbserial-xxxx flash monitor

Replace COM5 or /dev/cu.usbserial-xxxx with your actual device port.
Exit monitor with Ctrl + ].

## 4) Django Setup

Go to Django folder:
cd server/luminopal_web

Windows PowerShell:
python -m venv .venv
.\.venv\Scripts\Activate.ps1
pip install -r requirements.txt
python manage.py migrate
python manage.py runserver

macOS Terminal:
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
python manage.py migrate
python manage.py runserver

Open:
http://127.0.0.1:8000/

## 5) If requirements.txt needs update

After installing new packages:
pip freeze > requirements.txt

Commit updated requirements.txt.

## 6) Team Collaboration Flow

Before work:
git checkout main
git pull

Create branch:
git checkout -b feature/short-name

Commit and push:
git add .
git commit -m "clear message"
git push -u origin feature/short-name

Open Pull Request into main.

## 7) Avoid Environment Conflicts

Use separate terminals:
- Terminal A: ESP-IDF commands in esp32_lamp
- Terminal B: Django commands in server/luminopal_web

Do not mix both environments in the same terminal session.

## 8) Troubleshooting

idf.py not found:
- Windows: use ESP-IDF PowerShell
- macOS: ensure ESP-IDF export/init step from official guide is done

Board port not found:
- reconnect USB
- change cable/port
- install correct USB driver
- check system device list

PowerShell blocks venv activation:
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser

Then activate venv again.

Django fails to run:
- make sure venv is active
- run pip install -r requirements.txt
- run python manage.py migrate

## 9) Important Git Rules

Never commit:
- .venv, venv, virtualEnv
- build folders
- db.sqlite3
- .env secrets

## 10) Reference Links

ESP-IDF docs:
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/

ESP-IDF examples:
https://github.com/espressif/esp-idf/tree/master/examples

Django docs:
https://docs.djangoproject.com/

GitHub flow:
https://docs.github.com/en/get-started/using-github/github-flow

## 11) Contributor Checklist

Before pushing:
1. Firmware builds successfully
2. Django server runs locally
3. requirements.txt updated if dependencies changed
4. No build/venv/db artifacts in commit
5. Commit message is clear

If you want, I can also give a super short Quick Start section (10 lines) to place at the top for first-time teammates.