#!/bin/bash

# Upgrade the package list
sudo apt update && sudo apt upgrade -y
sudo apt install build-essential -y

# If you want to install clang to format the code 
# automatically with CMake, uncomment line below.
#sudo apt install clang -y 

# Install GTK+3.2
sudo apt install libgtk-3-dev

# Install cURL
sudo apt install curl && sudo apt install libcurl4-openssl-dev

# Install Glade
sudo apt install glade

# Install postgresql and libpq
sudo apt install postgresql && sudo apt install libpq-dev

# Configure postgresql
sudo -i -u postgres psql -c "CREATE USER projet WITH PASSWORD 'Respons11';"
sudo -u postgres createdb projet-todolist --owner projet

# You can now compile the project with CMake and run it.
