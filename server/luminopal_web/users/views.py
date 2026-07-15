from django.http import HttpResponse
from django.shortcuts import render, redirect
from django.contrib.auth.models import User
from django.contrib.auth import authenticate, login, logout
from .forms import RegisterForm




def register_view(request):

    if request.method == "POST":
        form = RegisterForm(request.POST)

        if form.is_valid():
            user = form.save(commit=False)

            user.set_password(
                form.cleaned_data["password"]
            )

            user.save()
            return redirect("login")
    else: 
        form = RegisterForm()

    
    return render(request, 
        "users/register.html", {
            "form": form
        }
    )



def login_view(request):

    if request.method == "POST":
        username = request.POST["username"]
        password = request.POST["password"]

        user = authenticate(
            request,
            username=username,
            password=password
        )

        if user:

            login(
                request,
                user
            )
            return redirect("/webapp/home")
    
    return render(
        request,
        "users/login.html"
    )
        

def logout_view(request):
    logout(request)
    return redirect("login")