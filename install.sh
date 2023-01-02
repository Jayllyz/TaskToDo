#!/bin/bash
# This script install all the dependencies for the project

# Upgrade the package list
sudo apt update -y 
sudo apt upgrade -y
sudo apt install build-essential -y

# Install CMake
sudo apt install cmake -y

# If you want to install clang to format the code 
# automatically with CMake, uncomment line below.
#sudo apt install clang -y 

# Install GTK+3.2
sudo apt install libgtk-3-dev -y

# Install cURL
sudo apt install curl -y
sudo apt install libcurl4-openssl-dev -y

# Install Glade
sudo apt install glade -y

# Install postgresql and libpq
sudo apt install postgresql -y
sudo apt install libpq-dev -y

# Configure postgresql
sudo -i -u postgres psql -c "CREATE USER project WITH PASSWORD 'Respons11';"
sudo -u postgres createdb TaskToDo --owner project

# Change the authentication method to scram-sha-256 in pg_hba.conf because
# the default method is peer and it doesn't work.
sudo nano /etc/postgresql/*/main/pg_hba.conf  # Change peer to scram-sha-256 
sudo service postgresql restart

# You can now compile the project with CMake and run it.
