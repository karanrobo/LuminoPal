from django.urls import path
from . import views

urlpatterns = [
    path("", views.index, name = "index"),
    path("home", views.home, name="home"),
    path("create_tasks/", views.create_task, name="create_task"),
    path("create_timers/", views.create_timer, name="create_timer"),

    path("delete_task/<int:id>/",views.delete_task,name="delete_task"),
    path("delete_timer/<int:id>/",views.delete_timer,name="delete_timer"),

    path(
        "task/edit/<int:id>/",
        views.edit_task,
        name="edit_task"
    ),


    path(
        "timer/edit/<int:id>/",
        views.edit_timer,
        name="edit_timer"
    ),



    path(
        "task/order/",
        views.task_order,
        name="task_order"
    ),


    path(
        "timer/order/",
        views.timer_order,
        name="timer_order"
    ),
]