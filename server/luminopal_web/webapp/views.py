import json

from django.http import HttpResponse, JsonResponse
from django.shortcuts import render, redirect, get_object_or_404
from django.contrib.auth.decorators import login_required
from .models import Task, Lamp
from datetime import timedelta
from django.utils import timezone
from django.contrib import messages
import secrets
import string
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt


def index(request):
    return redirect("login")



@login_required
def home(request):

    tasks = Task.objects.filter(
        user=request.user
    ).order_by("order")

    lamp = Lamp.objects.filter(
        owner=request.user
    ).first()

    return render(
        request,
        "webapp/home.html",
        {
            "tasks": tasks,
            "lamp": lamp,
            "page": "home",
        }
    )

@login_required
def create_task(request):

    if request.method == "POST":

        title = request.POST.get("title")

        duration = request.POST.get("duration")


        task = Task.objects.create(
            user=request.user,
            title=title,
        )


        if duration:
            task.timer_duration = timedelta(
                minutes=int(duration)
            )


        task.save()


    return redirect("home")




@login_required
def delete_task(request,id):

    Task.objects.filter(
        id=id,
        user=request.user
    ).delete()

    return redirect("home")


@login_required
def start_timer(request,id):

    task = get_object_or_404(
        Task,
        id=id,
        user=request.user
    )


    task.timer_started_at = timezone.now()

    task.timer_ends_at = (
        timezone.now()
        +
        task.timer_duration
    )

    task.status = "active"

    task.save()


    return redirect("home")

@login_required
def stop_timer(request,id):

    task = get_object_or_404(
        Task,
        id=id,
        user=request.user
    )


    task.timer_started_at = None
    task.timer_ends_at = None

    task.status = "paused"

    task.save()


    return redirect("home")


@login_required
def edit_task(request,id):

    task = get_object_or_404(
        Task,
        id=id,
        user=request.user
    )


    if request.method == "POST":

        task.title = request.POST.get(
            "title"
        )

        task.description = request.POST.get(
            "description"
        )

        task.status = request.POST.get(
            "status"
        )


        duration = request.POST.get(
            "duration"
        )

        if duration:
            task.timer_duration = timedelta(
                minutes=int(duration)
            )


        task.save()


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


    lamp = Lamp.objects.filter(
        owner=request.user
    ).first()


    if lamp:

        first_task = Task.objects.filter(
            user=lamp.owner
        ).exclude(
            status="completed"
        ).order_by(
            "order"
        ).first()


        lamp.current_task = first_task
        lamp.save()


    return JsonResponse(
        {
            "success":True
        }
    )


@login_required
def lamp_page(request):
    tasks = Task.objects.filter(user=request.user).order_by("order")
    lamp = Lamp.objects.filter(owner=request.user).first()

    return render(request, "webapp/home.html", {
        "tasks": tasks,
        "lamp": lamp,
        "page": "lamp",
    })


def generate_pair_code():

    chars = string.ascii_uppercase + string.digits

    return ''.join(
        secrets.choice(chars)
        for _ in range(6)
    )

@csrf_exempt
@login_required
def lamp_pair(request):

    if request.method == "POST":

        pair_code = request.POST.get(
            "pair_code"
        )


        if not pair_code:
            messages.error(
                request,
                "No pairing code provided"
            )

            return redirect("lamp")



        try:
            lamp = Lamp.objects.get(
                pair_code=pair_code
            )

        except Lamp.DoesNotExist:

            messages.error(
                request,
                "Invalid pairing code"
            )

            return redirect("lamp")



        lamp.owner = request.user
        lamp.pair_code = None

        lamp.save()


        messages.success(
            request,
            "Lamp paired successfully!"
        )


    return redirect("lamp")




@csrf_exempt
def lamp_status(request):

    device_id = request.GET.get("device_id")


    try:
        lamp = Lamp.objects.get(
            device_id=device_id
        )

    except Lamp.DoesNotExist:
        return JsonResponse(
            {"error": "Lamp not found"},
            status=404
        )


    if lamp.owner is not None:

        return JsonResponse(
            {
                "paired": True,
                "auth_token": lamp.auth_token
            }
        )


    return JsonResponse(
        {
            "paired": False
        }
    )





@csrf_exempt
def lamp_register(request):

    print("REGISTER CALLED")
    print(request.body)
    print(request.method)
    if request.method != "POST":
        return JsonResponse(
            {
                "error": "POST required"
            },
            status=400
        )


    try:
        data = json.loads(
            request.body
        )

    except json.JSONDecodeError:

        return JsonResponse(
            {
                "error": "Invalid JSON"
            },
            status=400
        )


    device_id = data.get(
        "device_id"
    )


    if not device_id:

        return JsonResponse(
            {
                "error": "Missing device_id"
            },
            status=400
        )


    lamp, created = Lamp.objects.get_or_create(
        device_id=device_id
    )


    # Generate a new code only if needed
    if not lamp.pair_code and lamp.owner is None:

        lamp.pair_code = generate_pair_code()

        lamp.save()


    response = {
        "pair_code": lamp.pair_code,
        "paired": lamp.owner is not None
    }

    print(response)

    return JsonResponse(response)




def get_lamp_from_request(request):

    device_id = request.GET.get(
        "device_id"
    )

    token = request.headers.get(
        "Authorization"
    )


    if not device_id or not token:
        return None


    try:
        lamp = Lamp.objects.get(
            device_id=device_id,
            auth_token=token
        )

        return lamp

    except Lamp.DoesNotExist:
        return None

@csrf_exempt
def get_tasks(request):

    device_id = request.GET.get("device_id")

    token = request.headers.get("Authorization")


    try:
        lamp = Lamp.objects.get(
            device_id=device_id,
            auth_token=token
        )

    except Lamp.DoesNotExist:
        return JsonResponse(
            {
                "error": "Unauthorized"
            },
            status=401
        )


    if lamp.owner is None:
        return JsonResponse(
            {
                "paired": False
            }
        )


    # Assign top task if none exists
    if lamp.current_task is None:

        task = Task.objects.filter(
            user=lamp.owner
        ).order_by(
            "order"
        ).first()


        lamp.current_task = task
        lamp.save()

    else:
        task = lamp.current_task



    if task is None:

        return JsonResponse(
            {
                "paired": True,
                "task": None
            }
        )


    return JsonResponse(
        {
            "paired": True,
            "task":
            {
                "title": task.title,

                "description":
                    task.description or "",


                "timer_end":
                    int(task.timer_ends_at.timestamp())
                    if task.timer_ends_at
                    else 0,


                "deadline":
                    int(task.due_at.timestamp())
                    if task.due_at
                    else 0
            }
        }
    )

@login_required
def lamp_unpair(request,id):

    lamp = get_object_or_404(
        Lamp,
        id=id,
        owner=request.user
    )


    lamp.owner = None

    lamp.pair_code = generate_pair_code()

    lamp.save()


    messages.success(
        request,
        "Lamp unpaired"
    )


    return redirect(
        "lamp"
    )



@login_required
def lamp_edit(request,id):

    lamp = get_object_or_404(
        Lamp,
        id=id,
        owner=request.user
    )


    if request.method == "POST":

        lamp.name = request.POST.get(
            "name"
        )

        lamp.save()


        messages.success(
            request,
            "Lamp updated"
        )


    return redirect(
        "lamp"
    )