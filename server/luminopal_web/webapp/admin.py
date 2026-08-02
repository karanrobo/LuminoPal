from django.contrib import admin
from .models import Task, Lamp


@admin.register(Task)
class TaskAdmin(admin.ModelAdmin):
    list_display = ("title", "user")


@admin.register(Lamp)
class LampAdmin(admin.ModelAdmin):
    list_display = (
        "name",
        "owner",
        "device_id",
        "is_online",
        "last_seen",
    )
    search_fields = (
        "name",
        "owner__username",
        "device_id",
    )
    list_filter = ("is_online",)

