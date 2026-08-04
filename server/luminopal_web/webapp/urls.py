from django.urls import path
from . import views


urlpatterns = [
    path("", views.index, name="index"),

    path(
        "home/",
        views.home,
        name="home"
    ),

    path("lamp/", views.lamp_page, name="lamp"),
    path(
        "lamp/pair/",
        views.lamp_pair,
        name="lamp_pair"
    ),

    #path('api/lamp/task/', views.get_lamp_task_api, name='lamp_task_api'),

    path(
        "lamp/status/",
        views.lamp_status,
        name="lamp_status"
    ),

    path(
            "lamp/register/",
            views.lamp_register,
            name="lamp_register"
        ),

    path(
            "lamp/unpair/<int:id>",
                views.lamp_unpair,
                name="lamp_unpair"
         ),

    path(
            "lamp/edit/<int:id>",
            views.lamp_edit,
            name="lamp_edit"
        ),

    path(
            "lamp/get_tasks/",
            views.get_tasks,
            name="get_tasks"
        ),

    path(
        "lamp/timer/toggle/",
        views.timer_toggle,
        name="timer_toggle"
    ),

    path(
        "task/create/",
        views.create_task,
        name="create_task"
    ),

    

    path(
        "task/delete/<int:id>/",
        views.delete_task,
        name="delete_task"
    ),

    path(
        "task/edit/<int:id>/",
        views.edit_task,
        name="edit_task"
    ),

    path(
        "task/order/",
        views.task_order,
        name="task_order"
    ),

    path(
        "task/timer/start/<int:id>/",
        views.start_timer,
        name="start_timer"
    ),

    path(
        "task/timer/stop/<int:id>/",
        views.stop_timer,
        name="stop_timer"
    ),
]