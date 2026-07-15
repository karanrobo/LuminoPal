from django.contrib import admin
from .models import Task, Timer


@admin.register(Task)
class TaskAdmin(admin.ModelAdmin):
    list_display = ("title", "user")


@admin.register(Timer)
class TimerAdmin(admin.ModelAdmin):
    list_display = ("title", "user", "duration")