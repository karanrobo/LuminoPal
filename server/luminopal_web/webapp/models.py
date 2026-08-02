from django.conf import settings
from django.db import models
from django.conf import settings
from django.db import models
import secrets

class Task(models.Model):

    STATUS_CHOICES = [
        ("todo", "To Do"),
        ("active", "Active"),
        ("paused", "Paused"),
        ("completed", "Completed"),
    ]

    user = models.ForeignKey(
        settings.AUTH_USER_MODEL,
        on_delete=models.CASCADE,
        related_name="tasks"
    )

    title = models.CharField(max_length=200)

    description = models.TextField(
        blank=True
    )

    status = models.CharField(
        max_length=20,
        choices=STATUS_CHOICES,
        default="todo"
    )


    # ----------------
    # Deadline system
    # ----------------

    due_at = models.DateTimeField(
        null=True,
        blank=True
    )


    # ----------------
    # Personal timer
    # ----------------

    timer_duration = models.DurationField(
        null=True,
        blank=True
    )

    timer_started_at = models.DateTimeField(
        null=True,
        blank=True
    )

    timer_ends_at = models.DateTimeField(
        null=True,
        blank=True
    )


    created_at = models.DateTimeField(
        auto_now_add=True
    )

    completed_at = models.DateTimeField(
        null=True,
        blank=True
    )

    order = models.PositiveIntegerField(
        default=0
    )





class Lamp(models.Model):

    owner = models.OneToOneField(
        settings.AUTH_USER_MODEL,
        on_delete=models.CASCADE,
        related_name="lamp",
        null=True,
        blank=True
    )

    name = models.CharField(
        max_length=100,
        default="My LuminoPal"
    )

    current_task = models.ForeignKey(
        Task,
        on_delete=models.SET_NULL,
        null=True,
        blank=True,
        related_name="+"
    )

    device_id = models.CharField(
        max_length=64,
        unique=True
    )

    auth_token = models.CharField(
        max_length=128,
        unique=True,
        editable=False,
        default=secrets.token_hex
    )

    pair_code = models.CharField(
        max_length=6,
        blank=True,
        null=True
    )

    is_online = models.BooleanField(
        default=False
    )

    last_seen = models.DateTimeField(
        null=True,
        blank=True
    )

    wifi_name = models.CharField(
        max_length=64,
        blank=True
    )

    created_at = models.DateTimeField(
        auto_now_add=True
    )

    updated_at = models.DateTimeField(
        auto_now=True
    )

    def __str__(self):
        return self.name
