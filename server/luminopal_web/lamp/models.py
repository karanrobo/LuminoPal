from django.conf import settings
from django.db import models

class Lamp(models.Model):
    # status for the lamp
    LAMP_STATUS = [
        ("offline", "offline"),
        ("online", "online"),
        ("paused", "Paused"),
        ("finished", "Finished"),
    ]
    lamp_name = models.CharField(max_length=200)
    api_address = models.CharField(max_length=500)

