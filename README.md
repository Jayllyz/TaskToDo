<h1 align="center"> TaskTodo software </h1>

<p align="center">
  ESGI Project
</p>

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

## Table of Contents

- [Table of Contents](#table-of-contents)
- [Introduction](#introduction)
- [Features](#features)
- [Installation](#installation)
- [Contributors](#contributors)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Introduction

Projet de logiciel de planification de tâches réalisé en C avec la librairie GTK+3.2, l'UI est réalisée avec Glade.

## Features

- Ajouter/supprimer des tâches
- Éditer les tâches (nom, description, date de fin, priorité)
- Groupe de tâches pour synchroniser le statut et la date de fin
- Ajouter des projets
- Récupération du calendrier à l'aide de cURL
- Suivi de ses finances
- Calculatrice de base intégrée
- Filtre pour retrouver ses tâches (par date, par priorité, par projet)

## Installation

Ce projet a été réalisé sur Windows avec un environnement **WSL**, il est donc conseiller de l'installer sur une distribution Linux ou de l'installer sur Windows avec un environnement WSL.

```bash
# Clone the repository and go to the project folder
cd TaskToDo

# Run the script to install the dependencies
# Read the script before running it
# You may need to change some lines to fit your needs
chmod +x install.sh
./install.sh

# If the script doesn't work, run the following commands
sudo apt install dos2unix -y
# and then convert the file to unix format
dos2unix install.sh

# Compile and run with CMake
mkdir build && cd build
cmake ../
make
./TaskToDo

# Edit in settings/config.txt to change the settings
# Make sure to set init db to 0 the first time you run the program to insert
# the values in the database
```

## Contributors

[@Jayllyz](https://github.com/Jayllyz) & [@userMeh](https://github.com/userMeh)
