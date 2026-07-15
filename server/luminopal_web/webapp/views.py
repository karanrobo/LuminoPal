import json

from django.http import HttpResponse, JsonResponse
from django.shortcuts import render, redirect, get_object_or_404
from django.contrib.auth.decorators import login_required
from .models import Task, Timer
from datetime import timedelta

def index(request):
    return HttpResponse("There will be a task list here")



@login_required
def home(request):
    tasks = Task.objects.filter(user=request.user)
    timers = Timer.objects.filter(user=request.user)
    return render(
        request, "webapp/home.html", 
                  {
                      "tasks": tasks, 
                      "timers": timers
                    }
                )

@login_required
def create_task(request):
    if request.method == "POST":
        taskname = request.POST["tasks"]

        if taskname:
            Task.objects.create(
                user=request.user,
                title=taskname,
            )
            

    return redirect("home")


@login_required
def create_timer(request):
    if request.method == "POST":
        timer_name = request.POST["timers"]
        mins = int(request.POST["duration"])
        duration = timedelta(minutes=mins)
        if timer_name:
            Timer.objects.create(
                user=request.user,
                title=timer_name,
                duration=duration
            )
           

    return redirect("home")




@login_required
def delete_task(request,id):

    Task.objects.filter(
        id=id,
        user=request.user
    ).delete()

    return redirect("home")



@login_required
def delete_timer(request,id):

    Timer.objects.filter(
        id=id,
        user=request.user
    ).delete()

    return redirect("home")






@login_required
def edit_task(request,id):

    task=get_object_or_404(
        Task,
        id=id,
        user=request.user
    )


    if request.method=="POST":

        task.title=request.POST["title"]

        task.description=request.POST["description"]

        task.save()


    return redirect("home")





@login_required
def edit_timer(request,id):

    timer=get_object_or_404(
        Timer,
        id=id,
        user=request.user
    )


    if request.method=="POST":

        timer.title=request.POST["title"]

        timer.status=request.POST["status"]

        timer.save()


    return redirect("home")







@login_required
def task_order(request):

    data=json.loads(request.body)


    for item in data:

        Task.objects.filter(
            id=item["id"],
            user=request.user
        ).update(
            order=item["order"]
        )


    return JsonResponse(
        {
            "success":True
        }
    )






@login_required
def timer_order(request):

    data=json.loads(request.body)


    for item in data:

        Timer.objects.filter(
            id=item["id"],
            user=request.user
        ).update(
            order=item["order"]
        )


    return JsonResponse(
        {
            "success":True
        }
    )